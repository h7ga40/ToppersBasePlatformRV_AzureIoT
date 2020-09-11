#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <kernel.h>
#include <t_syslog.h>
#include <t_stdlib.h>
#include <kernel_impl.h>
#include <target_syssvc.h>
#include "kendryte-k210.h"
#include "device.h"
#include "atomic.h"
#include "kpu.h"
#include "utils.h"
#include "kpu_main.h"
#include "kernel_cfg.h"

#define sil_orw_mem(a, b) sil_wrw_mem((a), sil_rew_mem(a) | (b))

uint64_t sysctl_get_time_us(void)
{
	uint64_t v_cycle = read_cycle();
	return v_cycle * 1000000 / SYSCTRL_CLOCK_FREQ_IN0;
}

static int is_memory(uintptr_t address)
{
	enum
	{
		mem_len = 6 * 1024 * 1024,
		mem_no_cache_len = 8 * 1024 * 1024,
	};
	return ((address >= 0x80000000) && (address < 0x80000000 + mem_len)) || ((address >= 0x40000000) && (address < 0x40000000 + mem_no_cache_len)) || (address == 0x50450040);
}

uint32_t is_memory_cache(uintptr_t address)
{
#define MEM_CACHE_LEN (6 * 1024 * 1024)

	return ((address >= 0x80000000) && (address < 0x80000000 + MEM_CACHE_LEN));
}

int plic_irq_enable(INTNO irq_number)
{
	if (irq_number != INTNO_AI)
		return -1;
	ena_int(irq_number);
	return 0;
}

int plic_set_priority(INTNO irq_number, uint32_t priority)
{
	if (irq_number != INTNO_AI)
		return -1;
	set_ipriority(irq_number, priority);
	return 0;
}

plic_irq_callback_t ai_done_callback;
void *ai_done_ctx;

void plic_irq_register(INTNO irq, plic_irq_callback_t callback, void *ctx)
{
	if (irq != INTNO_AI)
		return;

	dis_int(INTNO_AI);

	ai_done_callback = callback;
	ai_done_ctx = ctx;

	ena_int(INTNO_AI);
}

void ai_done_isr(intptr_t exinf)
{
	dis_int(INTNO_AI);
	if (ai_done_callback != NULL)
	{
		ai_done_callback(ai_done_ctx);
	}
	ena_int(INTNO_AI);
}

plic_irq_callback_t ai_dma_done_callback;
void *ai_dma_done_ctx;

void kpu_dmac_irq_register(dmac_channel_number_t channel_num,
						   plic_irq_callback_t dmac_callback, void *ctx, uint32_t priority)
{
	if (channel_num != AI_DMA_CH)
		return;

	//set_ipriority(INTNO_DMAAI, priority);

	dis_int(INTNO_DMAAI);

	ai_dma_done_callback = dmac_callback;
	ai_dma_done_ctx = ctx;

	ena_int(INTNO_DMAAI);
}

void ai_dma_done_isr(DMA_Handle_t *dma)
{
	dis_int(INTNO_DMAAI);

	if (ai_dma_done_callback != NULL)
	{
		ai_dma_done_callback(ai_dma_done_ctx);
	}

	ena_int(INTNO_DMAAI);
}

void dmac_set_irq(dmac_channel_number_t channel_num,
				  plic_irq_callback_t dmac_callback, void *ctx, uint32_t priority)
{
	if (channel_num != AI_DMA_CH)
		return;

	//set_ipriority(INTNO_DMAAI, priority);

	dis_int(INTNO_DMAAI);

	ai_dma_done_callback = dmac_callback;
	ai_dma_done_ctx = ctx;

	ena_int(INTNO_DMAAI);
}

DMA_Handle_t g_ai_hdma;

void dmac_set_single_mode(dmac_channel_number_t channel_num,
						  const void *src, void *dest, uint8_t src_inc,
						  uint8_t dest_inc,
						  uint8_t dmac_burst_size,
						  uint8_t dmac_trans_width,
						  size_t block_size)
{
	if (channel_num != AI_DMA_CH)
		return;

	DMA_Handle_t *hdma = &g_ai_hdma;
	int mem_type_src = is_memory((uintptr_t)src), mem_type_dest = is_memory((uintptr_t)dest);
	uint8_t flow_control;
	if (mem_type_src == 0 && mem_type_dest == 0)
		flow_control = DMA_PERIPH_TO_PERIPH;
	else if (mem_type_src == 1 && mem_type_dest == 0)
		flow_control = DMA_MEMORY_TO_PERIPH;
	else if (mem_type_src == 0 && mem_type_dest == 1)
		flow_control = DMA_PERIPH_TO_MEMORY;
	else
		flow_control = DMA_MEMORY_TO_MEMORY;

	hdma->Init.Direction = flow_control;											 /* DMA転送方向 */
	hdma->Init.SrcHandShake = (mem_type_src ? DMAC_HS_SOFTWARE : DMAC_HS_HARDWARE);	 /* ソースハンドシェイク */
	hdma->Init.DrcHandShake = (mem_type_dest ? DMAC_HS_SOFTWARE : DMAC_HS_HARDWARE); /* デスティネーションハンドシェイク */
	hdma->Init.SrcInc = src_inc;													 /* ソースインクリメント設定 */
	hdma->Init.DstInc = dest_inc;													 /* デスティネーションインクリメント設定 */
	hdma->Init.SrcTransWidth = dmac_trans_width;									 /* ソース転送幅 */
	hdma->Init.DstTransWidth = dmac_trans_width;									 /* デスティネーション転送幅 */
	hdma->Init.SrcBurstSize = dmac_burst_size;										 /* ソースバーストサイズ */
	hdma->Init.DstBurstSize = dmac_burst_size;										 /* デスティネーションバーストサイズ */
	dma_reset(hdma);

	dma_start(hdma, (uintptr_t)src, (uintptr_t)dest, block_size);
}

#define LAYER_BURST_SIZE 12

#define KPU_DEBUG 0
#define USE_CACHED_AI_RAM 0

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))
#define ALIGN_UP(x, align) ((x + (align - 1)) & (~(align - 1)))

static int ai_step(void *userdata);
static int kpu_kmodel_done(kpu_model_context_t *ctx);

volatile kpu_config_t *const kpu = (volatile kpu_config_t *)AI_BASE_ADDR;
static volatile uint32_t kpu_status;

static void kpu_send_layer(const kpu_layer_argument_t *layer)
{
	kpu->layer_argument_fifo = layer->interrupt_enabe.reg;
	kpu->layer_argument_fifo = layer->image_addr.reg;
	kpu->layer_argument_fifo = layer->image_channel_num.reg;
	kpu->layer_argument_fifo = layer->image_size.reg;
	kpu->layer_argument_fifo = layer->kernel_pool_type_cfg.reg;
	kpu->layer_argument_fifo = layer->kernel_load_cfg.reg;
	kpu->layer_argument_fifo = layer->kernel_offset.reg;
	kpu->layer_argument_fifo = layer->kernel_calc_type_cfg.reg;
	kpu->layer_argument_fifo = layer->write_back_cfg.reg;
	kpu->layer_argument_fifo = layer->conv_value.reg;
	kpu->layer_argument_fifo = layer->conv_value2.reg;
	kpu->layer_argument_fifo = layer->dma_parameter.reg;
}

void kpu_input_dma(const kpu_layer_argument_t *layer, const uint8_t *src, dmac_channel_number_t dma_ch, plic_irq_callback_t callback, void *userdata)
{
	uint64_t input_size = layer->kernel_calc_type_cfg.data.channel_switch_addr * 64 * (layer->image_channel_num.data.i_ch_num + 1);
	dmac_set_irq(dma_ch, callback, userdata, 1);
	dmac_set_single_mode(dma_ch, (void *)src, (void *)(uintptr_t)(AI_IO_BASE_ADDR + layer->image_addr.data.image_src_addr * 64), DMAC_ADDR_INCREMENT, DMAC_ADDR_INCREMENT,
						 DMAC_MSIZE_16, DMAC_TRANS_WIDTH_64, input_size / 8);
}

static void kpu_conv2d_core(kpu_layer_argument_t *layer)
{
	kpu_send_layer(layer);
}

void kpu_conv2d(kpu_layer_argument_t *layer)
{
	kpu->interrupt_clear.data = (kpu_config_interrupt_t){
		.calc_done_int = 1,
		.layer_cfg_almost_empty_int = 1,
		.layer_cfg_almost_full_int = 1};
	kpu->interrupt_mask.data = (kpu_config_interrupt_t){
		.calc_done_int = 1,
		.layer_cfg_almost_empty_int = 0,
		.layer_cfg_almost_full_int = 1};
	kpu_conv2d_core(layer);
}

void kpu_global_average_pool(const uint8_t *src, const quantize_param_t *src_param, int kernel_size, int channels, uint8_t *dest, const quantize_param_t *dest_param)
{
	quantize_param_t q1 = *src_param, q2 = *dest_param;
	size_t oc, y, x;

	if (((uintptr_t)dest) >= AI_IO_BASE_ADDR && ((uintptr_t)dest) < AI_IO_BASE_ADDR + 2 * 1024 * 1024)
	{
		uint32_t row_padding = 16;
		uint32_t row_group = 4;
		uint32_t row_length = 1;
		uint32_t height = 4;

		for (oc = 0; oc < channels; oc++)
		{
			uint8_t *channel_origin = dest + oc / row_group * row_length * height * 64 + oc % row_group * row_padding;
			for (y = 0; y < 1; y++)
			{
				uint8_t *y_origin = channel_origin + y * row_length * 64;
				for (x = 0; x < 1; x++)
				{
					int64_t sum = 0;
					size_t i;
					for (i = 0; i < kernel_size; i++)
						sum += *src++;

					int value = ((sum * q1.scale + q1.bias) / kernel_size - q2.bias) / q2.scale;
					if (value < 0)
						value = 0;
					if (value > 0xFF)
						value = 0xFF;
					y_origin[x] = value;
				}
			}
		}
	}
	else
	{
		for (oc = 0; oc < channels; oc++)
		{
			int64_t sum = 0;
			size_t i;
			for (i = 0; i < kernel_size; i++)
				sum += *src++;

			int value = ((sum * q1.scale + q1.bias) / kernel_size - q2.bias) / q2.scale;
			if (value < 0)
				value = 0;
			if (value > 0xFF)
				value = 0xFF;
			dest[oc] = value;
		}
	}
}

void kpu_global_average_pool_float(const uint8_t *src, const quantize_param_t *src_param, int kernel_size, int channels, float *dest)
{
	quantize_param_t q = *src_param;
	size_t oc;

	for (oc = 0; oc < channels; oc++)
	{
		int64_t sum = 0;
		size_t i;
		for (i = 0; i < kernel_size; i++)
			sum += *src++;

		float value = (sum * q.scale + q.bias) / kernel_size;
		dest[oc] = value;
	}
}

#if USE_CACHED_AI_RAM
static void kpu_flush_cache(uint32_t addr, size_t lines)
{
	size_t line;
	for (line = 0; line < lines; line++)
	{
		const uint64_t *src = (const uint64_t *)(AI_RAM_BASE_ADDR + (addr + line) * 64);
		uint64_t *dest = (uint64_t *)(AI_IO_BASE_ADDR + (addr + line) * 64);
		size_t i;
		for (i = 0; i < 8; i++)
			dest[i] = src[i];
	}
}
#endif
static int64_t kpu_carry_shift(int64_t value, uint32_t shift)
{
	if (shift > 0)
	{
		value >>= shift - 1;
		if (value & 0x1)
		{
			if (value < 0)
				value = (value >> 1) - 1;
			else
				value = (value >> 1) + 1;
		}
		else
		{
			value >>= 1;
		}
	}

	return value;
}
static void kpu_upload_core(size_t width, size_t height, size_t channels, const uint8_t *src, uint32_t kpu_addr)
{
	uint8_t *dest = (uint8_t *)(uintptr_t)(AI_IO_BASE_ADDR + kpu_addr * 64);
	size_t oc, y, x;
	uint32_t row_padding;
	uint32_t row_group;
	uint32_t row_length;
	if (width <= 16)
	{
		row_padding = 16;
		row_group = 4;
		row_length = 1;
	}
	else if (width <= 32)
	{
		row_padding = 32;
		row_group = 2;
		row_length = 1;
	}
	else
	{
		row_padding = 64;
		row_group = 1;
		row_length = (width + 63) / 64;
	}

	if ((uintptr_t)src % 8 == 0 && width % 8 == 0)
	{
#define UPLOAD_BEGIN()                                                                                             \
	for (oc = 0; oc < channels; oc++)                                                                              \
	{                                                                                                              \
		uint8_t *channel_origin = dest + oc / row_group * row_length * height * 64 + oc % row_group * row_padding; \
		for (y = 0; y < height; y++)                                                                               \
		{                                                                                                          \
			uint64_t *y_origin = (uint64_t *)(channel_origin + y * row_length * 64);

#define UPLOAD_END() \
	}                \
	}

		width /= 8;
		const uint64_t *u64_src = (const uint64_t *)src;
		if (width == 1)
		{
			UPLOAD_BEGIN()
			y_origin[0] = *u64_src++;
			UPLOAD_END()
		}
		else if (width == 2)
		{
			UPLOAD_BEGIN()
			{
				y_origin[0] = *u64_src++;
				y_origin[1] = *u64_src++;
			}
			UPLOAD_END()
		}
		else if (width == 4)
		{
			UPLOAD_BEGIN()
			{
				y_origin[0] = *u64_src++;
				y_origin[1] = *u64_src++;
				y_origin[2] = *u64_src++;
				y_origin[3] = *u64_src++;
			}
			UPLOAD_END()
		}
		else
		{
			UPLOAD_BEGIN()
			for (x = 0; x < width; x++)
				y_origin[x] = *u64_src++;
			UPLOAD_END()
		}
	}
	else
	{
		for (oc = 0; oc < channels; oc++)
		{
			uint8_t *channel_origin = dest + oc / row_group * row_length * height * 64 + oc % row_group * row_padding;
			for (y = 0; y < height; y++)
			{
				uint8_t *y_origin = channel_origin + y * row_length * 64;
				for (x = 0; x < width; x++)
					y_origin[x] = *src++;
			}
		}
	}
}
static void kpu_kmodel_input_with_padding(const kpu_layer_argument_t *layer, const uint8_t *src)
{
	size_t width = layer->image_size.data.i_row_wid + 1;
	size_t height = layer->image_size.data.i_col_high + 1;
	size_t channels = layer->image_channel_num.data.i_ch_num + 1;

	kpu_upload_core(width, height, channels, src, layer->image_addr.data.image_src_addr);
}

static void kpu_kmodel_input_float(const float *src, float *dest, size_t count)
{
	memcpy(dest, src, count * sizeof(float));
}

static void kpu_float_activation(float *data, size_t count, kpu_model_activation_t act)
{
	size_t i;

	if (act == KLA_RELU)
	{
		for (i = 0; i < count; i++)
			data[i] = max(data[i], 0);
	}
	else if (act == KLA_RELU6)
	{
		for (i = 0; i < count; i++)
			data[i] = min(max(data[i], 0), 6);
	}
}

static void kpu_kmodel_add(const kpu_model_add_layer_argument_t *arg, kpu_model_context_t *ctx)
{
	const float *src_a = (const float *)(ctx->main_buffer + arg->main_mem_in_a_address);
	const float *src_b = (const float *)(ctx->main_buffer + arg->main_mem_in_b_address);
	float *dest = (float *)(ctx->main_buffer + arg->main_mem_out_address);
	size_t i, count = arg->count;

	for (i = 0; i < count; i++)
		dest[i] = src_a[i] + src_b[i];
}

static void kpu_quantized_add(const kpu_model_quant_add_layer_argument_t *arg, kpu_model_context_t *ctx)
{
	const uint8_t *src_a = (const uint8_t *)(ctx->main_buffer + arg->main_mem_in_a_address);
	const uint8_t *src_b = (const uint8_t *)(ctx->main_buffer + arg->main_mem_in_b_address);
	size_t count = ALIGN_UP(arg->count, 8) / 8;
	int64_t off_a = arg->in_a_offset, mul_a = arg->in_a_mul, sh_a = arg->in_a_shift;
	int64_t off_b = arg->in_b_offset, mul_b = arg->in_b_mul, sh_b = arg->in_b_shift;
	int64_t off_o = arg->out_offset, mul_o = arg->out_mul, sh_o = arg->out_shift;

	uint8_t *dest = (uint8_t *)(ctx->main_buffer + arg->main_mem_out_address);
	size_t i;

	if (sh_a == sh_b)
	{
#define QADD_UNROLL_1(x)     \
	int64_t a##x = *src_a++; \
	int64_t b##x = *src_b++;

#define QADD_UNROLL_2(x) \
	a##x += off_a;       \
	b##x += off_b;

#define QADD_UNROLL_3(x) \
	a##x *= mul_a;       \
	b##x *= mul_b;

#define QADD_UNROLL_4(x) \
	int64_t v##x = a##x + b##x;

#define QADD_UNROLL_5(x) \
	v##x >>= sh_a;

#define QADD_UNROLL_6(x) \
	v##x *= mul_o;

#define QADD_UNROLL_7(x) \
	v##x = kpu_carry_shift(v##x, sh_o);

#define QADD_UNROLL_8(x) \
	v##x += off_o;

#define QADD_UNROLL_9(x) \
	v##x = min(0xFF, max(0, v##x));

#define QADD_UNROLL_10(x) \
	*dest++ = v##x;

#define QADD_UNROLL_S(x)                       \
	QADD_UNROLL_##x(0)                         \
		QADD_UNROLL_##x(1)                     \
			QADD_UNROLL_##x(2)                 \
				QADD_UNROLL_##x(3)             \
					QADD_UNROLL_##x(4)         \
						QADD_UNROLL_##x(5)     \
							QADD_UNROLL_##x(6) \
								QADD_UNROLL_##x(7)

		for (i = 0; i < count; i++)
		{
			QADD_UNROLL_S(1);
			QADD_UNROLL_S(2);
			QADD_UNROLL_S(3);
			QADD_UNROLL_S(4);
			QADD_UNROLL_S(5);
			QADD_UNROLL_S(6);
			QADD_UNROLL_S(7);
			QADD_UNROLL_S(8);
			QADD_UNROLL_S(9);
			QADD_UNROLL_S(10);
		}
	}
	else
	{
#undef QADD_UNROLL_1
#define QADD_UNROLL_1(x)     \
	int64_t a##x = *src_a++; \
	int64_t b##x = *src_b++;

#undef QADD_UNROLL_2
#define QADD_UNROLL_2(x) \
	a##x += off_a;       \
	b##x += off_b;

#undef QADD_UNROLL_3
#define QADD_UNROLL_3(x) \
	a##x *= mul_a;       \
	b##x *= mul_b;

#undef QADD_UNROLL_4
#define QADD_UNROLL_4(x) \
	a##x >>= sh_a;       \
	b##x >>= sh_b;

#undef QADD_UNROLL_5
#define QADD_UNROLL_5(x) \
	int64_t v##x = a##x + b##x;

#undef QADD_UNROLL_6
#define QADD_UNROLL_6(x) \
	v##x *= mul_o;

#undef QADD_UNROLL_7
#define QADD_UNROLL_7(x) \
	v##x = kpu_carry_shift(v##x, sh_o);

#undef QADD_UNROLL_8
#define QADD_UNROLL_8(x) \
	v##x += off_o;

#undef QADD_UNROLL_9
#define QADD_UNROLL_9(x) \
	v##x = min(0xFF, max(0, v##x));

#undef QADD_UNROLL_10
#define QADD_UNROLL_10(x) \
	*dest++ = v##x;

#undef QADD_UNROLL_S
#define QADD_UNROLL_S(x)                       \
	QADD_UNROLL_##x(0)                         \
		QADD_UNROLL_##x(1)                     \
			QADD_UNROLL_##x(2)                 \
				QADD_UNROLL_##x(3)             \
					QADD_UNROLL_##x(4)         \
						QADD_UNROLL_##x(5)     \
							QADD_UNROLL_##x(6) \
								QADD_UNROLL_##x(7)

		for (i = 0; i < count; i++)
		{
			QADD_UNROLL_S(1);
			QADD_UNROLL_S(2);
			QADD_UNROLL_S(3);
			QADD_UNROLL_S(4);
			QADD_UNROLL_S(5);
			QADD_UNROLL_S(6);
			QADD_UNROLL_S(7);
			QADD_UNROLL_S(8);
			QADD_UNROLL_S(9);
			QADD_UNROLL_S(10);
		}
	}
}

static void kpu_global_average_pool2d(const kpu_model_gap2d_layer_argument_t *arg, kpu_model_context_t *ctx)
{
	const float *src = (const float *)(ctx->main_buffer + arg->main_mem_in_address);
	float *dest = (float *)(ctx->main_buffer + arg->main_mem_out_address);
	size_t oc, channels = arg->channels, kernel_size = arg->kernel_size;

	for (oc = 0; oc < channels; oc++)
	{
		float sum = 0.f;
		size_t i;
		for (i = 0; i < kernel_size; i++)
			sum += *src++;

		dest[oc] = sum / kernel_size;
	}
}

static void kpu_quantized_max_pool2d(const kpu_model_quant_max_pool2d_layer_argument_t *arg, kpu_model_context_t *ctx)
{
	const uint8_t *src = (const uint8_t *)(ctx->main_buffer + arg->main_mem_in_address);
	uint8_t *dest = (uint8_t *)(ctx->main_buffer + arg->main_mem_out_address);
	kpu_model_shape_t in_shape = arg->in_shape, out_shape = arg->out_shape;
	uint32_t kernel_width = arg->kernel_width, kernel_height = arg->kernel_height;
	uint32_t stride_width = arg->stride_width, stride_height = arg->stride_height;
	uint32_t padding_width = arg->padding_width, padding_height = arg->padding_height;

	uint32_t out_y, out_x, oc;

	for (oc = 0; oc < out_shape.channels; oc++)
	{
		const uint8_t *channel_src = src + in_shape.width * in_shape.height * oc;
		for (out_y = 0; out_y < out_shape.height; out_y++)
		{
			for (out_x = 0; out_x < out_shape.width; out_x++)
			{
				int32_t in_x_origin = (int32_t)(out_x * stride_width) - padding_width;
				int32_t in_y_origin = (int32_t)(out_y * stride_height) - padding_height;
				int32_t kernel_x_start = max(0, -in_x_origin);
				int32_t kernel_x_end = min(kernel_width, in_shape.width - in_x_origin);
				int32_t kernel_y_start = max(0, -in_y_origin);
				int32_t kernel_y_end = min(kernel_height, in_shape.height - in_y_origin);
				uint8_t value = 0;

				int32_t kernel_y, kernel_x;
				for (kernel_y = kernel_y_start; kernel_y < kernel_y_end; kernel_y++)
				{
					for (kernel_x = kernel_x_start; kernel_x < kernel_x_end; kernel_x++)
					{
						int32_t in_x = in_x_origin + kernel_x;
						int32_t in_y = in_y_origin + kernel_y;
						value = max(value, channel_src[in_y * in_shape.width + in_x]);
					}
				}

				*dest++ = value;
			}
		}
	}
}

static void kpu_average_pool2d(const kpu_model_ave_pool2d_layer_argument_t *arg, kpu_model_context_t *ctx)
{
	const float *src = (const float *)(ctx->main_buffer + arg->main_mem_in_address);
	float *dest = (float *)(ctx->main_buffer + arg->main_mem_out_address);
	kpu_model_shape_t in_shape = arg->in_shape, out_shape = arg->out_shape;
	uint32_t kernel_width = arg->kernel_width, kernel_height = arg->kernel_height;
	uint32_t stride_width = arg->stride_width, stride_height = arg->stride_height;
	uint32_t padding_width = arg->padding_width, padding_height = arg->padding_height;

	uint32_t out_y, out_x, oc;

	for (oc = 0; oc < out_shape.channels; oc++)
	{
		const float *channel_src = src + in_shape.width * in_shape.height * oc;
		for (out_y = 0; out_y < out_shape.height; out_y++)
		{
			for (out_x = 0; out_x < out_shape.width; out_x++)
			{
				int32_t in_x_origin = (int32_t)(out_x * stride_width) - padding_width;
				int32_t in_y_origin = (int32_t)(out_y * stride_height) - padding_height;
				int32_t kernel_x_start = max(0, -in_x_origin);
				int32_t kernel_x_end = min(kernel_width, in_shape.width - in_x_origin);
				int32_t kernel_y_start = max(0, -in_y_origin);
				int32_t kernel_y_end = min(kernel_height, in_shape.height - in_y_origin);
				float value = 0;
				float kernel_count = 0;

				int32_t kernel_y, kernel_x;
				for (kernel_y = kernel_y_start; kernel_y < kernel_y_end; kernel_y++)
				{
					for (kernel_x = kernel_x_start; kernel_x < kernel_x_end; kernel_x++)
					{
						int32_t in_x = in_x_origin + kernel_x;
						int32_t in_y = in_y_origin + kernel_y;
						value += channel_src[in_y * in_shape.width + in_x];
						kernel_count++;
					}
				}

				*dest++ = value / kernel_count;
			}
		}
	}
}

static void kpu_quantize(const kpu_model_quantize_layer_argument_t *arg, kpu_model_context_t *ctx)
{
	size_t count = arg->count;
	const float *src = (const float *)(ctx->main_buffer + arg->main_mem_in_address);

	kpu_model_quant_param_t q = arg->quant_param;

	float scale = 1.f / q.scale;

	uint8_t *dest = (uint8_t *)(ctx->main_buffer + arg->mem_out_address);
	size_t i;
	for (i = 0; i < count; i++)
	{
		int value = roundf((*src++ - q.bias) * scale);
		if (value < 0)
			value = 0;
		if (value > 0xFF)
			value = 0xFF;
		*dest++ = (uint8_t)value;
	}
}

static void kpu_kmodel_dequantize(const kpu_model_dequantize_layer_argument_t *arg, kpu_model_context_t *ctx)
{
	const uint8_t *src = (const uint8_t *)(ctx->main_buffer + arg->main_mem_in_address);
	float *dest = (float *)(ctx->main_buffer + arg->main_mem_out_address);
	size_t oc, count = arg->count;
	kpu_model_quant_param_t q = arg->quant_param;

	for (oc = 0; oc < count; oc++)
		dest[oc] = *src++ * q.scale + q.bias;
}

static void kpu_kmodel_channelwise_dequantize(const kpu_model_channelwise_dequant_argument_t *arg, kpu_model_context_t *ctx)
{
	const uint8_t *src = (const uint8_t *)(ctx->main_buffer + arg->main_mem_in_address);
	float *dest = (float *)(ctx->main_buffer + arg->main_mem_out_address);
	size_t oc, i, channels = arg->channels, count = arg->channel_size;

	for (oc = 0; oc < channels; oc++)
	{
		const kpu_model_quant_param_t q = arg->quant_params[oc];

		for (i = 0; i < count; i++)
			*dest++ = *src++ * q.scale + q.bias;
	}
}

static void kpu_requantize(const kpu_model_requantize_layer_argument_t *arg, kpu_model_context_t *ctx)
{
	const uint8_t *src = (const uint8_t *)(ctx->main_buffer + arg->main_mem_in_address);
	uint8_t *dest = (uint8_t *)(ctx->main_buffer + arg->main_mem_out_address);
	size_t oc, count = arg->count;
	const uint8_t *table = arg->table;

	if (false && count % 8 == 0)
	{
		for (oc = 0; oc < count;)
		{
			dest[oc++] = table[*src++];
			dest[oc++] = table[*src++];
			dest[oc++] = table[*src++];
			dest[oc++] = table[*src++];
			dest[oc++] = table[*src++];
			dest[oc++] = table[*src++];
			dest[oc++] = table[*src++];
			dest[oc++] = table[*src++];
		}
	}
	else
	{
		for (oc = 0; oc < count; oc++)
			dest[oc] = table[src[oc]];
	}
}

static void kpu_l2_normalization(const kpu_model_l2_norm_layer_argument_t *arg, kpu_model_context_t *ctx)
{
	const float *src = (const float *)(ctx->main_buffer + arg->main_mem_in_address);
	float *dest = (float *)(ctx->main_buffer + arg->main_mem_out_address);
	size_t oc, channels = arg->channels;

	float sum = 0.f;
	const float epsilon = 1e-10f;
	for (oc = 0; oc < channels; oc++)
		sum += src[oc] * src[oc];
	if (sum < epsilon)
		sum = epsilon;
	sum = 1.f / sqrtf(sum);
	for (oc = 0; oc < channels; oc++)
		dest[oc] = src[oc] * sum;
}

static void kpu_softmax(const kpu_model_softmax_layer_argument_t *arg, kpu_model_context_t *ctx)
{
	const float *src = (const float *)(ctx->main_buffer + arg->main_mem_in_address);
	float *dest = (float *)(ctx->main_buffer + arg->main_mem_out_address);
	size_t oc, channels = arg->channels;

	float max = FLT_MIN;
	for (oc = 0; oc < channels; oc++)
		max = fmaxf(max, src[oc]);

	float sum = 0.f;
	for (oc = 0; oc < channels; oc++)
	{
		float value = expf(src[oc] - max);
		sum += value;
		dest[oc] = value;
	}

	for (oc = 0; oc < channels; oc++)
		dest[oc] /= sum;
}

static void kpu_concat(const kpu_model_concat_layer_argument_t *arg, kpu_model_context_t *ctx)
{
	uint8_t *dest = (uint8_t *)(ctx->main_buffer + arg->main_mem_out_address);
	uint32_t count = arg->input_count, i;

	for (i = 0; i < count; i++)
	{
		kpu_model_memory_range_t input = arg->inputs_mem[i];
		const uint8_t *src = (const uint8_t *)(ctx->main_buffer + input.start);
		memcpy(dest, src, input.size);
		dest += input.size;
	}
}

static void kpu_kmodel_fully_connected(const kpu_model_fully_connected_layer_argument_t *arg, kpu_model_context_t *ctx)
{
	const float *src = (const float *)(ctx->main_buffer + arg->main_mem_in_address);
	float *dest = (float *)(ctx->main_buffer + arg->main_mem_out_address);
	uint32_t in_channels = arg->in_channels, out_channels = arg->out_channels, ic, oc;
	float *weights = (float *)malloc(in_channels * out_channels * sizeof(float));
	float *bias = (float *)malloc(out_channels * sizeof(float));
	memcpy(weights, arg->weights, out_channels * in_channels * sizeof(float));
	memcpy(bias, arg->weights + in_channels * out_channels, out_channels * sizeof(float));

	if (in_channels % 8 == 0)
	{
#define FC_UNROLL_1(x)     \
	float i##x = *c_src++; \
	float w##x = *c_weights++;

#define FC_UNROLL_2(x) \
	sum += i##x * w##x;

#define FC_UNROLL_S(x)                       \
	FC_UNROLL_##x(0)                         \
		FC_UNROLL_##x(1)                     \
			FC_UNROLL_##x(2)                 \
				FC_UNROLL_##x(3)             \
					FC_UNROLL_##x(4)         \
						FC_UNROLL_##x(5)     \
							FC_UNROLL_##x(6) \
								FC_UNROLL_##x(7)

		for (oc = 0; oc < out_channels; oc++)
		{
			const float *c_src = src;
			const float *c_weights = weights + oc * in_channels;

			float sum = 0.0f;
			for (ic = 0; ic < in_channels / 8; ic++)
			{
				FC_UNROLL_S(1);
				FC_UNROLL_S(2);
			}

			dest[oc] = sum + bias[oc];
		}
	}
	else
	{
		for (oc = 0; oc < out_channels; oc++)
		{
			const float *c_weights = weights + oc * in_channels;

			float sum = 0.0f;
			for (ic = 0; ic < in_channels; ic++)
				sum += src[ic] * c_weights[ic];
			dest[oc] = sum + bias[oc];
		}
	}
	free(weights);
	free(bias);
	kpu_float_activation(dest, out_channels, arg->act);
}

static void kpu_tf_flatten(const kpu_model_tf_flatten_layer_argument_t *arg, kpu_model_context_t *ctx)
{
	const float *src = (const float *)(ctx->main_buffer + arg->main_mem_in_address);
	float *dest = (float *)(ctx->main_buffer + arg->main_mem_out_address);
	kpu_model_shape_t in_shape = arg->shape;
	uint32_t oc, oy, ox;

	for (oy = 0; oy < in_shape.height; oy++)
		for (ox = 0; ox < in_shape.width; ox++)
			for (oc = 0; oc < in_shape.channels; oc++)
				*dest++ = src[(oc * in_shape.height + oy) * in_shape.width + ox];
}

static void kpu_resize_nearest_neighbor(const kpu_model_resize_nearest_neighbor_layer_argument_t *arg, kpu_model_context_t *ctx)
{
	const float *src = (const float *)(ctx->main_buffer + arg->main_mem_in_address);
	float *dest = (float *)(ctx->main_buffer + arg->main_mem_out_address);
	kpu_model_shape_t in_shape = arg->in_shape;
	uint32_t out_width = arg->out_width, out_height = arg->out_height;
	uint32_t oc, oy, ox;

	float height_scale = (float)in_shape.height / out_height;
	float width_scale = (float)in_shape.width / out_width;

	for (oc = 0; oc < in_shape.channels; oc++)
	{
		const float *channel_src = src + in_shape.width * in_shape.height * oc;
		for (oy = 0; oy < out_height; oy++)
		{
			uint32_t in_y = (uint32_t)min(floorf(oy * height_scale), in_shape.height - 1);
			const float *y_origin = channel_src + in_y * in_shape.width;
			for (ox = 0; ox < out_width; ox++)
			{
				uint32_t in_x = (uint32_t)min(floorf(ox * width_scale), in_shape.width - 1);
				*dest++ = y_origin[in_x];
			}
		}
	}
}

static void kpu_quant_resize_nearest_neighbor(const kpu_model_quant_resize_nearest_neighbor_layer_argument_t *arg, kpu_model_context_t *ctx)
{
	const uint8_t *src = (const uint8_t *)(ctx->main_buffer + arg->main_mem_in_address);
	uint8_t *dest = (uint8_t *)(ctx->main_buffer + arg->main_mem_out_address);
	kpu_model_shape_t in_shape = arg->in_shape;
	uint32_t out_width = arg->out_width, out_height = arg->out_height;
	uint32_t oc, oy, ox;

	float height_scale = (float)in_shape.height / out_height;
	float width_scale = (float)in_shape.width / out_width;

	for (oc = 0; oc < in_shape.channels; oc++)
	{
		const uint8_t *channel_src = src + in_shape.width * in_shape.height * oc;
		for (oy = 0; oy < out_height; oy++)
		{
			uint32_t in_y = (uint32_t)min(floorf(oy * height_scale), in_shape.height - 1);
			const uint8_t *y_origin = channel_src + in_y * in_shape.width;
			for (ox = 0; ox < out_width; ox++)
			{
				uint32_t in_x = (uint32_t)min(floorf(ox * width_scale), in_shape.width - 1);
				*dest++ = y_origin[in_x];
			}
		}
	}
}

static void kpu_logistic(const kpu_model_logistic_layer_argument_t *arg, kpu_model_context_t *ctx)
{
	const float *src = (const float *)(ctx->main_buffer + arg->main_mem_in_address);
	float *dest = (float *)(ctx->main_buffer + arg->main_mem_out_address);
	size_t oc, channels = arg->channels;

	for (oc = 0; oc < channels; oc++)
		dest[oc] = 1.f / (1.f + expf(-src[oc]));
}

static void kpu_conv(const kpu_model_conv_layer_argument_t *arg, kpu_model_context_t *ctx)
{
	volatile kpu_layer_argument_t layer = *(const volatile kpu_layer_argument_t *)(ctx->model_buffer + arg->layer_offset);
	layer.kernel_load_cfg.data.para_start_addr = (uintptr_t)(ctx->model_buffer + arg->weights_offset) - IOMEM;
	layer.kernel_pool_type_cfg.data.bwsx_base_addr = (uintptr_t)(ctx->model_buffer + arg->bn_offset) - IOMEM;
	layer.kernel_calc_type_cfg.data.active_addr = (uintptr_t)(ctx->model_buffer + arg->act_offset) - IOMEM;

	if (arg->flags & KLF_MAIN_MEM_OUT)
	{
		dmac_channel_number_t dma_ch = ctx->dma_ch;
		uint8_t *dest = ctx->main_buffer + arg->main_mem_out_address;
		kpu->interrupt_clear.data = (kpu_config_interrupt_t){
			.calc_done_int = 1,
			.layer_cfg_almost_empty_int = 1,
			.layer_cfg_almost_full_int = 1};
		kpu->interrupt_mask.data = (kpu_config_interrupt_t){
			.calc_done_int = 1,
			.layer_cfg_almost_empty_int = 1,
			.layer_cfg_almost_full_int = 1};
		layer.dma_parameter.data.send_data_out = 1;
		select_dma_channel(dma_ch, DMA_SELECT_AI_RX_REQ);
		if (ctx->current_layer < ctx->layers_length)
			dmac_set_irq(dma_ch, ai_step, ctx, 1);
		else
			dmac_set_irq(dma_ch, (plic_irq_callback_t)kpu_kmodel_done, ctx, 1);
		dmac_set_single_mode(dma_ch, (void *)(&kpu->fifo_data_out), dest, DMAC_ADDR_NOCHANGE, DMAC_ADDR_INCREMENT,
							 DMAC_MSIZE_8, DMAC_TRANS_WIDTH_64, (layer.dma_parameter.data.dma_total_byte + 8) / 8);
	}
	else
	{
		kpu->interrupt_clear.data = (kpu_config_interrupt_t){
			.calc_done_int = 1,
			.layer_cfg_almost_empty_int = 1,
			.layer_cfg_almost_full_int = 1};

		kpu->interrupt_mask.data = (kpu_config_interrupt_t){
			.calc_done_int = 0,
			.layer_cfg_almost_empty_int = 1,
			.layer_cfg_almost_full_int = 1};
		layer.interrupt_enabe.data.int_en = 1;
	}

	kpu_send_layer((const kpu_layer_argument_t *)&layer);
}

static void kpu_add_padding(const kpu_model_add_padding_layer_argument_t *arg, kpu_model_context_t *ctx)
{
	const uint8_t *src = (const uint8_t *)(ctx->main_buffer + arg->main_mem_in_address);
#if USE_CACHED_AI_RAM
	uint8_t *dest = (uint8_t *)(uintptr_t)(AI_RAM_BASE_ADDR + arg->kpu_mem_out_address * 64);
#else
	uint8_t *dest = (uint8_t *)(uintptr_t)(AI_IO_BASE_ADDR + arg->kpu_mem_out_address * 64);
#endif

	uint32_t row_padding = 16;
	uint32_t row_group = 4;
	uint32_t row_length = 1;
	uint32_t height = 4;
	uint32_t oc, x, y, channels = arg->channels;

	for (oc = 0; oc < channels; oc++)
	{
		uint8_t *channel_origin = dest + oc / row_group * row_length * height * 64 + oc % row_group * row_padding;
		for (y = 0; y < 1; y++)
		{
			uint8_t *y_origin = channel_origin + y * row_length * 64;
			for (x = 0; x < 1; x++)
				y_origin[x] = *src++;
		}
	}

#if USE_CACHED_AI_RAM
	uint32_t lines = row_length * height * channels / row_group;
	kpu_flush_cache(arg->kpu_mem_out_address, lines);
#endif
}

static void kpu_remove_padding(const kpu_model_remove_padding_layer_argument_t *arg, kpu_model_context_t *ctx)
{
	const uint8_t *src = (const uint8_t *)(ctx->main_buffer + arg->main_mem_in_address);
	uint8_t *dest = (uint8_t *)(ctx->main_buffer + arg->main_mem_out_address);
	uint32_t oc, channels = arg->channels;

	for (oc = 0; oc < channels; oc++)
		*dest++ = src[oc * 16];
}

static void kpu_upload(const kpu_model_upload_layer_argument_t *arg, kpu_model_context_t *ctx)
{
	size_t width = arg->width;
	size_t height = arg->height;
	size_t channels = arg->channels;

	kpu_upload_core(width, height, channels, ctx->main_buffer + arg->main_mem_in_address, arg->kpu_mem_out_address);
}

int kpu_load_kmodel(kpu_model_context_t *ctx, const uint8_t *buffer)
{
#if FIX_CACHE
	configASSERT(is_memory_cache((uintptr_t)buffer));
#endif
	uintptr_t base_addr = (uintptr_t)buffer;
	const kpu_kmodel_header_t *header = (const kpu_kmodel_header_t *)buffer;

	if (header->version == 3 && header->arch == 0)
	{
		ctx->model_buffer = buffer;
		ctx->output_count = header->output_count;
		ctx->outputs = (const kpu_model_output_t *)(base_addr + sizeof(kpu_kmodel_header_t));
		ctx->layer_headers = (const kpu_model_layer_header_t *)((uintptr_t)ctx->outputs + sizeof(kpu_model_output_t) * ctx->output_count);
		ctx->layers_length = header->layers_length;
		ctx->body_start = (const uint8_t *)((uintptr_t)ctx->layer_headers + sizeof(kpu_model_layer_header_t) * header->layers_length);
		ctx->main_buffer = (uint8_t *)malloc(header->main_mem_usage);
		if (!ctx->main_buffer)
			return -1;
		uint32_t body_size = 0;
		for (int i = 0; i < ctx->layers_length; i++)
		{
			const kpu_model_layer_header_t *cnt_layer_header = ctx->layer_headers + i;
			body_size += cnt_layer_header->body_size;
		}
		uint8_t *body_start_iomem = (uint8_t *)((uintptr_t)ctx->body_start - IOMEM);
		const uint8_t *body_start_cache = ctx->body_start;
		memcpy(body_start_iomem, body_start_cache, body_size);
		for (int i = 0; i < body_size; i++)
		{
			configASSERT(body_start_iomem[i] == body_start_cache[i]);
		}
	}
	else
	{
		return -1;
	}

	return 0;
}

int kpu_get_output(kpu_model_context_t *ctx, uint32_t index, uint8_t **data, size_t *size)
{
	if (index >= ctx->output_count)
		return -1;

	const kpu_model_output_t *output = ctx->outputs + index;
	*data = ctx->main_buffer + output->address;
	*size = output->size;
	return 0;
}

void kpu_model_free(kpu_model_context_t *ctx)
{
	free(ctx->main_buffer);
	ctx->main_buffer = NULL;
}

#if KPU_DEBUG
static uint64_t last_time;
static uint64_t total_time;
static uint64_t kpu_time;
static uint32_t last_layer_type;

static const char *str_layer_type(uint32_t type)
{
	switch (type)
	{
	case KL_ADD:
		return "Add";
	case KL_QUANTIZED_ADD:
		return "QuantAdd";
	case KL_GLOBAL_AVERAGE_POOL2D:
		return "GAP";
	case KL_QUANTIZED_MAX_POOL2D:
		return "QuantMaxPool2d";
	case KL_AVERAGE_POOL2D:
		return "AveragePool2d";
	case KL_QUANTIZE:
		return "Quantize";
	case KL_DEQUANTIZE:
		return "Dequantize";
	case KL_REQUANTIZE:
		return "Requantize";
	case KL_L2_NORMALIZATION:
		return "L2Norm";
	case KL_SOFTMAX:
		return "Softmax";
	case KL_CONCAT:
		return "Concat";
	case KL_QUANTIZED_CONCAT:
		return "QuantConcat";
	case KL_FULLY_CONNECTED:
		return "FullyConnected";
	case KL_TENSORFLOW_FLATTEN:
		return "TFFlatten";
	case KL_RESIZE_NEAREST_NEIGHBOR:
		return "ResizeNearestNeighbor";
	case KL_QUANTIZED_RESIZE_NEAREST_NEIGHBOR:
		return "QuantResizeNearestNeighbor";
	case KL_CHANNELWISE_DEQUANTIZE:
		return "ChannelwiseDequantize";
	case KL_LOGISTIC:
		return "Logistic";
	case KL_K210_CONV:
		return "K210Conv";
	case KL_K210_ADD_PADDING:
		return "K210AddPad";
	case KL_K210_REMOVE_PADDING:
		return "K210RemovePad";
	case KL_K210_UPLOAD:
		return "K210Upload";
	default:
		return "Unknown";
	}
}
#endif

static int kpu_kmodel_done(kpu_model_context_t *ctx)
{
	kpu->interrupt_clear.data = (kpu_config_interrupt_t){
		.calc_done_int = 1,
		.layer_cfg_almost_empty_int = 1,
		.layer_cfg_almost_full_int = 1};
	kpu->interrupt_mask.data = (kpu_config_interrupt_t){
		.calc_done_int = 1,
		.layer_cfg_almost_empty_int = 1,
		.layer_cfg_almost_full_int = 1};
#if KPU_DEBUG
	uint32_t cnt_layer_id = ctx->current_layer;
	uint64_t time = sysctl_get_time_us();
	if (last_time != 0)
	{
		uint64_t layer_time = time - last_time;
		syslog(LOG_NOTICE, "layer %d/%d [%s]: %d.%03d ms", cnt_layer_id, ctx->layers_length, str_layer_type(last_layer_type), layer_time / 1000, layer_time % 1000);
		total_time += layer_time;
		if (last_layer_type == KL_K210_CONV)
			kpu_time += layer_time;
	}

	syslog(LOG_NOTICE, "KPU: %d.%03d ms", kpu_time / 1000, kpu_time % 1000);
	syslog(LOG_NOTICE, "CPU: %d.%03d ms", (total_time - kpu_time) / 1000, (total_time - kpu_time) % 1000);
	syslog(LOG_NOTICE, "Model: %d.%03d ms", total_time / 1000, total_time % 1000);
#endif
	ctx->done_callback(ctx->userdata);
	return 0;
}

static int ai_step(void *userdata)
{
	kpu_model_context_t *ctx = (kpu_model_context_t *)userdata;

	uint32_t cnt_layer_id = ctx->current_layer;
	const uint8_t *layer_body = ctx->current_body;
	const kpu_model_layer_header_t *cnt_layer_header = ctx->layer_headers + cnt_layer_id;
	if (cnt_layer_id >= ctx->layers_length)
	{
		//syslog(LOG_NOTICE, "overrun");
		kpu_kmodel_done(ctx);
		return -1;
	}

	ctx->current_layer++;
	ctx->current_body += cnt_layer_header->body_size;

#if KPU_DEBUG
	uint64_t time = sysctl_get_time_us();
	if (last_time != 0)
	{
		uint64_t layer_time = time - last_time;
		syslog(LOG_NOTICE, "layer %d/%d [%s]: %d.%03d ms", cnt_layer_id, ctx->layers_length, str_layer_type(last_layer_type), layer_time / 1000, layer_time % 1000);
		total_time += layer_time;
		if (last_layer_type == KL_K210_CONV)
			kpu_time += layer_time;
	}

	last_layer_type = cnt_layer_header->type;
	last_time = sysctl_get_time_us();
#endif

	switch (cnt_layer_header->type)
	{
	case KL_ADD:
		kpu_kmodel_add((const kpu_model_add_layer_argument_t *)layer_body, ctx);
		break;
	case KL_QUANTIZED_ADD:
		kpu_quantized_add((const kpu_model_quant_add_layer_argument_t *)layer_body, ctx);
		break;
	case KL_GLOBAL_AVERAGE_POOL2D:
		kpu_global_average_pool2d((const kpu_model_gap2d_layer_argument_t *)layer_body, ctx);
		break;
	case KL_QUANTIZED_MAX_POOL2D:
		kpu_quantized_max_pool2d((const kpu_model_quant_max_pool2d_layer_argument_t *)layer_body, ctx);
		break;
	case KL_AVERAGE_POOL2D:
		kpu_average_pool2d((const kpu_model_ave_pool2d_layer_argument_t *)layer_body, ctx);
		break;
	case KL_QUANTIZE:
		kpu_quantize((const kpu_model_quantize_layer_argument_t *)layer_body, ctx);
		break;
	case KL_DEQUANTIZE:
		kpu_kmodel_dequantize((const kpu_model_dequantize_layer_argument_t *)layer_body, ctx);
		break;
	case KL_REQUANTIZE:
		kpu_requantize((const kpu_model_requantize_layer_argument_t *)layer_body, ctx);
		break;
	case KL_L2_NORMALIZATION:
		kpu_l2_normalization((const kpu_model_l2_norm_layer_argument_t *)layer_body, ctx);
		break;
	case KL_SOFTMAX:
		kpu_softmax((const kpu_model_softmax_layer_argument_t *)layer_body, ctx);
		break;
	case KL_CONCAT:
	case KL_QUANTIZED_CONCAT:
		kpu_concat((const kpu_model_concat_layer_argument_t *)layer_body, ctx);
		break;
	case KL_FULLY_CONNECTED:
		kpu_kmodel_fully_connected((const kpu_model_fully_connected_layer_argument_t *)layer_body, ctx);
		break;
	case KL_TENSORFLOW_FLATTEN:
		kpu_tf_flatten((const kpu_model_tf_flatten_layer_argument_t *)layer_body, ctx);
		break;
	case KL_RESIZE_NEAREST_NEIGHBOR:
		kpu_resize_nearest_neighbor((const kpu_model_resize_nearest_neighbor_layer_argument_t *)layer_body, ctx);
		break;
	case KL_QUANTIZED_RESIZE_NEAREST_NEIGHBOR:
		kpu_quant_resize_nearest_neighbor((const kpu_model_quant_resize_nearest_neighbor_layer_argument_t *)layer_body, ctx);
		break;
	case KL_CHANNELWISE_DEQUANTIZE:
		kpu_kmodel_channelwise_dequantize((const kpu_model_channelwise_dequant_argument_t *)layer_body, ctx);
		break;
	case KL_LOGISTIC:
		kpu_logistic((const kpu_model_logistic_layer_argument_t *)layer_body, ctx);
		break;
	case KL_K210_CONV:
		kpu_conv((const kpu_model_conv_layer_argument_t *)layer_body, ctx);
		return 0;
	case KL_K210_ADD_PADDING:
		kpu_add_padding((const kpu_model_add_padding_layer_argument_t *)layer_body, ctx);
		break;
	case KL_K210_REMOVE_PADDING:
		kpu_remove_padding((const kpu_model_remove_padding_layer_argument_t *)layer_body, ctx);
		break;
	case KL_K210_UPLOAD:
		kpu_upload((const kpu_model_upload_layer_argument_t *)layer_body, ctx);
		break;
	default:
		assert(!"Layer is not supported.");
		kpu_kmodel_done(ctx);
		return -1;
	}

	if (ctx->current_layer < (ctx->layers_length - 1))
		ai_step(userdata);
	else
		kpu_kmodel_done(ctx);
	return 0;
}

static void ai_step_not_isr(void *userdata)
{
	dis_int(INTNO_DMAAI);
	dis_int(INTNO_AI);

	ai_step(userdata);

	ena_int(INTNO_DMAAI);
	ena_int(INTNO_AI);
}

int kpu_run_kmodel(kpu_model_context_t *ctx, const uint8_t *src, dmac_channel_number_t dma_ch, kpu_done_callback_t done_callback, void *userdata)
{
	ctx->dma_ch = dma_ch;
	ctx->done_callback = done_callback;
	ctx->userdata = userdata;
	ctx->current_layer = 0;
	ctx->current_body = ctx->body_start;
#if KPU_DEBUG
	last_time = 0;
	total_time = 0;
	kpu_time = 0;
#endif

	kpu_kmodel_header_t *header = (kpu_kmodel_header_t *)ctx->model_buffer;
	kpu->interrupt_clear.reg = 7;
	kpu->fifo_threshold.data = (kpu_config_fifo_threshold_t){
		.fifo_full_threshold = 10, .fifo_empty_threshold = 1};
	kpu->eight_bit_mode.data = (kpu_config_eight_bit_mode_t){
		.eight_bit_mode = header->flags & 1};
	kpu->interrupt_mask.data = (kpu_config_interrupt_t){
		.calc_done_int = 1,
		.layer_cfg_almost_empty_int = 0,
		.layer_cfg_almost_full_int = 1};

	//plic_set_priority(INTNO_AI, 1);
	plic_irq_register(INTNO_AI, ai_step, ctx);
	plic_irq_enable(INTNO_AI);

	const kpu_model_layer_header_t *first_layer_header = ctx->layer_headers;

	switch (first_layer_header->type)
	{
	case KL_K210_CONV:
	{
		const kpu_model_conv_layer_argument_t *first_layer = (const kpu_model_conv_layer_argument_t *)ctx->body_start;
		kpu_layer_argument_t layer_arg = *(volatile kpu_layer_argument_t *)(ctx->model_buffer + first_layer->layer_offset);

		if ((layer_arg.image_size.data.i_row_wid + 1) % 64 != 0)
		{
			kpu_kmodel_input_with_padding(&layer_arg, src);
			ai_step_not_isr(ctx);
		}
		else
		{
			kpu_input_dma(&layer_arg, src, ctx->dma_ch, ai_step, ctx);
		}
	}
	break;
	case KL_FULLY_CONNECTED:
	{
		const kpu_model_fully_connected_layer_argument_t *first_layer = (const kpu_model_fully_connected_layer_argument_t *)ctx->body_start;
		kpu_kmodel_input_float((const float *)src, (float *)(ctx->main_buffer + first_layer->main_mem_in_address), first_layer->in_channels);
		ai_step_not_isr(ctx);
	}
	break;
	default:
		return -1;
	}

	return 0;
}

ER kpu_init(kpu_model_context_t *ctx)
{
	g_ai_hdma.chnum = AI_DMA_CH;
	g_ai_hdma.xfercallback = ai_dma_done_isr;
	g_ai_hdma.errorcallback = NULL;
	g_ai_hdma.Init.Request = DMA_SELECT_AI_RX_REQ;		/* DMA選択 */
	g_ai_hdma.Init.Direction = DMA_PERIPH_TO_MEMORY;	/* DMA転送方向 */
	g_ai_hdma.Init.SrcMultBlock = DMAC_MULTBLOCK_CONT;	/* ソースマルチブロックタイプ */
	g_ai_hdma.Init.DrcMultBlock = DMAC_MULTBLOCK_CONT;	/* デスティネーションマルチブロックタイプ */
	g_ai_hdma.Init.SrcHandShake = DMAC_HS_HARDWARE;		/* ソースハンドシェイク */
	g_ai_hdma.Init.DrcHandShake = DMAC_HS_SOFTWARE;		/* デスティネーションハンドシェイク */
	g_ai_hdma.Init.SrcHwhsPol = DMAC_HWHS_POLARITY_LOW; /* ソースハードウェアハンドシェイク極性 */
	g_ai_hdma.Init.DrcHwhsPol = DMAC_HWHS_POLARITY_LOW; /* デスティネーションハードウェアハンドシェイク極性 */
	g_ai_hdma.Init.Priority = 4;						/* 優先度 */
	g_ai_hdma.Init.SrcMaster = DMAC_MASTER1;			/* ソースマスター設定 */
	g_ai_hdma.Init.DstMaster = DMAC_MASTER2;			/* デスティネーションマスター設定 */
	g_ai_hdma.Init.SrcInc = DMAC_ADDR_NOCHANGE;			/* ソースインクリメント設定 */
	g_ai_hdma.Init.DstInc = DMAC_ADDR_INCREMENT;		/* デスティネーションインクリメント設定 */
	g_ai_hdma.Init.SrcTransWidth = DMAC_TRANS_WIDTH_32; /* ソース転送幅 */
	g_ai_hdma.Init.DstTransWidth = DMAC_TRANS_WIDTH_32; /* デスティネーション転送幅 */
	g_ai_hdma.Init.SrcBurstSize = DMAC_MSIZE_4;			/* ソースバーストサイズ */
	g_ai_hdma.Init.DstBurstSize = DMAC_MSIZE_4;			/* デスティネーションバーストサイズ */
	g_ai_hdma.Init.IocBlkTrans = 0;						/* IOCブロック転送 */
	g_ai_hdma.localdata = (void *)ctx;

	return dma_init(&g_ai_hdma);
}
