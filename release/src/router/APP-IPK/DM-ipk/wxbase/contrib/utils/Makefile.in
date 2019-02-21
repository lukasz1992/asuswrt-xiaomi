# $Id: Makefile.in 34130 2005-05-18 13:52:48Z JS $

CONTRIB_UTILS=convertrc

all:
	@for d in $(CONTRIB_UTILS); do (cd $$d && $(MAKE)); done

clean:
	@for d in $(CONTRIB_UTILS); do (cd $$d && $(MAKE) clean); done

