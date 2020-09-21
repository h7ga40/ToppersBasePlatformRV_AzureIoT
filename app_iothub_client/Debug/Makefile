#
#  TOPPERS/ASP Kernel
#      Toyohashi Open Platform for Embedded Real-Time Systems/
#      Advanced Standard Profile Kernel
# 
#  Copyright (C) 2000-2003 by Embedded and Real-Time Systems Laboratory
#                              Toyohashi Univ. of Technology, JAPAN
#  Copyright (C) 2006-2019 by Embedded and Real-Time Systems Laboratory
#              Graduate School of Information Science, Nagoya Univ., JAPAN
# 
#  上記著作権者は，以下の(1)～(4)の条件を満たす場合に限り，本ソフトウェ
#  ア（本ソフトウェアを改変したものを含む．以下同じ）を使用・複製・改
#  変・再配布（以下，利用と呼ぶ）することを無償で許諾する．
#   (1) 本ソフトウェアをソースコードの形で利用する場合には，上記の著作
#       権表示，この利用条件および下記の無保証規定が，そのままの形でソー
#       スコード中に含まれていること．
#   (2) 本ソフトウェアを，ライブラリ形式など，他のソフトウェア開発に使
#       用できる形で再配布する場合には，再配布に伴うドキュメント（利用
#       者マニュアルなど）に，上記の著作権表示，この利用条件および下記
#       の無保証規定を掲載すること．
#   (3) 本ソフトウェアを，機器に組み込むなど，他のソフトウェア開発に使
#      用できない形で再配布する場合には，次のいずれかの条件を満たすこ
#      と．
#     (a) 再配布に伴うドキュメント（利用者マニュアルなど）に，上記の著
#         作権表示，この利用条件および下記の無保証規定を掲載すること．
#     (b) 再配布の形態を，別に定める方法によって，TOPPERSプロジェクトに
#        報告すること．
#   (4) 本ソフトウェアの利用により直接的または間接的に生じるいかなる損
#      害からも，上記著作権者およびTOPPERSプロジェクトを免責すること．
#      また，本ソフトウェアのユーザまたはエンドユーザからのいかなる理
#      由に基づく請求からも，上記著作権者およびTOPPERSプロジェクトを
#      免責すること．
# 
#  本ソフトウェアは，無保証で提供されているものである．上記著作権者お
#  よびTOPPERSプロジェクトは，本ソフトウェアに関して，特定の使用目的
#  に対する適合性も含めて，いかなる保証も行わない．また，本ソフトウェ
#  アの利用により直接的または間接的に生じたいかなる損害に関しても，そ
#  の責任を負わない．
# 
#  $Id$
# 

#
#  ターゲットの指定（Makefile.targetで上書きされるのを防ぐため）
#
all:

#
#  ターゲット略称の定義
#
TARGET = k210_gcc

#
#  プログラミング言語の定義
#
SRCLANG = c
ifeq ($(SRCLANG),c)
	LIBS = -lc -lm
endif
ifeq ($(SRCLANG),c++)
	USE_CXX = true
	CXXLIBS = -lstdc++ -lc -lm
	CXXRTS = cxxrt.o newlibrt.o
endif

#
#  ソースファイルのディレクトリの定義
#
SRCDIR = ../../asp_baseplatform

#
#  オブジェクトファイル名の拡張子の設定
#
OBJEXT = elf

#
#  実行環境の定義（ターゲット依存に上書きされる場合がある）
#
DBGENV := ROM

#
#  カーネルライブラリ（libkernel.a）のディレクトリ名
#  （カーネルライブラリもmake対象にする時は，空に定義する）
#
KERNEL_LIB = 

#
#  カーネルを関数単位でコンパイルするかどうかの定義
#
KERNEL_FUNCOBJS = 

#
#  トレースログを取得するかどうかの定義
#
ENABLE_TRACE = 

#
#  開発ツール（コンパイラ等）のディレクトリの定義
#
DEVTOOLDIR = 

#
#  ユーティリティプログラムの名称
#
PERL = /usr/bin/perl
CFG = $(SRCDIR)/cfg/cfg/cfg

#
#  オブジェクトファイル名の定義
#
OBJNAME = app_iothub_client
ifdef OBJEXT
	OBJFILE = $(OBJNAME).$(OBJEXT)
	CFG1_OUT = cfg1_out.$(OBJEXT)
else
	OBJFILE = $(OBJNAME)
	CFG1_OUT = cfg1_out
endif

#
#  中間オブジェクトファイルと依存関係ファイルを置くディレクトリの定義
#
OBJDIR = obj
DEPDIR = obj

#
#  ターゲット依存部のディレクトリの定義
#
TARGETDIR = $(SRCDIR)/target/$(TARGET)

#
#  ターゲット依存の定義のインクルード
#
include $(TARGETDIR)/Makefile.target

#
#  共通コンパイルオプションの定義
#
COPTS := $(COPTS) -g -std=gnu11 -ffunction-sections -fdata-sections
ifndef OMIT_WARNING_ALL
  COPTS := $(COPTS) -Wall
endif
ifndef OMIT_OPTIMIZATION
  COPTS := $(COPTS) -O2
endif
COPTS := $(COPTS) -Wno-unused-variable -Wno-unused-function
#CDEFS := $(CDEFS) -DLWIP_DEBUG
CDEFS := $(CDEFS) -DMONITOR_PORTID=1 -DUSE_CUPCOMMAND -DNOT_USE_TBP_MKTIME_GMTIME -DSPI_WAIT_TIME=100 -DSDEV_SENSE_ONETIME -DMAX_FONT_IMAGE=24
CDEFS := $(CDEFS) -DUSE_WOLFSSL=1 -DHSM_TYPE_SYMM_KEY -DUSE_PROV_MODULE -Dget_time=azure_get_time
INCLUDES := -I. -I$(SRCDIR)/include -I$(SRCDIR)/arch -I$(SRCDIR) $(INCLUDES)
LDFLAGS := $(LDFLAGS) -Wl,--wrap=exit
CFG1_OUT_LDFLAGS := $(CFG1_OUT_LDFLAGS) 
LIBS := $(LIBS) $(CXXLIBS) \
	$(SRCDIR)/../zlib-1.2.11/Debug/libzlib.a \
	$(SRCDIR)/../wolfssl-4.4.0/Debug/libwolfssl.a \
	$(SRCDIR)/../azure_iot_sdk/Debug/libazure_iot_sdk.a
CFLAGS = $(COPTS) $(CDEFS) $(INCLUDES)

#
#  アプリケーションプログラムに関する定義
#
APPLNAME = app_iothub_client
APPLDIR := $(APPLDIR) ../src ../kendryte
APPL_CFG = main.cfg

APPL_DIR := $(APPLDIR) $(SRCDIR)/library
APPL_ASMOBJS :=
ifdef USE_CXX
	APPL_CXXOBJS := $(APPL_CXXOBJS)
	APPL_COBJS := $(APPL_COBJS) main.o client.o code.o envcmd.o command.o esp_at_socket.o kpu_main.o region_layer.o kpu.o
else
	APPL_COBJS := $(APPL_COBJS) main.o client.o code.o envcmd.o command.o esp_at_socket.o kpu_main.o region_layer.o kpu.o
endif
APPL_COBJS := $(APPL_COBJS) log_output.o vasyslog.o t_perror.o strerror.o stub.o yolo.o
APPL_CFLAGS := $(APPL_CFLAGS)
APPL_LIBS =
ifdef APPLDIR
	INCLUDES := $(INCLUDES) $(foreach dir,$(APPLDIR),-I$(dir))
endif
INCLUDES += -I$(SRCDIR)/../azure_iot_sdk/certs
INCLUDES += -I$(SRCDIR)/../azure_iot_sdk/c-utility/inc
INCLUDES += -I$(SRCDIR)/../azure_iot_sdk/c-utility/pal/toppers
INCLUDES += -I$(SRCDIR)/../azure_iot_sdk/iothub_client/inc
INCLUDES += -I$(SRCDIR)/../azure_iot_sdk/iothub_client/inc/internal
INCLUDES += -I$(SRCDIR)/../azure_iot_sdk/provisioning_client/adapters
INCLUDES += -I$(SRCDIR)/../azure_iot_sdk/provisioning_client/inc
INCLUDES += -I$(SRCDIR)/../azure_iot_sdk/provisioning_client/inc/azure_prov_client/internal
INCLUDES += -I$(SRCDIR)/../azure_iot_sdk/deps/parson
INCLUDES += -I$(SRCDIR)/../azure_iot_sdk/deps/azure-macro-utils-c/inc
INCLUDES += -I$(SRCDIR)/../azure_iot_sdk/deps/umock-c/inc
INCLUDES += -I$(SRCDIR)/../azure_iot_sdk/serializer/inc
INCLUDES += -I$(SRCDIR)/../azure_iot_sdk/uhttp/inc
INCLUDES += -I$(SRCDIR)/../azure_iot_sdk/umqtt/inc
INCLUDES += -I$(SRCDIR)/../wolfssl-4.4.0/wolfssl
INCLUDES += -I$(SRCDIR)/../wolfssl-4.4.0/wolfssl/wolfcrypt
INCLUDES += -I$(SRCDIR)/../wolfssl-4.4.0
INCLUDES += -I$(SRCDIR)/../zlib-1.2.11/src

#
#  ミドルウェアの Makefile のインクルード
#
include $(SRCDIR)/monitor/Makefile.config
include $(SRCDIR)/files/ff/Makefile.config
include $(SRCDIR)/files/Makefile.config
include $(SRCDIR)/ui/snfont_disp/Makefile.config

#
#  GDIC Makefile のインクルード
#
include $(SRCDIR)/gdic/sipeed_st7789/Makefile.config
include $(SRCDIR)/gdic/sipeed_ov2640/Makefile.config
include $(SRCDIR)/gdic/spi_driver/Makefile.config

#
#  システムサービスに関する定義
#
SYSSVC_DIR := $(SYSSVC_DIR) $(SRCDIR)/syssvc $(SRCDIR)/library $(SRCDIR)/pdic/k210
SYSSVC_ASMOBJS := $(SYSSVC_ASMOBJS)
SYSSVC_COBJS := $(SYSSVC_COBJS) banner.o syslog.o serial.o logtask.o \
				 device.o spi.o spi_reg.o dvp.o clock.o malloc.o riscv64.o \
				 $(CXXRTS)
SYSSVC_CFLAGS := $(SYSSVC_CFLAGS)
SYSSVC_LIBS := $(SYSSVC_LIBS)
INCLUDES := $(INCLUDES) -I$(SRCDIR)/pdic/k210

#
#  トレースログ記録のサンプルコードに関する定義
#
ifdef ENABLE_TRACE
	COPTS := $(COPTS) -DTOPPERS_ENABLE_TRACE
	SYSSVC_DIR := $(SYSSVC_DIR) $(SRCDIR)/arch/tracelog
endif

#
#  ターゲットファイル
#
.PHONY: all
#all: $(DEPDIR) check
all: $(DEPDIR) check $(OBJNAME).bin
#all: $(DEPDIR) check $(OBJNAME).srec

##### 以下は編集しないこと #####

#
#  コンフィギュレータに関する定義
#
CFG_TABS := --api-table $(SRCDIR)/kernel/kernel_api.csv \
			--cfg1-def-table $(SRCDIR)/kernel/kernel_def.csv $(CFG_TABS)

CFG_ASMOBJS := $(CFG_ASMOBJS)
CFG_COBJS := kernel_cfg.o $(CFG_COBJS)
CFG_OBJS := $(CFG_ASMOBJS) $(CFG_COBJS)
CFG2_OUT_SRCS := kernel_cfg.h kernel_cfg.c $(CFG2_OUT_SRCS)

#
#  カーネルに関する定義
#
#  KERNEL_ASMOBJS: カーネルライブラリに含める，ソースがアセンブリ言語の
#				   オブジェクトファイル．
#  KERNEL_COBJS: カーネルのライブラリに含める，ソースがC言語で，ソース
#				 ファイルと1対1に対応するオブジェクトファイル．
#  KERNEL_LCSRCS: カーネルのライブラリに含めるC言語のソースファイルで，
#				  1つのソースファイルから複数のオブジェクトファイルを生
#				  成するもの．
#  KERNEL_LCOBJS: 上のソースファイルから生成されるオブジェクトファイル．
#  KERNEL_AUX_COBJS: ロードモジュールに含めないが，カーネルのソースファ
#					 イルと同じオプションを適用してコンパイルすべき，ソー
#					 スがC言語のオブジェクトファイル．
#
KERNEL_DIR := $(KERNEL_DIR) $(SRCDIR)/kernel
KERNEL_ASMOBJS := $(KERNEL_ASMOBJS)
KERNEL_COBJS := $(KERNEL_COBJS)
KERNEL_CFLAGS := $(KERNEL_CFLAGS) -I$(SRCDIR)/kernel
ifdef OMIT_MAKEOFFSET
  OFFSET_H =
else
  OFFSET_H = offset.h
ifndef OFFSET_TF
  KERNEL_AUX_COBJS := $(KERNEL_AUX_COBJS) makeoffset.o
endif
endif

#
#  環境に依存するコンパイルオプションの定義
#
ifdef DBGENV
  CDEFS := $(CDEFS) -D$(DBGENV)
endif

#
#  カーネルのファイル構成の定義
#
include $(SRCDIR)/kernel/Makefile.kernel
ifdef KERNEL_FUNCOBJS
	KERNEL_LCSRCS := $(KERNEL_FCSRCS)
	KERNEL_LCOBJS := $(foreach file,$(KERNEL_FCSRCS),$($(file:.c=)))
else
	KERNEL_CFLAGS := -DALLFUNC $(KERNEL_CFLAGS)
	KERNEL_COBJS := $(KERNEL_COBJS) \
					$(foreach file,$(KERNEL_FCSRCS),$(file:.c=.o))
endif

#
#  ソースファイルのあるディレクトリに関する定義
#
vpath %.c $(KERNEL_DIR) $(SYSSVC_DIR) $(APPL_DIR)
vpath %.cpp $(KERNEL_DIR) $(SYSSVC_DIR) $(APPL_DIR)
vpath %.S $(KERNEL_DIR) $(SYSSVC_DIR) $(APPL_DIR)
vpath %.cfg $(APPL_DIR)

#
#  中間オブジェクトファイルを置くディレクトリの処理
#
APPL_ASMOBJS   := $(addprefix $(OBJDIR)/, $(APPL_ASMOBJS))
APPL_CXXOBJS   := $(addprefix $(OBJDIR)/, $(APPL_CXXOBJS))
APPL_COBJS     := $(addprefix $(OBJDIR)/, $(APPL_COBJS))
SYSSVC_ASMOBJS := $(addprefix $(OBJDIR)/, $(SYSSVC_ASMOBJS))
SYSSVC_COBJS   := $(addprefix $(OBJDIR)/, $(SYSSVC_COBJS))
KERNEL_ASMOBJS := $(addprefix $(OBJDIR)/, $(KERNEL_ASMOBJS))
KERNEL_COBJS   := $(addprefix $(OBJDIR)/, $(KERNEL_COBJS))
KERNEL_LCOBJS  := $(addprefix $(OBJDIR)/, $(KERNEL_LCOBJS))
CFG_ASMOBJS    := $(addprefix $(OBJDIR)/, $(CFG_ASMOBJS))
CFG_COBJS      := $(addprefix $(OBJDIR)/, $(CFG_COBJS))

#
#  コンパイルのための変数の定義
#
APPL_OBJS = $(APPL_ASMOBJS) $(APPL_COBJS) $(APPL_CXXOBJS)
SYSSVC_OBJS = $(SYSSVC_ASMOBJS) $(SYSSVC_COBJS)
KERNEL_LIB_OBJS = $(KERNEL_ASMOBJS) $(KERNEL_COBJS) $(KERNEL_LCOBJS)
CFG_OBJS = $(CFG_ASMOBJS) $(CFG_COBJS)
ALL_OBJS = $(START_OBJS) $(APPL_OBJS) $(SYSSVC_OBJS) $(CFG_OBJS) \
											$(END_OBJS) $(HIDDEN_OBJS)
ALL_LIBS = -lkernel $(LIBS)
ifdef KERNEL_LIB
	LIBS_DEP = $(KERNEL_LIB)/libkernel.a $(filter %.a,$(LIBS))
	OBJ_LDFLAGS := $(OBJ_LDFLAGS) -L$(KERNEL_LIB)
	REALCLEAN_FILES := libkernel.a $(REALCLEAN_FILES)
else
	LIBS_DEP = libkernel.a $(filter %.a,$(LIBS))
	OBJ_LDFLAGS := $(OBJ_LDFLAGS) -L.
endif

ifdef TEXT_START_ADDRESS
	LDFLAGS := $(LDFLAGS) -Wl,-Ttext,$(TEXT_START_ADDRESS)
endif
ifdef DATA_START_ADDRESS
	LDFLAGS := $(LDFLAGS) -Wl,-Tdata,$(DATA_START_ADDRESS)
endif
ifdef LDSCRIPT
	LDFLAGS := $(LDFLAGS) -T $(LDSCRIPT)
endif

#
#  カーネルのコンフィギュレーションファイルの生成
#
cfg1_out.c: cfg1_out.timestamp ;
cfg1_out.timestamp: $(APPL_CFG)
	$(CFG) --pass 1 $(CFG_KERNEL) $(INCLUDES) $(CFG_TABS) $<
	touch -r cfg1_out.c cfg1_out.timestamp

$(CFG1_OUT): $(START_OBJS) $(OBJDIR)/cfg1_out.o $(END_OBJS) $(HIDDEN_OBJS)
	$(LINK) $(CFLAGS) $(LDFLAGS) $(CFG1_OUT_LDFLAGS) -o $(CFG1_OUT) \
						$(START_OBJS) $(OBJDIR)/cfg1_out.o $(LIBS) $(END_OBJS)

cfg1_out.syms: $(CFG1_OUT)
	$(NM) -n $(CFG1_OUT) > cfg1_out.syms

cfg1_out.srec: $(CFG1_OUT)
	$(OBJCOPY) -O srec -S $(CFG1_OUT) cfg1_out.srec

$(CFG2_OUT_SRCS): kernel_cfg.timestamp ;
kernel_cfg.timestamp: $(APPL_CFG) cfg1_out.syms cfg1_out.srec
	$(CFG) --pass 2 $(CFG_KERNEL) $(INCLUDES) \
				-T $(TARGETDIR)/target.tf $(CFG_TABS) $<
	touch -r kernel_cfg.c kernel_cfg.timestamp

#
#  オフセットファイル（offset.h）の生成規則
#
$(OFFSET_H): offset.timestamp ;
ifdef OFFSET_TF
offset.timestamp: $(APPL_CFG) kernel_cfg.timestamp
	$(CFG) --pass 3 --kernel asp $(INCLUDES) \
				--rom-image cfg1_out.srec --symbol-table cfg1_out.syms \
				-T $(OFFSET_TF) $(CFG_TABS) $<
	touch -r $(OFFSET_H) offset.timestamp
else
offset.timestamp: makeoffset.s $(SRCDIR)/utils/genoffset
	$(PERL) $(SRCDIR)/utils/genoffset makeoffset.s > offset.h
	touch -r $(OFFSET_H) offset.timestamp
endif

#
#  カーネルライブラリファイルの生成
#
libkernel.a: $(OFFSET_H) $(KERNEL_LIB_OBJS)
	rm -f libkernel.a
	$(AR) -rcs libkernel.a $(KERNEL_LIB_OBJS)
	$(RANLIB) libkernel.a

#
#  並列makeのための依存関係の定義
#
$(APPL_OBJS) $(SYSSVC_OBJS): | kernel_cfg.timestamp
$(APPL_ASMOBJS) $(SYSSVC_ASMOBJS) $(KERNEL_ASMOBJS) $(CFG_ASMOBJS): \
														| offset.timestamp

#
#  特別な依存関係の定義
#
$(OBJDIR)/banner.o: $(filter-out $(OBJDIR)/banner.o,$(ALL_OBJS)) \
																$(LIBS_DEP)

#
#  全体のリンク
#
$(OBJFILE): $(ALL_OBJS) $(LIBS_DEP)
	$(LINK) $(CFLAGS) $(LDFLAGS) $(OBJ_LDFLAGS) -o $(OBJFILE) \
			-Wl,-Map=$(OBJNAME).map,--cref -Wl,--gc-sections -Wl,-estart,--gc-keep-exported=_kernel_istk \
			$(START_OBJS) $(APPL_OBJS) $(SYSSVC_OBJS) $(CFG_OBJS) \
			-Wl,--start-group $(ALL_LIBS) -Wl,--end-group $(END_OBJS)

#
#  依存関係フォルダの作成
#
$(DEPDIR):
	mkdir $@

#
#  シンボルファイルの生成
#
$(OBJNAME).syms: $(OBJFILE)
	$(NM) -n $(OBJFILE) > $(OBJNAME).syms

#
#  バイナリファイルの生成
#
$(OBJNAME).bin: $(OBJFILE)
	$(OBJCOPY) -O binary -S $(OBJFILE) $(OBJNAME).bin
	$(OBJDUMP) -h $(OBJFILE)
	$(SIZE) $(OBJFILE)

#
#  Sレコードファイルの生成
#
$(OBJNAME).srec: $(OBJFILE)
	$(OBJCOPY) -O srec -S $(OBJFILE) $(OBJNAME).srec

#
#  エラーチェック処理
#
.PHONY: check
check: check.timestamp ;
check.timestamp: $(APPL_CFG) $(OBJNAME).syms $(OBJNAME).srec
#	$(CFG) --pass 3 --kernel asp $(INCLUDES) \
#				--rom-image cfg1_out.srec --symbol-table cfg1_out.syms \
#				-T $(OFFSET_TF) $(CFG_TABS) $<
	touch -r $(OFFSET_H) check.timestamp
	@echo "configuration check passed"

#
#  コンパイル結果の消去
#
.PHONY: clean
clean:
	rm -f \#* *~ $(OBJDIR)/*.o $(DEPDIR)/*.d $(CLEAN_FILES)
	rm -f $(OBJFILE) $(OBJNAME).syms $(OBJNAME).srec $(OBJNAME).bin
	rm -f kernel_cfg.timestamp $(CFG2_OUT_SRCS)
	rm -f offset.timestamp $(OFFSET_H)
	rm -f cfg1_out.syms cfg1_out.srec $(CFG1_OUT)
	rm -f cfg1_out.timestamp cfg1_out.c
ifndef KERNEL_LIB
	rm -f libkernel.a
endif
	rm -f makeoffset.s offset.h

.PHONY: cleankernel
cleankernel:
	rm -f makeoffset.s $(OFFSET_H) $(KERNEL_LIB_OBJS)
	rm -f $(KERNEL_LIB_OBJS:$(OBJDIR)/%.o=$(DEPDIR)/%.d)


.PHONY: realclean
realclean: clean
	rm -f $(REALCLEAN_FILES)

#
#  コンフィギュレータが生成したファイルのコンパイルルールの定義
#
#  コンフィギュレータが生成したファイルは，共通のコンパイルオプション
#  のみを付けてコンパイルする．
#
ALL_CFG_COBJS = $(CFG_COBJS) $(OBJDIR)/cfg1_out.o
ALL_CFG_ASMOBJS = $(CFG_ASMOBJS)

$(ALL_CFG_COBJS): $(OBJDIR)/%.o: %.c
	$(CC) -c -o $@ -MD -MP -MF $(DEPDIR)/$*.d $(CFLAGS) $(CFG_CFLAGS) $<

$(ALL_CFG_COBJS:$(OBJDIR)/%.o=%.s): %.s: %.c
	$(CC) -S -o $@ $(CFLAGS) $(CFG_CFLAGS) $<

$(ALL_CFG_ASMOBJS): $(OBJDIR)/%.o: %.S
	$(CC) -c -o $@ -MD -MP -MF $(DEPDIR)/$*.d $(CFLAGS) $(CFG_CFLAGS) $<

#
#  依存関係ファイルのインクルード
#
-include $(DEPDIR)/*.d

#
#  開発ツールのコマンド名の定義
#
ifeq ($(TOOL),gcc)
	#
	#  GNU開発環境用
	#
	ifdef GCC_TARGET
		GCC_TARGET_PREFIX = $(GCC_TARGET)-
	else
		GCC_TARGET_PREFIX =
	endif
	CC := $(GCC_TARGET_PREFIX)gcc
	CXX := $(GCC_TARGET_PREFIX)g++
	AS := $(GCC_TARGET_PREFIX)as
	LD := $(GCC_TARGET_PREFIX)ld
	AR := $(GCC_TARGET_PREFIX)ar
	NM := $(GCC_TARGET_PREFIX)nm
	RANLIB := $(GCC_TARGET_PREFIX)ranlib
	OBJCOPY := $(GCC_TARGET_PREFIX)objcopy
	OBJDUMP := $(GCC_TARGET_PREFIX)objdump
	SIZE := $(GCC_TARGET_PREFIX)size
endif

ifdef DEVTOOLDIR
	CC := $(DEVTOOLDIR)/$(CC)
	CXX := $(DEVTOOLDIR)/$(CXX)
	AS := $(DEVTOOLDIR)/$(AS)
	LD := $(DEVTOOLDIR)/$(LD)
	AR := $(DEVTOOLDIR)/$(AR)
	NM := $(DEVTOOLDIR)/$(NM)
	RANLIB := $(DEVTOOLDIR)/$(RANLIB)
	OBJCOPY := $(DEVTOOLDIR)/$(OBJCOPY)
	OBJDUMP := $(DEVTOOLDIR)/$(OBJDUMP)
endif

ifdef USE_CXX
	LINK = $(CXX)
else
	LINK = $(CC)
endif

#
#  コンパイルルールの定義
#
$(KERNEL_COBJS): $(OBJDIR)/%.o: %.c
	$(CC) -c -o $@ -MD -MP -MF $(DEPDIR)/$*.d $(CFLAGS) $(KERNEL_CFLAGS) $<

$(KERNEL_COBJS:$(OBJDIR)/%.o=%.s): %.s: %.c
	$(CC) -S -o $@ $(CFLAGS) $(KERNEL_CFLAGS) $<

$(KERNEL_LCOBJS): $(OBJDIR)/%.o:
	$(CC) -c -o $@ -DTOPPERS_$(*F) -MD -MP -MF $(DEPDIR)/$*.d \
									$(CFLAGS) $(KERNEL_CFLAGS) $<

$(KERNEL_LCOBJS:$(OBJDIR)/%.o=%.s): %.s:
	$(CC) -S -o $@ -DTOPPERS_$(*F) $(CFLAGS) $(KERNEL_CFLAGS) $<

$(KERNEL_ASMOBJS): $(OBJDIR)/%.o: %.S
	$(CC) -c -o $@ -MD -MP -MF $(DEPDIR)/$*.d $(CFLAGS) $(KERNEL_CFLAGS) $<

$(SYSSVC_COBJS): $(OBJDIR)/%.o: %.c
	$(CC) -c -o $@ -MD -MP -MF $(DEPDIR)/$*.d $(CFLAGS) $(SYSSVC_CFLAGS) $<

$(SYSSVC_COBJS:$(OBJDIR)/%.o=%.s): %.s: %.c
	$(CC) -S -o $@ $(CFLAGS) $(SYSSVC_CFLAGS) $<

$(SYSSVC_ASMOBJS): $(OBJDIR)/%.o: %.S
	$(CC) -c -o $@ -MD -MP -MF $(DEPDIR)/$*.d $(CFLAGS) $(SYSSVC_CFLAGS) $<

$(APPL_COBJS): $(OBJDIR)/%.o: %.c
	$(CC) -c -o $@ -MD -MP -MF $(DEPDIR)/$*.d $(CFLAGS) $(APPL_CFLAGS) $<

$(APPL_COBJS:$(OBJDIR)/%.o=%.s): %.s: %.c
	$(CC) -S -o $@ $(CFLAGS) $(APPL_CFLAGS) $<

$(APPL_CXXOBJS): $(OBJDIR)/%.o: %.cpp
	$(CXX) -c -o $@ -MD -MP -MF $(DEPDIR)/$*.d $(CFLAGS) $(APPL_CFLAGS) $<

$(APPL_CXXOBJS:$(OBJDIR)/%.o=%.s): %.s: %.cpp
	$(CXX) -S -o $@ $(CFLAGS) $(APPL_CFLAGS) $<

$(APPL_ASMOBJS): $(OBJDIR)/%.o: %.S
	$(CC) -c -o $@ -MD -MP -MF $(DEPDIR)/$*.d $(CFLAGS) $(APPL_CFLAGS) $<

#
#  デフォルトコンパイルルールを上書き
#
$(OBJDIR)/%.o: %.c
	@echo "*** Default compile rules should not be used."
	$(CC) -c -o $@ -MD -MP -MF $(DEPDIR)/$*.d $(CFLAGS) $<

%.s: %.c
	@echo "*** Default compile rules should not be used."
	$(CC) -S -o $@ $(CFLAGS) $<

$(OBJDIR)/%.o: %.cpp
	@echo "*** Default compile rules should not be used."
	$(CXX) -c -o $@ -MD -MP -MF $(DEPDIR)/$*.d $(CFLAGS) $<

%.s: %.cpp
	@echo "*** Default compile rules should not be used."
	$(CXX) -S -o $@ $(CFLAGS) $<

$(OBJDIR)/%.o: %.S
	@echo "*** Default compile rules should not be used."
	$(CC) -c -o $@ -MD -MP -MF $(DEPDIR)/$*.d $(CFLAGS) $<
