export THIS_MAKEFILE := Makefile.FeOS

GRIT        := $(DEVKITARM)/bin/grit.exe

CODECS := $(notdir $(patsubst %/,%,$(dir $(wildcard codecs/*/Makefile))))
GFX		:=  gfx
PNGFILES		:= $(foreach dir, $(GFX),$(notdir $(wildcard $(dir)/*.png)))
IMGBINS			:= $(PNGFILES:.png=.img.bin)

all: arm7 $(CODECS)
	@make --no-print-directory -f Makefile.FeOS

arm7:
	@make --no-print-directory -C arm7SndMod

$(CODECS):
	@make --no-print-directory -C codecs/$@

clean:
	@make --no-print-directory -f Makefile.FeOS clean
	@make --no-print-directory -C arm7SndMod clean
	@for i in $(CODECS); do make --no-print-directory -C codecs/$$i clean; done

install: all
	@make --no-print-directory -f Makefile.FeOS install
	@make --no-print-directory -C arm7SndMod install
	
	@for i in $(CODECS); do make --no-print-directory -C codecs/$$i install; done
		
convert: $(IMGBINS)
$(IMGBINS) : %.img.bin : $(GFX)/%.png $(GFX)/%.grit
		$(GRIT) $<  -o$(CURDIR)/gfx/$*