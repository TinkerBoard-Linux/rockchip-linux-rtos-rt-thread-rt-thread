
CODE_PATH := $(shell pwd)/codec
DECODE_PATH := $(CODE_PATH)/decode
ENCODE_PATH := $(CODE_PATH)/encode

LIBS_INSTALL_DIRS := $(CODE_PATH)/
ifneq ($(LIBS_INSTALL_DIRS), $(wildcard $(LIBS_INSTALL_DIRS)))
LIBS_INSTALL_DIRS := ./
endif

LIBS_CODEC_OBJS ?= 

ifeq ($(CONFIG_AUDIO_DECODER_MP3), y)
MP3_DIRS = $(DECODE_PATH)/mp3
ifeq ($(MP3_DIRS), $(wildcard $(MP3_DIRS)))
MP3_INCLUDE_PATHS := \
		-I"$(DECODE_PATH)/mp3" \
		-I"$(DECODE_PATH)/../../" \
		-I"$(DECODE_PATH)/../../dsp_include"
LIBS_MP3_SRCS := $(foreach dir,$(MP3_DIRS),$(wildcard $(dir)/*.[cS] $(dir)/*.cpp))
LIBS_MP3_NAME := $(LIBS_INSTALL_DIRS)/libmp3.a
LIBS_MP3_CFLAGS := $(CFLAGS) $(MP3_INCLUDE_PATHS) $(INCLUDE_PATHS)
LIBS_MP3_CPPFLAGS := $(CPPFLAGS)
LIBS_MP3_SRCS_OBJS := $(addsuffix .o, $(basename $(LIBS_MP3_SRCS)))
LIBS_CLEAN_OBJS += $(LIBS_MP3_SRCS_OBJS)
LIBS_ALL_NAME += $(LIBS_MP3_NAME)
INCLUDE_PATHS += $(MP3_INCLUDE_PATHS)
LIBS_CODEC_OBJS += $(LIBS_MP3_SRCS_OBJS)
endif
endif


ifeq ($(findstring y,$(CONFIG_AUDIO_DECODER_AMR) $(CONFIG_AUDIO_ENCODER_AMR)), y)

AMR_DIRS := $(ENCODE_PATH)/amr/opencore-amr/amrnb \
	$(ENCODE_PATH)/amr/opencore-amr/opencore/codecs_v2/audio/gsm_amr/amr_nb/common/src \
	$(ENCODE_PATH)/amr/opencore-amr/opencore/codecs_v2/audio/gsm_amr/amr_nb/enc/src \
	$(ENCODE_PATH)/amr/opencore-amr/opencore/codecs_v2/audio/gsm_amr/amr_nb/dec/src \
	$(DECODE_PATH)/amr \
	$(ENCODE_PATH)/amr

ifeq ($(AMR_DIRS), $(wildcard $(AMR_DIRS)))
AMR_INCLUDE_PATHS := \
	-I"$(DECODE_PATH)/../../" \
	-I"$(ENCODE_PATH)/amr" \
	-I"$(DECODE_PATH)/amr" \
	-I"$(ENCODE_PATH)/amr/opencore-amr/opencore/codecs_v2/audio/gsm_amr/amr_nb/enc/include" \
	-I"$(ENCODE_PATH)/amr/opencore-amr/opencore/codecs_v2/audio/gsm_amr/amr_nb/enc/src" \
	-I"$(ENCODE_PATH)/amr/opencore-amr/oscl" \
	-I"$(ENCODE_PATH)/amr/opencore-amr/amrnb" \
	-I"$(ENCODE_PATH)/amr/opencore-amr/opencore/codecs_v2/audio/gsm_amr/amr_nb/dec/src" \
	-I"$(ENCODE_PATH)/amr/opencore-amr/opencore/codecs_v2/audio/gsm_amr/amr_nb/common/include" \
	-I"$(ENCODE_PATH)/amr/opencore-amr/opencore/codecs_v2/audio/gsm_amr/amr_nb/common/src" \
	-I"$(ENCODE_PATH)/amr/opencore-amr/opencore/codecs_v2/audio/gsm_amr/amr_nb/dec/include" \
	-I"$(ENCODE_PATH)/amr/opencore-amr/opencore/codecs_v2/audio/gsm_amr/common/dec/include"

AMR_FILES_IGNORE := common/src/bits2prm.cpp \
	common/src/copy.cpp \
	common/src/div_32.cpp \
	common/src/l_abs.cpp \
	common/src/q_plsf_3.cpp \
	common/src/q_plsf_3_tbl.cpp \
	common/src/qua_gain_tbl.cpp \
	common/src/r_fft.cpp \
	common/src/vad1.cpp \
	common/src/vad2.cpp \
	d_plsf_3.cpp \
	dec_lag3.cpp \
	decoder_gsm_amr.cpp \
	dtx_dec.cpp \
	pvgsmamrdecoder.cpp \
	pvgsmamrdecoder.cpp \
	qgain475_tab.cpp \
	dtx_enc.cpp \
	gsmamr_encoder_wrapper.cpp \
	qgain475.cpp \
	qgain795.cpp \
	qua_gain.cpp

AMR_FILES_IGNORE := $(addprefix %,$(AMR_FILES_IGNORE))
LIBS_AMR_SRCS := $(foreach dir,$(AMR_DIRS),$(wildcard $(dir)/*.[cS] $(dir)/*.cpp))
LIBS_AMR_SRCS := $(filter-out $(AMR_FILES_IGNORE),$(LIBS_AMR_SRCS))
LIBS_AMR_CFLAGS := $(CFLAGS) $(AMR_INCLUDE_PATHS) $(INCLUDE_PATHS)
LIBS_AMR_CPPFLAGS := $(CPPFLAGS) -xc
LIBS_AMR_SRCS_OBJS := $(addsuffix .o, $(basename $(LIBS_AMR_SRCS)))
LIBS_CLEAN_OBJS += $(LIBS_AMR_SRCS_OBJS)
INCLUDE_PATHS += $(AMR_INCLUDE_PATHS)

LIBS_AMR_NAME := $(LIBS_INSTALL_DIRS)/libamr.a
LIBS_AMR_CPPFLAGS += -DAMRNB_TINY
LIBS_ALL_NAME += $(LIBS_AMR_NAME)

LIBS_CODEC_OBJS += $(LIBS_AMR_SRCS_OBJS)

endif
endif # end amr decode and encode

ifeq ($(CONFIG_AUDIO_ENCODER_SPEEX), y)
SPEEX_DIRS := $(ENCODE_PATH)/speex \
	$(ENCODE_PATH)/speex/libspeex
ifeq ($(SPEEX_DIRS), $(wildcard $(SPEEX_DIRS)))
SPEEX_INCLUDE_PATHS := -I"$(ENCODE_PATH)/speex" \
		-I"$(ENCODE_PATH)/speex/include" \
		-I"$(ENCODE_PATH)/speex/include/speex" \
		-I"$(ENCODE_PATH)/speex/libspeex"
LIBS_SPEEX_SRCS := $(foreach dir,$(SPEEX_DIRS),$(wildcard $(dir)/*.[cS] $(dir)/*.cpp))
LIBS_SPEEX_NAME := $(LIBS_INSTALL_DIRS)/libspeex.a
LIBS_SPEEX_CFLAGS := $(CFLAGS) $(SPEEX_INCLUDE_PATHS) $(INCLUDE_PATHS)
LIBS_SPEEX_CPPFLAGS := $(CPPFLAGS)
LIBS_SPEEX_CPPFLAGS += -DFIXED_POINT
LIBS_SPEEX_SRCS_OBJS += $(addsuffix .o, $(basename $(LIBS_SPEEX_SRCS)))
LIBS_CLEAN_OBJS += $(LIBS_SPEEX_SRCS_OBJS)
LIBS_ALL_NAME += $(LIBS_SPEEX_NAME)
INCLUDE_PATHS += $(SPEEX_INCLUDE_PATHS)
LIBS_CODEC_OBJS += $(LIBS_SPEEX_SRCS_OBJS)
endif
endif

$(LIBS_SPEEX_SRCS_OBJS): $(LIBS_SPEEX_SRCS)
ifeq ($(CONFIG_AUDIO_ENCODER_SPEEX), y)
	$(Q)$(CC) $(LIBS_SPEEX_CFLAGS) $(LIBS_SPEEX_CPPFLAGS) -c $(patsubst %.o,%.c,$@) -o $@
endif

$(LIBS_MP3_SRCS_OBJS): $(LIBS_MP3_SRCS)
ifeq ($(CONFIG_AUDIO_DECODER_MP3), y)
	$(Q)$(CC) $(LIBS_MP3_CFLAGS) $(LIBS_MP3_CPPFLAGS) -c $(patsubst %.o,%.c,$@) -o $@
endif

$(LIBS_AMR_SRCS_OBJS): $(LIBS_AMR_SRCS)
ifeq ($(findstring y,$(CONFIG_AUDIO_DECODER_AMR) $(CONFIG_AUDIO_ENCODER_AMR)), y)
	$(Q)$(CC) $(LIBS_AMR_CFLAGS) $(LIBS_AMR_CPPFLAGS) -c $(patsubst %.o,%.cpp,$@) -o $@
endif
