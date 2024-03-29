/* This software and supporting documentation are distributed by
 *     Institut Federatif de Recherche 49
 *     CEA/NeuroSpin, Batiment 145,
 *     91191 Gif-sur-Yvette cedex
 *     France
 *
 * This software is governed by the CeCILL-B license under
 * French law and abiding by the rules of distribution of free software.
 * You can  use, modify and/or redistribute the software under the
 * terms of the CeCILL-B license as circulated by CEA, CNRS
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
 * knowledge of the CeCILL-B license and that you accept its terms.
 */

#ifndef ANAQT_BROWSER_TYPEEDIT_H
#define ANAQT_BROWSER_TYPEEDIT_H

#include <QListWidget>
#include <qcombobox.h>

/*

class QTypeEdit : public QWidget
{
  Q_OBJECT

public:
  ///
  QTypeEdit( const std::string & text, int x, int y, int w, int h,
	       QWidget* parent = 0, const char* name = 0,
               WindowFlags f = Qt::WindowFlags() );
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
#include <anatomist/application/Anatomist.h>

//class QCancelLineEdit;
class QObjectBrowser;
class QCancelLineEdit;
class QTreeWidgetItem;


///	Editor window for labels (interaction with hierarchies)
class listboxeditor : public QDialog
{
  Q_OBJECT

public:
  listboxeditor( const std::string & text, int x, int y, unsigned w, 
                 unsigned h, QObjectBrowser* br, carto::GenericObject* ao,
                 const std::string & att, QTreeWidgetItem* item,
                 QWidget* parent = theAnatomist->getQWidgetAncestor(),
                 const char* name = 0, Qt::WindowFlags f = Qt::WindowFlags() );
  ~listboxeditor();

//  std::string text() const;
  //carto::GenericObject* attributed() const { return( _ao ); }
  std::string attrib() const { return( _att.toStdString() ); }
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
  QTreeWidgetItem		*_item;

private:
};


#endif
