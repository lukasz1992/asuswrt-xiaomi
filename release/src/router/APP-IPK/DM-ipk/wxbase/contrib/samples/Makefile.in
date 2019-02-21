# $Id: Makefile.in 29320 2004-09-24 15:36:11Z VS $

CONTRIB_SAMPLES=fl gizmos mmedia ogl plot stc svg deprecated #applet net

all:
	@for d in $(CONTRIB_SAMPLES); do (cd $$d && $(MAKE)); done

clean:
	@for d in $(CONTRIB_SAMPLES); do (cd $$d && $(MAKE) clean); done

