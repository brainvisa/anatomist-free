/* Copyright (c) 1995-2005 CEA
 *
 *  This software and supporting documentation were developed by
 *      CEA/DSV/SHFJ
 *      4 place du General Leclerc
 *      91401 Orsay cedex
 *      France
 *
 * This software is governed by the CeCILL license version 2 under 
 * French law and abiding by the rules of distribution of free software.
 * You can  use, modify and/or redistribute the software under the 
 * terms of the CeCILL license version 2 as circulated by CEA, CNRS
 * and INRIA at the following URL "http://www.cecill.info". 
 * 
 * As a counterpart to the access to the source code and  rights to copy,
 * modify and redistribute granted by the license, users are provided only
 * with a limited warranty  and the software's author,  the holder of the
 * economic rights,  and the successive licensors  have only  limited
 * liability. 
 * 
 * In this respect, the user's attention is drawn to the risks associated
 * with loading,  using,  modifying and/or developing or reproducing the
 * software by the user in light of its specific status of free software,
 * that may mean  that it is complicated to manipulate,  and  that  also
 * therefore means  that it is reserved for developers  and  experienced
 * professionals having in-depth computer knowledge. Users are therefore
 * encouraged to load and test the software's suitability as regards their
 * requirements in conditions enabling the security of their systems and/or 
 * data to be ensured and,  more generally, to use and operate it in the 
 * same conditions as regards security. 
 * 
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL license version 2 and that you accept its terms.
 */

#ifndef ANAQT_BROWSER_TYPEEDIT_H
#define ANAQT_BROWSER_TYPEEDIT_H

#include <aims/qtcompat/qlistbox.h>
#include <qcombobox.h>

/*

class QTypeEdit : public QWidget
{
  Q_OBJECT

public:
  ///
  QTypeEdit( const std::string & text, int x, int y, int w, int h,
	       QWidget* parent = 0, const char* name = 0, WFlags f = 0 );
  ///
  ~QTypeEdit();

  ///
  std::string text() const
    { return( _te->text().utf8().data() ); }
  ///
  QTypeEdit	*typeEdit() { return( _te ); }

signals:

public slots:

protected:

  ///
  QCancelTypeEdit	*_te;

private:
};
*/

#include <cartobase/object/object.h>
#include <qdialog.h>
#include <aims/qtcompat/qlistview.h>

//class QCancelLineEdit;
class QObjectBrowser;
class QCancelLineEdit;


///	Editor window for labels (interaction with hierarchies)
class listboxeditor : public QDialog
{
  Q_OBJECT

public:
  listboxeditor( const std::string & text, int x, int y, unsigned w, 
                 unsigned h, QObjectBrowser* br, carto::GenericObject* ao,
                 const std::string & att, Q3ListViewItem* item,
                 QWidget* parent = 0, const char* name = 0, Qt::WFlags f = 0 );
  ~listboxeditor();

//  std::string text() const;
  //carto::GenericObject* attributed() const { return( _ao ); }
  std::string attrib() const { return( _att.utf8().data() ); }
  //Q3ListViewItem* item() const { return( _item ); }
  ///	Receive input from a browser (click on a hierarchy)
  //void receiveValue( const std::string & val );

public slots:
  void change(QComboBox *);
  QString get_result();
  void reject();
  //string send_text();

protected:
  QObjectBrowser		*_browser;
  QCancelLineEdit		*_te;
  carto::GenericObject		*_ao;
  QString			_att;
  Q3ListViewItem		*_item;

private:
};


#endif
