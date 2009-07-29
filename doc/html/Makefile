all:  ./manual_html/book1.html ./manual_pdf/manual_anatomist.pdf


./manual_html/book1.html: manual_anatomist.xml
	if [ -d ./manual_html ]; then rm -r ./manual_html; fi
	[ -d ./manual_html ] || mkdir ./manual_html
	[ -d ./manual_html/imagesAna ] || ln -s ../imagesAna manual_html/imagesAna
	db2html manual_anatomist.xml -o ./manual_html -d $(P4)/docbook-main/dsl/v_fr.dsl#html
	if [ -d bv ]; then rm -r bv; fi
	if [ -d bv.junk ]; then rm -r bv.junk; fi


 
./manual_pdf/manual_anatomist.pdf: manual_anatomist.xml
	if [ -d ./manual_pdf ]; then rm -r ./manual_pdf; fi
	[ -d ./manual_pdf ] || mkdir ./manual_pdf
	db2pdf manual_anatomist.xml -o ./manual_pdf -d $(P4)/docbook-main/dsl/v_fr.dsl#print
	if [ -d manual_anatomist ]; then rm -r manual_anatomist; fi
	if [ -d manual_anatomist.junk ]; then rm -r manual_anatomist.junk; fi
