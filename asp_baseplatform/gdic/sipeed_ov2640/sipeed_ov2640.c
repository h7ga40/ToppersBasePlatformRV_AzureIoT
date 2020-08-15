/*
 *  TOPPERS/ASP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Advanced Standard Profile Kernel
 * 
 *  Copyright (C) 2000-2003 by Embedded and Real-Time Systems Laboratory
 *                              Toyohashi Univ. of Technology, JAPAN
 *  Copyright (C) 2004-2012 by Embedded and Real-Time Systems Laboratory
 *              Graduate School of Information Science, Nagoya Univ., JAPAN
 *  Copyright (C) 2015-2019 by TOPPERS PROJECT Educational Working Group.
 * 
 *  上記著作権者は，以下の(1)～(4)の条件を満たす場合に限り，本ソフトウェ
 *  ア（本ソフトウェアを改変したものを含む．以下同じ）を使用・複製・改
 *  変・再配布（以下，利用と呼ぶ）することを無償で許諾する．
 *  (1) 本ソフトウェアをソースコードの形で利用する場合には，上記の著作
 *      権表示，この利用条件および下記の無保証規定が，そのままの形でソー
 *      スコード中に含まれていること．
 *  (2) 本ソフトウェアを，ライブラリ形式など，他のソフトウェア開発に使
 *      用できる形で再配布する場合には，再配布に伴うドキュメント（利用
 *      者マニュアルなど）に，上記の著作権表示，この利用条件および下記
 *      の無保証規定を掲載すること．
 *  (3) 本ソフトウェアを，機器に組み込むなど，他のソフトウェア開発に使
 *      用できない形で再配布する場合には，次のいずれかの条件を満たすこ
 *      と．
 *    (a) 再配布に伴うドキュメント（利用者マニュアルなど）に，上記の著
 *        作権表示，この利用条件および下記の無保証規定を掲載すること．
 *    (b) 再配布の形態を，別に定める方法によって，TOPPERSプロジェクトに
 *        報告すること．
 *  (4) 本ソフトウェアの利用により直接的または間接的に生じるいかなる損
 *      害からも，上記著作権者およびTOPPERSプロジェクトを免責すること．
 *      また，本ソフトウェアのユーザまたはエンドユーザからのいかなる理
 *      由に基づく請求からも，上記著作権者およびTOPPERSプロジェクトを
 *      免責すること．
 * 
 *  本ソフトウェアは，無保証で提供されているものである．上記著作権者お
 *  よびTOPPERSプロジェクトは，本ソフトウェアに関して，特定の使用目的
 *  に対する適合性も含めて，いかなる保証も行わない．また，本ソフトウェ
 *  アの利用により直接的または間接的に生じたいかなる損害に関しても，そ
 *  の責任を負わない．
 * 
 *  $Id$
 */
/* 
 *  SIPEED OV2640 CAMARA制御プログラムの本体
 */

#include <kernel.h>
#include <t_syslog.h>
#include <target_syssvc.h>
#include "device.h"

#include "dvp.h"
#include "ov2640_regs.h"
#include "stdlib.h"
#include "math.h"
#include "sipeed_ov2640.h"


#define IM_MAX(a,b)     ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a > _b ? _a : _b; })
#define IM_MIN(a,b)     ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a < _b ? _a : _b; })
#define IM_DIV(a,b)     ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _b ? (_a / _b) : 0; })
#define IM_MOD(a,b)     ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _b ? (_a % _b) : 0; })

#ifndef SNAPSHOT_TIMEOUT
#define SNAPDHOT_TIMEOUT    300
#endif


#define SVGA_HSIZE     (800)
#define SVGA_VSIZE     (600)

#define UXGA_HSIZE     (1600)
#define UXGA_VSIZE     (1200)
static const uint8_t ov2640_default[][2] = { //k210 
	{0xff, 0x01},
	{0x12, 0x80},
	{0xff, 0x00},
	{0x2c, 0xff},
	{0x2e, 0xdf},
	{0xff, 0x01},
	{0x3c, 0x32},
	{0x11, 0x00},
	{0x09, 0x02},
	{0x04, 0xa8},//v flip and h mirro
	{0x13, 0xe5},
	{0x14, 0x48},
	{0x2c, 0x0c},
	{0x33, 0x78},
	{0x3a, 0x33},
	{0x3b, 0xfb},
	{0x3e, 0x00},
	{0x43, 0x11},
	{0x16, 0x10},
	{0x39, 0x92},//test 92
	{0x23, 0x00},
	{0x36, 0x1a},
	{0x07, 0xc0},
	{0x4c, 0x00},
	{0x48, 0x00},
	{0x5b, 0x00},
	{0x4a, 0x81},
	{0x21, 0x99},
	{0x24, 0x40},
	{0x25, 0x38},
	{0x26, 0x82},
	{0x5c, 0x00},
	{0x63, 0x00},
	{0x46, 0x22},
	{0x0c, 0x3c},
	{0x61, 0x70},
	{0x62, 0x80},
	{0x7c, 0x05},
	{0x20, 0x80},
	{0x28, 0x30},
	{0x6c, 0x00},
	{0x6d, 0x80},
	{0x6e, 0x00},
	{0x70, 0x02},
	{0x71, 0x94},
	{0x73, 0xc1},
	{0x5a, 0x57},
	{0x37, 0xc0},
	{0x4f, 0xca},
	{0x50, 0xa8},
	{0x5a, 0x23},
	{0x6d, 0x00},
	{0x3d, 0x38},
	{0xff, 0x00},
	{0xe5, 0x7f},
	{0xf9, 0xc0},
	{0x41, 0x24},
	{0xe0, 0x14},
	{0x76, 0xff},
	{0x33, 0xa0},
	{0x42, 0x20},
	{0x43, 0x18},
	{0x4c, 0x00},
	{0x87, 0xd5},
	{0x88, 0x3f},
	{0xd7, 0x03},//[pixformat]:
	{0xd9, 0x10},
	{0xd3, 0x82},
	{0xc8, 0x08},
	{0xc9, 0x80},
	{0x7c, 0x00},
	{0x7d, 0x00},
	{0x7c, 0x03},
	{0x7d, 0x48},
	{0x7d, 0x48},
	{0x7c, 0x08},
	{0x7d, 0x20},
	{0x7d, 0x10},
	{0x7d, 0x0e},
	{0x90, 0x00},
	{0x91, 0x0e},
	{0x91, 0x1a},
	{0x91, 0x31},
	{0x91, 0x5a},
	{0x91, 0x69},
	{0x91, 0x75},
	{0x91, 0x7e},
	{0x91, 0x88},
	{0x91, 0x8f},
	{0x91, 0x96},
	{0x91, 0xa3},
	{0x91, 0xaf},
	{0x91, 0xc4},
	{0x91, 0xd7},
	{0x91, 0xe8},
	{0x91, 0x20},
	{0x92, 0x00},
	{0x93, 0x06},
	{0x93, 0xe3},
	{0x93, 0x05},
	{0x93, 0x05},
	{0x93, 0x00},
	{0x93, 0x04},
	{0x93, 0x00},
	{0x93, 0x00},
	{0x93, 0x00},
	{0x93, 0x00},
	{0x93, 0x00},
	{0x93, 0x00},
	{0x93, 0x00},
	{0x96, 0x00},
	{0x97, 0x08},
	{0x97, 0x19},
	{0x97, 0x02},
	{0x97, 0x0c},
	{0x97, 0x24},
	{0x97, 0x30},
	{0x97, 0x28},
	{0x97, 0x26},
	{0x97, 0x02},
	{0x97, 0x98},
	{0x97, 0x80},
	{0x97, 0x00},
	{0x97, 0x00},
	{0xc3, 0xed},
	{0xa4, 0x00},
	{0xa8, 0x00},
	{0xc5, 0x11},
	{0xc6, 0x51},
	{0xbf, 0x80},
	{0xc7, 0x10},
	{0xb6, 0x66},
	{0xb8, 0xa5},
	{0xb7, 0x64},
	{0xb9, 0x7c},
	{0xb3, 0xaf},
	{0xb4, 0x97},
	{0xb5, 0xff},
	{0xb0, 0xc5},
	{0xb1, 0x94},
	{0xb2, 0x0f},
	{0xc4, 0x5c},
	{0x5a, 0xc8},
	{0x5b, 0x96},
	{0x5c, 0x00},
	{0xc3, 0xed},
	{0x7f, 0x00},
	{0xda, 0x08},//pixformat
	{0xe5, 0x1f},
	{0xe1, 0x67},//pixformat
	{0xe0, 0x00},
	{0xdd, 0x7f},
	{0x05, 0x00},
#if 1	//color bar
	{0xff, 0x01},
	{0x12, 0x02},
#endif
	{0x00, 0x00}

};
static const uint8_t svga_config[][2] = { //k210 
	{0xff, 0x01},//bank sel
	{0x35, 0xda},//[SVGA]:
	{0x22, 0x1a},//[SVGA]:
	{0x37, 0xc3},//[SVGA]:
	{0x34, 0xc0},//[SVGA]:
	{0x06, 0x88},//[SVGA]:
	{0x0d, 0x87},//[SVGA]:
	{0x0e, 0x41},//[SVGA]:
	{0x42, 0x03},//[SVGA]:
	{0x3d, 0x34},//[SVGA]:
	{0x12, 0x40},//[SVGA]:  COM7,COM7_RES_SVGA  SVGA
	{0x03, 0x0f},//[SVGA]:  COM1,0x0F  
	{0x17, 0x11},//[SVGA]:HSTART
	{0x18, 0x43},//[SVGA]:HSTOP
	{0x19, 0x00},//[SVGA]:VSTART
	{0x1a, 0x4b},//[SVGA]:VSTOP
	{0x32, 0x09},//[SVGA]:REG32
	
	{0xff, 0x00},//bank sel
	{0xc0, 0x64},//[SVGA]:HSIZE8 SVGA_HSIZE>>3
	{0xc1, 0x4b},//[SVGA]:VSIZE8 SVGA_VSIZE>>3
	{0x8c, 0x00},//[SVGA]:SIZEL
	{0x86, 0x3d},//[SVGA]:
	{0x50, 0x00},//[SVGA]:CTRLI
	{0x51, 0xc8},//[SVGA]:HSIZE
	{0x52, 0x96},//[SVGA]:VSIZE
	{0x53, 0x00},//[SVGA]:XOFFL
	{0x54, 0x00},//[SVGA]:YOFFL
	{0x55, 0x00},//[SVGA]:VHYX
	{0xd3, 0x02},//[SVGA]:R_DVP_SP	
};

static const uint8_t uxga_regs[][2] = {
        { BANK_SEL, BANK_SEL_SENSOR },
        /* DSP input image resoultion and window size control */
        { COM7,    COM7_RES_UXGA},
        { COM1,    0x0F }, /* UXGA=0x0F, SVGA=0x0A, CIF=0x06 */
        { REG32,   0x36 }, /* UXGA=0x36, SVGA/CIF=0x09 */

        { HSTART,  0x11 }, /* UXGA=0x11, SVGA/CIF=0x11 */
        { HSTOP,   0x75 }, /* UXGA=0x75, SVGA/CIF=0x43 */

        { VSTART,  0x01 }, /* UXGA=0x01, SVGA/CIF=0x00 */
        { VSTOP,   0x97 }, /* UXGA=0x97, SVGA/CIF=0x4b */
        { 0x3d,    0x34 }, /* UXGA=0x34, SVGA/CIF=0x38 */

        { 0x35,    0x88 },
        { 0x22,    0x0a },
        { 0x37,    0x40 },
        { 0x34,    0xa0 },
        { 0x06,    0x02 },
        { 0x0d,    0xb7 },
        { 0x0e,    0x01 },
        { 0x42,    0x83 },

        /* Set DSP input image size and offset.
           The sensor output image can be scaled with OUTW/OUTH */
        { BANK_SEL, BANK_SEL_DSP },
        { R_BYPASS, R_BYPASS_DSP_BYPAS },

        { RESET,   RESET_DVP },
        { HSIZE8,  (UXGA_HSIZE>>3)}, /* Image Horizontal Size HSIZE[10:3] */
        { VSIZE8,  (UXGA_VSIZE>>3)}, /* Image Vertiacl Size VSIZE[10:3] */

        /* {HSIZE[11], HSIZE[2:0], VSIZE[2:0]} */
        { SIZEL,   ((UXGA_HSIZE>>6)&0x40) | ((UXGA_HSIZE&0x7)<<3) | (UXGA_VSIZE&0x7)},

        { XOFFL,   0x00 }, /* OFFSET_X[7:0] */
        { YOFFL,   0x00 }, /* OFFSET_Y[7:0] */
        { HSIZE,   ((UXGA_HSIZE>>2)&0xFF) }, /* H_SIZE[7:0] real/4 */
        { VSIZE,   ((UXGA_VSIZE>>2)&0xFF) }, /* V_SIZE[7:0] real/4 */

        /* V_SIZE[8]/OFFSET_Y[10:8]/H_SIZE[8]/OFFSET_X[10:8] */
        { VHYX,    ((UXGA_VSIZE>>3)&0x80) | ((UXGA_HSIZE>>7)&0x08) },
        { TEST,    (UXGA_HSIZE>>4)&0x80}, /* H_SIZE[9] */

        { CTRL2,   CTRL2_DCW_EN | CTRL2_SDE_EN |
            CTRL2_UV_AVG_EN | CTRL2_CMX_EN | CTRL2_UV_ADJ_EN },

        /* H_DIVIDER/V_DIVIDER */
        { CTRLI,   CTRLI_LP_DP | 0x00},
        /* DVP prescalar */
        { R_DVP_SP, R_DVP_SP_AUTO_MODE | 0x04},

        { R_BYPASS, R_BYPASS_DSP_EN },
        { RESET,    0x00 },
        {0, 0},
};

static const uint8_t yuv422_regs[][2] = {
        { BANK_SEL, BANK_SEL_DSP },
        { RESET,   RESET_DVP},
        { 0xD7,     0x01 },
        { IMAGE_MODE, IMAGE_MODE_YUV422 },
        { 0xE1,     0x67 },
        { RESET,    0x00 },
        {0, 0},
};

static const uint8_t rgb565_regs[][2] = {
        { BANK_SEL,   BANK_SEL_DSP },
        { RESET,      RESET_DVP},
        { 0xD7,       0x03},
        { IMAGE_MODE, IMAGE_MODE_RGB565 },
        { 0xE1,       0x77 },
        { RESET,      0x00 },
        {0,           0},
};

static const uint8_t jpeg_regs[][2] = {
        { BANK_SEL, BANK_SEL_DSP },
        { RESET,   RESET_DVP},
        { IMAGE_MODE, IMAGE_MODE_JPEG_EN|IMAGE_MODE_RGB565 },
        { 0xD7,     0x03 },
        { 0xE1,     0x77 },
        { QS,       0x0C },
        { RESET,    0x00 },
        {0, 0},
};

#define NUM_BRIGHTNESS_LEVELS (5)
static const uint8_t brightness_regs[NUM_BRIGHTNESS_LEVELS + 1][5] = {
    { BPADDR, BPDATA, BPADDR, BPDATA, BPDATA },
    { 0x00, 0x04, 0x09, 0x00, 0x00 }, /* -2 */
    { 0x00, 0x04, 0x09, 0x10, 0x00 }, /* -1 */
    { 0x00, 0x04, 0x09, 0x20, 0x00 }, /*  0 */
    { 0x00, 0x04, 0x09, 0x30, 0x00 }, /* +1 */
    { 0x00, 0x04, 0x09, 0x40, 0x00 }, /* +2 */
};

#define NUM_CONTRAST_LEVELS (5)
static const uint8_t contrast_regs[NUM_CONTRAST_LEVELS + 1][7] = {
    { BPADDR, BPDATA, BPADDR, BPDATA, BPDATA, BPDATA, BPDATA },
    { 0x00, 0x04, 0x07, 0x20, 0x18, 0x34, 0x06 }, /* -2 */
    { 0x00, 0x04, 0x07, 0x20, 0x1c, 0x2a, 0x06 }, /* -1 */
    { 0x00, 0x04, 0x07, 0x20, 0x20, 0x20, 0x06 }, /*  0 */
    { 0x00, 0x04, 0x07, 0x20, 0x24, 0x16, 0x06 }, /* +1 */
    { 0x00, 0x04, 0x07, 0x20, 0x28, 0x0c, 0x06 }, /* +2 */
};

#define NUM_SATURATION_LEVELS (5)
static const uint8_t saturation_regs[NUM_SATURATION_LEVELS + 1][5] = {
    { BPADDR, BPDATA, BPADDR, BPDATA, BPDATA },
    { 0x00, 0x02, 0x03, 0x28, 0x28 }, /* -2 */
    { 0x00, 0x02, 0x03, 0x38, 0x38 }, /* -1 */
    { 0x00, 0x02, 0x03, 0x48, 0x48 }, /*  0 */
    { 0x00, 0x02, 0x03, 0x58, 0x58 }, /* +1 */
    { 0x00, 0x02, 0x03, 0x58, 0x58 }, /* +2 */
};

static const int resolution[][2] = {
    {0,    0   },
    // C/SIF Resolutions
    {88,   72  },    /* QQCIF     */
    {176,  144 },    /* QCIF      */
    {352,  288 },    /* CIF       */
    {88,   60  },    /* QQSIF     */
    {176,  120 },    /* QSIF      */
    {352,  240 },    /* SIF       */
    // VGA Resolutions
    {40,   30  },    /* QQQQVGA   */
    {80,   60  },    /* QQQVGA    */
    {160,  120 },    /* QQVGA     */
    {320,  240 },    /* QVGA      */
    {640,  480 },    /* VGA       */
    {60,   40  },    /* HQQQVGA   */
    {120,  80  },    /* HQQVGA    */
    {240,  160 },    /* HQVGA     */
    // FFT Resolutions
    {64,   32  },    /* 64x32     */
    {64,   64  },    /* 64x64     */
    {128,  64  },    /* 128x64    */
    {128,  128 },    /* 128x64    */
    // Other
    {128,  160 },    /* LCD       */
    {128,  160 },    /* QQVGA2    */
    {720,  480 },    /* WVGA      */
    {752,  480 },    /* WVGA2     */
    {800,  600 },    /* SVGA      */
    {1280, 1024},    /* SXGA      */
    {1600, 1200},    /* UXGA      */
};


#define cambus_writeb(h, a, d)  dvp_sccb_send_data(((h)->hdvp), ((h)->_slaveAddr), (a), (d))

int Sipeed_OV2640_cambus_scan_gc0328(OV2640_t *hcmr);
int Sipeed_OV2640_dvpInitIrq(void);

static int cambus_readb(OV2640_t *hcmr, uint8_t reg_addr, uint8_t *reg_data)
{

    int ret = 0;
	*reg_data = dvp_sccb_receive_data(hcmr->hdvp, hcmr->_slaveAddr, reg_addr);
	if(0xff == *reg_data)
		ret = -1;
    return ret;
}

static int reverse_u32pixel(uint32_t* addr, uint32_t length)
{
  if(NULL == addr)
    return -1;

  uint32_t data;
  uint32_t* pend = addr+length;
  for(;addr<pend;addr++)
  {
	  data = *(addr);
	  *(addr) = ((data & 0x000000FF) << 24) | ((data & 0x0000FF00) << 8) | 
                ((data & 0x00FF0000) >> 8) | ((data & 0xFF000000) >> 24) ;
  }  //1.7ms
  
  
  return 0;
}

static int cambus_read_id(OV2640_t *hcmr, uint8_t addr, uint16_t *manuf_id, uint16_t *device_id)
{
	dvp_sccb_send_data(hcmr->hdvp, addr, 0xFF, 0x01);
	*manuf_id = (dvp_sccb_receive_data(hcmr->hdvp, addr, 0x1C) << 8) | dvp_sccb_receive_data(hcmr->hdvp, addr, 0x1D);
	*device_id = (dvp_sccb_receive_data(hcmr->hdvp, addr, 0x0A) << 8) | dvp_sccb_receive_data(hcmr->hdvp, addr, 0x0B);
	return 0;
}

static int cambus_scan(OV2640_t *hcmr)
{

	uint16_t manuf_id = 0;
	uint16_t device_id = 0;
    for (uint8_t addr=0x08; addr<=0x77; addr++) {
		cambus_read_id(hcmr, addr ,&manuf_id,&device_id);
		if(0xffff != device_id)
		{
			return addr ;
		}
    }
    return 0;
}


void
ov2640_getResolition(OV2640_t *hcmr, framesize_t frameSize)
{
	hcmr->_width    = resolution[frameSize][0];
	hcmr->_height   = resolution[frameSize][1];
}

ER
ov2640_sensor_ov_detect(OV2640_t *hcmr)
{
	DVP_Handle_t *hdvp = hcmr->hdvp;

	/* Reset the sensor */
	dvp_dcmi_reset(hdvp, true);
	dly_tsk(10);

	dvp_dcmi_reset(hdvp, false);
	dly_tsk(10);

	/* Probe the ov sensor */
	hcmr->_slaveAddr = cambus_scan(hcmr);
	if(hcmr->_slaveAddr == 0){
		/* Sensor has been held in reset,
		   so the reset line is active low */
		hcmr->_resetPoliraty = ACTIVE_LOW;

		/* Pull the sensor out of the reset state,systick_sleep() */
		dvp_dcmi_reset(hdvp, true);
		dly_tsk(10);

		/* Probe again to set the slave addr */
		hcmr->_slaveAddr = cambus_scan(hcmr);
		if(hcmr->_slaveAddr == 0){
			hcmr->_pwdnPoliraty = ACTIVE_LOW;
			dvp_dcmi_powerdown(hdvp, false);
			dly_tsk(10);

			hcmr->_slaveAddr = cambus_scan(hcmr);
			if(hcmr->_slaveAddr == 0){
				hcmr->_resetPoliraty = ACTIVE_HIGH;
				dvp_dcmi_reset(hdvp, false);
				dly_tsk(10);

				hcmr->_slaveAddr = cambus_scan(hcmr);
				if(hcmr->_slaveAddr == 0){
					//should do something?
					return E_SYS;
				}
			}
		}
	}

	// Clear sensor chip ID.
	hcmr->_id = 0;

	if(hcmr->_slaveAddr == LEPTON_ID){
		hcmr->_id = LEPTON_ID;
		/*set LEPTON xclk rate*/
		/*lepton_init*/
	}
	else{
		// Read ON semi sensor ID.
		cambus_readb(hcmr, ON_CHIP_ID, &hcmr->_id);
		if(hcmr->_id == MT9V034_ID){
			/*set MT9V034 xclk rate*/
			/*mt9v034_init*/
		}
		else{	// Read OV sensor ID.
			cambus_readb(hcmr, OV_CHIP_ID, &hcmr->_id);
			// Initialize sensor struct.
			switch(hcmr->_id){
			case OV9650_ID:
				/*ov9650_init*/
				break;
			case OV2640_ID:
				// printf("detect ov2640, id:%x\n", _slaveAddr);
				break;
			case OV7725_ID:
				/*ov7725_init*/
				break;
			default:
				// Sensor is not supported.
				return E_SYS;
			}
		}
	}
	return E_OK;
}

ER
ov2640_sensro_gc_detect(OV2640_t *hcmr)
{
	DVP_Handle_t *hdvp = hcmr->hdvp;
	uint8_t id;

	dvp_dcmi_powerdown(hdvp, false);//enable gc0328 要恢? normal 工作模式，需将 PWDN pin 接入低?平即可，同?写入初始化寄存器即可
	dvp_dcmi_reset(hdvp, false);	//reset gc3028
	dly_tsk(10);
	dvp_dcmi_reset(hdvp, true);
	dly_tsk(10);
	id = ov2640_cambus_scan_gc0328(hcmr);
	if(0 == id){
		return E_SYS;
	}
	else{
		// printf("[MAIXPY]: gc0328 id = %x\n",id); 
		hcmr->_slaveAddr = GC0328_ADDR;
		hcmr->_id = id;
	}
	return E_OK;
}


ER
ov2640_reset(OV2640_t *hcmr)
{
	int i=0;
	const uint8_t (*regs)[2];

	/* Reset all registers */
	cambus_writeb(hcmr, BANK_SEL, BANK_SEL_SENSOR);
	cambus_writeb(hcmr, COM7, COM7_SRST);

	/* delay n ms */
	dly_tsk(10);

	i = 0;
	regs = ov2640_default;
	/* Write initial regsiters */
	while (regs[i][0]) {
		cambus_writeb(hcmr, regs[i][0], regs[i][1]);
		i++;
	}
	i = 0;
	regs = svga_config;
	/* Write DSP input regsiters */
	while (regs[i][0]) {
		cambus_writeb(hcmr, regs[i][0], regs[i][1]);
		i++;
	}
	return E_OK;
}

ER
ov2640_set_pixformat(OV2640_t *hcmr)
{
	int i=0;
	const uint8_t (*regs)[2]=NULL;

	/* read pixel format reg */
	switch(hcmr->pixFormat){
	case PIXFORMAT_RGB565:
		regs = rgb565_regs;
		break;
	case PIXFORMAT_YUV422:
	case PIXFORMAT_GRAYSCALE:
		regs = yuv422_regs;
		break;
	case PIXFORMAT_JPEG:
		regs = jpeg_regs;
		break;
	default:
		return E_PAR;
	}

	/* Write initial regsiters */
	while(regs[i][0]){
		cambus_writeb(hcmr, regs[i][0], regs[i][1]);
		i++;
	}
	switch(hcmr->pixFormat) {
	case PIXFORMAT_RGB565:
		hcmr->hdvp->Init.Format = DVP_FORMAT_RGB;
		break;
	case PIXFORMAT_YUV422:
		hcmr->hdvp->Init.Format = DVP_FORMAT_YUY;
		break;
	case PIXFORMAT_GRAYSCALE:
		hcmr->hdvp->Init.Format = DVP_FORMAT_Y;
		break;
	case PIXFORMAT_JPEG:
		hcmr->hdvp->Init.Format = DVP_FORMAT_RGB;
		break;
	default:
		return E_PAR;
	}
	dvp_set_image_format(hcmr->hdvp);
	/* delay n ms */
	dly_tsk(30);
	return E_OK;
}

ER
ov2640_set_framesize(OV2640_t *hcmr)
{
	uint8_t clkrc;
	uint16_t w = hcmr->_width;
	uint16_t h = hcmr->_height;
	int i=0;
	const uint8_t (*regs)[2];

	if((w <= 800) && (h <= 600)){
		clkrc =0x80;
		regs = svga_config;
//		regs = ov2640_config;
	}
	else{
		clkrc =0x81;
		regs = uxga_regs;
	}

	/* Disable DSP */
	cambus_writeb(hcmr, BANK_SEL, BANK_SEL_DSP);
	cambus_writeb(hcmr, R_BYPASS, R_BYPASS_DSP_BYPAS);

	/* Set CLKRC */
	if(clkrc == 0x81){
		cambus_writeb(hcmr, BANK_SEL, BANK_SEL_SENSOR);
		cambus_writeb(hcmr, CLKRC, clkrc);
	}

	/* Write DSP input regsiters */
	while(regs[i][0]){
		cambus_writeb(hcmr, regs[i][0], regs[i][1]);
		i++;
	}

	 /* Write output width */
	cambus_writeb(hcmr,0xe0,0x04 ); /* OUTH[8]/OUTW[9:8] */
	cambus_writeb(hcmr, ZMOW, (w>>2)&0xFF); /* OUTW[7:0] (real/4) */
	cambus_writeb(hcmr, ZMOH, (h>>2)&0xFF); /* OUTH[7:0] (real/4) */
	cambus_writeb(hcmr, ZMHH, ((h>>8)&0x04)|((w>>10)&0x03)); /* OUTH[8]/OUTW[9:8] */
	cambus_writeb(hcmr,0xe0,0x00 ); /* OUTH[8]/OUTW[9:8] */

	/* Enable DSP */
	cambus_writeb(hcmr, BANK_SEL, BANK_SEL_DSP);
	cambus_writeb(hcmr, R_BYPASS, R_BYPASS_DSP_EN);

	/* delay n ms */
	dly_tsk(30);
	hcmr->hdvp->Init.Width  = w;
	hcmr->hdvp->Init.Height = h;
	return dvp_set_image_size(hcmr->hdvp);
}


ER
ov2640_activate(OV2640_t *hcmr, bool_t run)
{
	return dvp_activate(hcmr->hdvp, run);
}

int
ov2640_id(OV2640_t *hcmr)
{
    return hcmr->_id;
}

ER
ov2640_snapshot(OV2640_t *hcmr)
{
	DVP_Handle_t *hdvp = hcmr->hdvp;
	int32_t timeout = SNAPDHOT_TIMEOUT;

	//wait for new frame
	hdvp->state = DVP_STATE_ACTIVATE;

	while(hdvp->state != DVP_STATE_FINISH){
		if(--timeout <= 0)
			return E_TMOUT;
		if(hdvp->semid != 0)
			twai_sem(hdvp->semid, 1);
		else
			dly_tsk(1);
	}
	reverse_u32pixel((uint32_t*)hcmr->_dataBuffer, hcmr->_width * hcmr->_height/2);
	return E_OK;
}

ER
ov2640_cambus_scan_gc0328(OV2640_t *hcmr)
{
	uint8_t id;

	dvp_sccb_send_data(hcmr->hdvp, GC0328_ADDR, 0xFE, 0x00);
	id = dvp_sccb_receive_data(hcmr->hdvp, GC0328_ADDR, 0xf0);
	if(id != 0x9d){
		return 0;
	}
	return id;
}


ER
ov2640_setInvert(OV2640_t *hcmr, bool_t invert)
{
	uint8_t reg;

	cambus_readb(hcmr, BANK_SEL, &reg);
	cambus_writeb(hcmr, BANK_SEL, reg | BANK_SEL_SENSOR);
	cambus_readb(hcmr, REG04, &reg);

	if(invert){
		reg |= REG04_HFLIP_IMG;
	}
	else{
		reg &= ~REG04_HFLIP_IMG;
	}
	cambus_writeb(hcmr, REG04, reg);
    return E_OK;
}

ER
ov2640_set_contrast(OV2640_t *hcmr, int level)
{
	int i;

	level += (NUM_CONTRAST_LEVELS / 2 + 1);
	if(level < 0 || level > NUM_CONTRAST_LEVELS){
		return E_PAR;
	}

	/* Switch to DSP register bank */
	cambus_writeb(hcmr, BANK_SEL, BANK_SEL_DSP);

	/* Write contrast registers */
    for(i = 0 ; i < sizeof(contrast_regs[0])/sizeof(contrast_regs[0][0]) ; i++){
		cambus_writeb(hcmr, contrast_regs[0][i], contrast_regs[level][i]);
	}
	return E_OK;
}

ER
ov2640_set_brightness(OV2640_t *hcmr, int level)
{
	int i;

	level += (NUM_BRIGHTNESS_LEVELS / 2 + 1);
	if(level < 0 || level > NUM_BRIGHTNESS_LEVELS){
		return E_PAR;
	}

	/* Switch to DSP register bank */
	cambus_writeb(hcmr, BANK_SEL, BANK_SEL_DSP);

	/* Write brightness registers */
	for(i = 0 ; i < sizeof(brightness_regs[0])/sizeof(brightness_regs[0][0]) ; i++){
		cambus_writeb(hcmr, brightness_regs[0][i], brightness_regs[level][i]);
	}
	return E_OK;
}

ER
ov2640_set_saturation(OV2640_t *hcmr, int level)
{
	int i;

	level += (NUM_SATURATION_LEVELS / 2 + 1);
	if(level < 0 || level > NUM_SATURATION_LEVELS){
		return E_PAR;
	}

	/* Switch to DSP register bank */
	cambus_writeb(hcmr, BANK_SEL, BANK_SEL_DSP);

	/* Write contrast registers */
	for(i = 0 ; i < sizeof(saturation_regs[0])/sizeof(saturation_regs[0][0]) ; i++){
		cambus_writeb(hcmr, saturation_regs[0][i], saturation_regs[level][i]);
	}
	return E_OK;
}

ER
ov2640_set_gainceiling(OV2640_t *hcmr, gainceiling_t gainceiling)
{
	/* Switch to SENSOR register bank */
	cambus_writeb(hcmr, BANK_SEL, BANK_SEL_SENSOR);

	/* Write gain ceiling register */
	cambus_writeb(hcmr, COM9, COM9_AGC_SET(gainceiling));
	return E_OK;
}

ER
ov2640_set_quality(OV2640_t *hcmr, int qs)
{
	/* Switch to DSP register bank */
	cambus_writeb(hcmr, BANK_SEL, BANK_SEL_DSP);

	/* Write QS register */
	cambus_writeb(hcmr, QS, qs);
	return E_OK;
}

ER
ov2640_set_colorbar(OV2640_t *hcmr, bool_t enable)
{
	uint8_t reg;

    /* Switch to SENSOR register bank */
	cambus_writeb(hcmr, BANK_SEL, BANK_SEL_SENSOR);

	/* Update COM7 */
	cambus_readb(hcmr, COM7, &reg);

	if(enable){
		reg |= COM7_COLOR_BAR;
	}
	else{
		reg &= ~COM7_COLOR_BAR;
	}
	cambus_writeb(hcmr, COM7, reg);
	return E_OK;
}

ER
ov2640_set_auto_exposure(OV2640_t *hcmr, bool_t enable, int exposure_us)
{
	uint32_t freq = hcmr->hdvp->Init.Freq;
	uint8_t reg;
	int ret = 0;

	cambus_readb(hcmr, BANK_SEL, &reg);
	cambus_writeb(hcmr, BANK_SEL, reg | BANK_SEL_SENSOR);
	cambus_readb(hcmr, COM8, &reg);
	cambus_writeb(hcmr, COM8, COM8_SET_AEC(reg, enable));

	if((!enable) && (exposure_us >= 0)){
		int t_line = 0;
		int pll_mult, clk_rc, exposure;
		int t_pclk = 0;
		cambus_readb(hcmr, COM7, &reg);

		if (COM7_GET_RES(reg) == COM7_RES_UXGA) t_line = 1600 + 322;
		if (COM7_GET_RES(reg) == COM7_RES_SVGA) t_line = 800 + 390;
		if (COM7_GET_RES(reg) == COM7_RES_CIF) t_line = 400 + 195;

		ret |= cambus_readb(hcmr, CLKRC, &reg);
		pll_mult = (reg & CLKRC_DOUBLE) ? 2 : 1;
		clk_rc = ((reg & CLKRC_DIVIDER_MASK) + 1) * 2;

		ret |= cambus_readb(hcmr, BANK_SEL, &reg);
		cambus_writeb(hcmr, BANK_SEL, reg & (~BANK_SEL_SENSOR));
		ret |= cambus_readb(hcmr, IMAGE_MODE, &reg);

		if (IMAGE_MODE_GET_FMT(reg) == IMAGE_MODE_YUV422) t_pclk = 2;
		if (IMAGE_MODE_GET_FMT(reg) == IMAGE_MODE_RAW10) t_pclk = 1;
		if (IMAGE_MODE_GET_FMT(reg) == IMAGE_MODE_RGB565) t_pclk = 2;

		exposure = IM_MAX(IM_MIN(((exposure_us*(((freq/clk_rc)*pll_mult)/1000000))/t_pclk)/t_line,0xFFFF),0x0000);

		ret |= cambus_readb(hcmr, BANK_SEL, &reg);
		cambus_writeb(hcmr, BANK_SEL, reg | BANK_SEL_SENSOR);

		ret |= cambus_readb(hcmr, REG04, &reg);
		cambus_writeb(hcmr, REG04, (reg & 0xFC) | ((exposure >> 0) & 0x3));

		ret |= cambus_readb(hcmr, AEC, &reg);
		cambus_writeb(hcmr, AEC, (reg & 0x00) | ((exposure >> 2) & 0xFF));

		ret |= cambus_readb(hcmr, REG04, &reg);
		cambus_writeb(hcmr, REG04, (reg & 0xC0) | ((exposure >> 10) & 0x3F));
	}
	if(ret == 0)
		return E_OK;
	else
		return E_SYS;
}

ER
ov2640_get_exposure_us(OV2640_t *hcmr, int *exposure_us)
{
	uint32_t freq = hcmr->hdvp->Init.Freq;
	uint8_t reg, aec_10, aec_92, aec_1510;
	int t_line = 0;
	int t_pclk = 0;
	int pll_mult, clk_rc;
	uint16_t exposure;
	int ret = cambus_readb(hcmr, BANK_SEL, &reg);

	cambus_writeb(hcmr, BANK_SEL, reg | BANK_SEL_SENSOR);
	ret |= cambus_readb(hcmr, COM8, &reg);

    // DISABLED
    // if (reg & COM8_AEC_EN) {
    //     ret |= cambus_writeb(_slaveAddr, COM8, reg & (~COM8_AEC_EN));
    // }
    // DISABLED

	ret |= cambus_readb(hcmr, REG04, &aec_10);
	ret |= cambus_readb(hcmr, AEC, &aec_92);
	ret |= cambus_readb(hcmr, REG45, &aec_1510);

    // DISABLED
    // if (reg & COM8_AEC_EN) {
    //     ret |= cambus_writeb(_slaveAddr, COM8, reg | COM8_AEC_EN);
    // }
    // DISABLED

	ret |= cambus_readb(hcmr, COM7, &reg);

	if (COM7_GET_RES(reg) == COM7_RES_UXGA) t_line = 1600 + 322;
	if (COM7_GET_RES(reg) == COM7_RES_SVGA) t_line = 800 + 390;
	if (COM7_GET_RES(reg) == COM7_RES_CIF) t_line = 400 + 195;

	ret |= cambus_readb(hcmr, CLKRC, &reg);
	pll_mult = (reg & CLKRC_DOUBLE) ? 2 : 1;
	clk_rc = ((reg & CLKRC_DIVIDER_MASK) + 1) * 2;

	ret |= cambus_readb(hcmr, BANK_SEL, &reg);
	cambus_writeb(hcmr, BANK_SEL, reg & (~BANK_SEL_SENSOR));
	ret |= cambus_readb(hcmr, IMAGE_MODE, &reg);

	if (IMAGE_MODE_GET_FMT(reg) == IMAGE_MODE_YUV422) t_pclk = 2;
	if (IMAGE_MODE_GET_FMT(reg) == IMAGE_MODE_RAW10) t_pclk = 1;
	if (IMAGE_MODE_GET_FMT(reg) == IMAGE_MODE_RGB565) t_pclk = 2;

	exposure = ((aec_1510 & 0x3F) << 10) + ((aec_92 & 0xFF) << 2) + ((aec_10 & 0x3) << 0);
	*exposure_us = (exposure*t_line*t_pclk)/(((freq/clk_rc)*pll_mult)/1000000);
	if(ret == 0)
		return E_OK;
	else
		return E_SYS;
}

ER
ov2640_set_auto_whitebal(OV2640_t *hcmr, bool_t enable, float r_gain_db, float g_gain_db, float b_gain_db)
{
	uint8_t reg;
	int ret = cambus_readb(hcmr, BANK_SEL, &reg);
	cambus_writeb(hcmr, BANK_SEL, reg & (~BANK_SEL_SENSOR));
	ret |= cambus_readb(hcmr, CTRL1, &reg);
	cambus_writeb(hcmr, CTRL1, (reg & (~CTRL1_AWB)) | (enable ? CTRL1_AWB : 0));

	if((!enable) && (!isnanf(r_gain_db)) && (!isnanf(g_gain_db)) && (!isnanf(b_gain_db))
                      && (!isinff(r_gain_db)) && (!isinff(g_gain_db)) && (!isinff(b_gain_db))) {
    }
	if(ret == 0)
		return E_OK;
	else
		return E_SYS;
}

ER
ov2640_set_vflip(OV2640_t *hcmr, bool_t enable)
{
	uint8_t reg;
	int ret = cambus_readb(hcmr, BANK_SEL, &reg);
	cambus_writeb(hcmr, BANK_SEL, reg | BANK_SEL_SENSOR);
	ret |= cambus_readb(hcmr, REG04, &reg);

	if(enable){
		reg |= REG04_VFLIP_IMG;
		reg |= REG04_VREF_EN;
	}
	else{
		reg &= ~REG04_VFLIP_IMG;
		reg &= ~REG04_VREF_EN;
	}
	cambus_writeb(hcmr, REG04, reg);
	if(ret == 0)
		return E_OK;
	else
		return E_SYS;
}

#ifdef USE_GAIN
ER
ov2640_set_auto_gain(OV2640_t *hcmr, bool_t enable, float gain_db, float gain_db_ceiling)
{
	uint8_t reg;

	cambus_readb(hcmr, BANK_SEL, &reg);
	cambus_writeb(hcmr, BANK_SEL, reg | BANK_SEL_SENSOR);
	cambus_readb(hcmr, COM8, &reg);
	cambus_writeb(hcmr, COM8, (reg & (~COM8_AGC_EN)) | ((enable != 0) ? COM8_AGC_EN : 0));

	if((!enable) && (!isnanf(gain_db)) && (!isinff(gain_db))){
		float gain = IM_MAX(IM_MIN(expf((gain_db / 20.0) * log(10.0)), 32.0), 1.0);
		int gain_temp = roundf(log2(IM_MAX(gain / 2.0, 1.0)));
		int gain_hi = 0xF >> (4 - gain_temp);
		int gain_lo = IM_MIN(roundf(((gain / (1 << gain_temp)) - 1.0) * 16.0), 15);

		cambus_writeb(hcmr, GAIN, (gain_hi << 4) | (gain_lo << 0));
	}
	else if(enable && (!isnanf(gain_db_ceiling)) && (!isinff(gain_db_ceiling))){
		float gain_ceiling = IM_MAX(IM_MIN(expf((gain_db_ceiling / 20.0) * log(10.0)), 128.0), 2.0);

		cambus_readb(hcmr, COM9, &reg);
		cambus_writeb(hcmr, COM9, (reg & 0x1F) | (((int)ceilf(log2(gain_ceiling)) - 1) << 5));
	}
	return E_OK;
}

ER
ov2640_get_gain_db(OV2640_t *hcmr, float *gain_db)
{
    uint8_t reg, gain;
	int hi_gain;
	float lo_gain;

	cambus_readb(hcmr, BANK_SEL, &reg);
	cambus_writeb(hcmr, BANK_SEL, reg | BANK_SEL_SENSOR);
	cambus_readb(hcmr, COM8, &reg);

    // DISABLED
    // if (reg & COM8_AGC_EN) {
    //     ret |= cambus_writeb(_slaveAddr, COM8, reg & (~COM8_AGC_EN));
    // }
    // DISABLED

	cambus_readb(hcmr, GAIN, &gain);

    // DISABLED
    // if (reg & COM8_AGC_EN) {
    //     ret |= cambus_writeb(_slaveAddr, COM8, reg | COM8_AGC_EN);
    // }
    // DISABLED

	hi_gain = 1 << (((gain >> 7) & 1) + ((gain >> 6) & 1) + ((gain >> 5) & 1) + ((gain >> 4) & 1));
	lo_gain = 1.0 + (((gain >> 0) & 0xF) / 16.0);
	*gain_db = 20.0 * (log(hi_gain * lo_gain) / log(10.0));
	return E_OK;
}
#endif	/* USE_GAIN */

