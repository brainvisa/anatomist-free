
BASENAME = anafold
SRCPATH  = ../../src
LANGUAGES = fr

all:	project

project:
	PRO=`mktemp /tmp/ana-linguist.XXXXXX`; \
	/bin/echo -n 'TRANSLATIONS =' >> $${PRO}; \
	for i in ${LANGUAGES}; do \
	  echo ' \' >> $${PRO}; \
	  /bin/echo -n '  '$$i'/${BASENAME}.ts' >> $${PRO}; \
	done; \
	echo >> $${PRO}; \
	echo >> $${PRO}; \
	echo 'SOURCES = \' >> $${PRO}; \
	find $(SRCPATH) -name '*.cc' -exec echo "{} \\" >> $${PRO} \; ; \
	lupdate $${PRO}; \
	lrelease $${PRO}; \
	rm $${PRO}
