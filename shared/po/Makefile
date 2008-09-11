# options:
# LUPDATEOPTS: -noobsolete (for instance)

BASENAME = anatomist
SRCPATH = `if [ "${OS}" != Windows_NT ]; then cd ../..; pwd; else echo C:/msys/1.0/home/demo/anatomist-stable; fi`
LANGUAGES = en fr

all:	project

project:
	PRO=/tmp/ana-linguist.$$$$; \
	printf 'TRANSLATIONS =' >> $${PRO}; \
	for i in ${LANGUAGES}; do \
	  echo ' \' >> $${PRO}; \
	  printf '  '${SRCPATH}/shared/po/$$i'/${BASENAME}.ts' >> $${PRO}; \
	done; \
	echo >> $${PRO}; \
	echo >> $${PRO}; \
	echo 'SOURCES = \' >> $${PRO}; \
	find $(SRCPATH)/src -name '*.cc' -exec echo "  {} \\" >> $${PRO} \; ; \
	${QTDIR}/bin/lupdate ${LUPDATEOPTS} $${PRO}; \
	rm $${PRO}

