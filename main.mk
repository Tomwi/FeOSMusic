#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------

ifeq ($(strip $(DEVKITPRO)),)
$(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>devkitPro")
endif

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

ifeq ($(strip $(FEOSSDK)),)
$(error "Please set FEOSSDK in your environment. export FEOSSDK=<path to>FeOS/sdk")
endif

FEOSMK = $(FEOSSDK)/mk
export THIS_MAKEFILE := main.mk

#---------------------------------------------------------------------------------
# TARGET is the name of the output
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# DATA is a list of directories containing data files
# INCLUDES is a list of directories containing header files
#---------------------------------------------------------------------------------
TARGET        := $(shell basename $(CURDIR))
BUILD         := build
SOURCES       := source source/fix_fft source/gfx
DATA          := data
INCLUDES      := include include/gfx
CODEC_FILE    := audiocodecs.cfg

GFX      := gfx
PNGFILES := $(foreach dir, $(GFX),$(notdir $(wildcard $(dir)/*.png)))
IMGBINS  := $(PNGFILES:.png=.img.bin)
SSUBG    := shared_sbg
GRIT     := $(DEVKITARM)/bin/grit

CONF_DEFINES =
CONF_USERLIBS = libfar feos3d soundStream
CONF_LIBS = -lfeos3d -llibfar -lsoundStream
CONF_FSDIR = fs
CONF_PREREQUISITES += convert
CONF_EXTRACLEAN := $(GFX)/*.bin $(GFX)/$(SSUBG)/*.bin $(CONF_FSDIR)

include $(FEOSMK)/app.mk

install: all
	@mkdir -p $(FEOSDEST)/data/FeOS/bin
	@mkdir -p $(FEOSDEST)/data/FeOSMusic/cfg
	@cp $(TARGET).fx2 $(FEOSDEST)/data/FeOS/bin/$(TARGET).fx2
	@cp $(CODEC_FILE) $(FEOSDEST)/data/FeOSMusic/cfg/$(CODEC_FILE)

convert: $(IMGBINS)
	@make -C $(GFX)/$(SSUBG)
	@mkdir -p $(CONF_FSDIR)
	@cp $(GFX)/*.bin $(CONF_FSDIR)
	@cp $(GFX)/$(SSUBG)/*.bin $(CONF_FSDIR)

$(IMGBINS) : %.img.bin : $(GFX)/%.png $(GFX)/%.grit
	@$(GRIT) $< -fh! -o$(CURDIR)/$(GFX)/$*
