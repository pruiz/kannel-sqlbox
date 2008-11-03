man1pages = gw/sqlbox.1
man5pages = gw/sqlbox.5
docsrcs = $(wildcard grep -l '<!DOCTYPE ' doc/*.xml)
docs = $(docsrcs:.xml=.html) $(docsrcs:.xml=.rtf) $(docsrcs:.xml=.ps) $(docsrcs:.xml=.pdf)
DOCSTARGET=@DOCSTARGET@
DOCDRAFTS=@DOCDRAFTS@

pssrcs = $(wildcard doc/*.png)
ps = $(pssrcs:.png=.ps)  

figsrcs = $(wildcard doc/*.fig)
figs = $(figsrcs:.fig=.png)  $(figsrcs:.fig=.ps)

.xml.html:
	sed "s/#FIGTYPE#/.png/;s/#VERSION#/${VERSION}/;s/#DATE#/`date +%Y.%m.%d`/;s/#DRAFTS#/${DOCDRAFTS}/" $< > $*.tmp
	${JADE} -V nochunks -t sgml -d $(HTML_DSL) $*.tmp > $@
	rm -f $*.tmp

.xml.rtf:
	sed "s/#FIGTYPE#/.ps/;s/#VERSION#/${VERSION}/;s/#DATE#/`date +%Y.%m.%d`/;s/#DRAFTS#/${DOCDRAFTS}/" $< > $*.tmp
	cd `dirname $<` && $(JADE) -o `basename $*`.rtf -t rtf -d $(TEX_DSL) `basename $*`.tmp
	rm -f $*.tmp

.xml.ps:
	sed "s/#FIGTYPE#/.ps/;s/#VERSION#/${VERSION}/;s/#DATE#/`date +%Y.%m.%d`/;s/#DRAFTS#/${DOCDRAFTS}/" $< > $*.tmp
	$(JADE) -o $*.tex -t tex -d $(TEX_DSL) $*.tmp
	rm -f $*.tmp
	cd `dirname $<` && $(JADETEX) `basename $*`.tex >/dev/null || true
	cd `dirname $<` && $(JADETEX) `basename $*`.tex >/dev/null || true
	cd `dirname $<` && $(JADETEX) `basename $*`.tex >/dev/null || \
	    ( echo Check `dirname $<`/`basename $*`.log for errors && false)
	rm -f $*.log
	cd `dirname $<` && $(DVIPS) -q -o `basename $*`.ps `basename $*`.dvi
	rm -f $*.dvi $*.tex $*.aux 

.xml.pdf:
	sed "s/#FIGTYPE#/.png/;s/#VERSION#/${VERSION}/;s/#DATE#/`date +%Y.%m.%d`/;s/#DRAFTS#/${DOCDRAFTS}/" $< > $*.tmp
	$(JADE) -o $*.tex -t tex -d $(TEX_DSL) $*.tmp
	rm -f $*.tmp
	cd `dirname $<` && $(JADETEX) `basename $*`.tex >/dev/null || true
	cd `dirname $<` && $(JADETEX) `basename $*`.tex >/dev/null || true
	cd `dirname $<` && $(JADETEX) `basename $*`.tex >/dev/null || true
	rm -f $*.log $*.dvi
	cd `dirname $<` && $(PDFJADETEX) `basename $*`.tex > /dev/null || true 
	test -r $*.pdf || false
	rm -f $*.log $*.tex $*.aux $*.out

.fig.png:
	$(FIG2DEV) -Lpng $< $@

.fig.ps:
	$(FIG2DEV) -Lps $< $@

.png.ps:
	$(CONVERT) $< $@

docs: figs ps $(docs)
no-docs:
figs: $(figs)
ps: $(ps)
pp: $(pres)

SUBDIRS = gw

EXTRA_DIST = KannelLICENSE bootstrap
