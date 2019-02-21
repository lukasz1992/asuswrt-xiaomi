# $Id: Makefile.in 18040 2002-12-04 14:11:26Z VZ $

CONTRIB_SAMPLES=editlbox dynsash dynsash_switch multicell splittree led

all:
	@for d in $(CONTRIB_SAMPLES); do (cd $$d && $(MAKE)); done

clean:
	@for d in $(CONTRIB_SAMPLES); do (cd $$d && $(MAKE) clean); done

