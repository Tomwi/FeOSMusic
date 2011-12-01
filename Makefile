export THIS_MAKEFILE := Makefile.FeOS

CODECS := $(notdir $(patsubst %/,%,$(dir $(wildcard codecs/*/Makefile))))

all: arm7 $(CODECS)
	@make -f Makefile.FeOS

arm7:
	@make -C arm7SndMod

$(CODECS):
	@make -C codecs/$@

clean:
	@make -f Makefile.FeOS clean
	@make -C arm7SndMod clean
	@for i in $(CODECS); do make -C codecs/$$i clean; done

install: all
	@make -f Makefile.FeOS install
	@make -C arm7SndMod install
	@for i in $(CODECS); do make -C codecs/$$i install; done
