export THIS_MAKEFILE := Makefile.FeOS

GRIT        := $(DEVKITARM)/bin/grit

CODECS 		:= $(notdir $(patsubst %/,%,$(dir $(wildcard codecs/*/Makefile))))
FILTERS		:= $(notdir $(patsubst %/,%,$(dir $(wildcard filters/*/Makefile))))
GFX			:=  gfx
PNGFILES	:= $(foreach dir, $(GFX),$(notdir $(wildcard $(dir)/*.png)))
IMGBINS		:= $(PNGFILES:.png=.img.bin)
SSUBG		:= shared_sbg

all: $(CODECS) $(FILTERS) convert
	@make --no-print-directory -f Makefile.FeOS

$(CODECS):
	@make --no-print-directory -C codecs/$@
	
$(FILTERS):
	@make --no-print-directory -C filters/$@

clean:
	@make --no-print-directory -f Makefile.FeOS clean
	@for i in $(CODECS); do make --no-print-directory -C codecs/$$i clean; done
	@rm -rf fs
	@rm -f $(GFX)/*.bin

install: all
	@make --no-print-directory -f Makefile.FeOS install
	@for i in $(CODECS); do make --no-print-directory -C codecs/$$i install; done
	@for i in $(FILTERS); do make --no-print-directory -C filters/$$i install; done
	
convert: $(IMGBINS)
	@make -C gfx/$(SSUBG)
	@[ -d fs ] || mkdir -p fs || exit 1
	@cp $(GFX)/*.bin fs
	@cp $(GFX)/$(SSUBG)/*.bin fs

$(IMGBINS) : %.img.bin : $(GFX)/%.png $(GFX)/%.grit
	@$(GRIT) $< -fh! -o$(CURDIR)/gfx/$*

