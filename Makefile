export THIS_MAKEFILE := Makefile.FeOS

CODECS := $(notdir $(patsubst %/,%,$(dir $(wildcard codecs/*/Makefile))))

all: $(CODECS)
	@make -f Makefile.FeOS

$(CODECS):
	@make -C codecs/$@

clean:
	@make -f Makefile.FeOS clean
	@for i in $(CODECS); do make -C codecs/$$i clean; done

ifneq ($(strip $(DEST)),)
install: all
	@make -f Makefile.FeOS install
	@for i in $(CODECS); do make -C codecs/$$i install; done
endif
