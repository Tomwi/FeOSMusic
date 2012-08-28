FEOSMK = $(FEOSSDK)/mk

MANIFEST    := package.manifest
PACKAGENAME := feosmusic

CODECS  := $(notdir $(patsubst %/,%,$(dir $(wildcard codecs/*/Makefile))))
FILTERS	:= $(notdir $(patsubst %/,%,$(dir $(wildcard filters/*/Makefile))))

.PHONY: all clean install main $(CODECS) $(FILTERS)

all: main $(CODECS) $(FILTERS)

main:
	@make -f main.mk $(CONF_TARGET)

$(CODECS):
	@make -C codecs/$@ $(CONF_TARGET)
	
$(FILTERS):
	@make -C filters/$@ $(CONF_TARGET)

clean: CONF_TARGET := clean
clean: all

install: CONF_TARGET := install
install: all

include $(FEOSMK)/packagetop.mk
