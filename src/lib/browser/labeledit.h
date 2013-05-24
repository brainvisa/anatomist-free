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


#ifndef ANAQT_BROWSER_LABELEDIT_H
#define ANAQT_BROWSER_LABELEDIT_H

#include <cartobase/object/attributed.h>
#include <anatomist/application/Anatomist.h>
#include <qdialog.h>

class QCancelLineEdit;
class QObjectBrowser;
class QTreeWidgetItem;


///	Editor window for labels (interaction with hierarchies)
class QLabelEdit : public QDialog
{
  Q_OBJECT

public:
  QLabelEdit( const std::string & text, int x, int y, unsigned w, unsigned h, 
	      QObjectBrowser* br, carto::GenericObject* ao, 
	      const std::string & att, QTreeWidgetItem* item, 
	      QWidget* parent = theAnatomist->getQWidgetAncestor(), const char* name = 0, Qt::WFlags f = 0 );
  QLabelEdit( const std::string & text, int x, int y, unsigned w, unsigned h, 
	      QObjectBrowser* br, const std::set<carto::GenericObject*> & ao,
	      const std::string & att, const std::set<QTreeWidgetItem*> & item, 
	      QWidget* parent = theAnatomist->getQWidgetAncestor(), const char* name = 0, Qt::WFlags f = 0 );
  ~QLabelEdit();

  std::string text() const;
  carto::GenericObject* attributed() const;
  std::set<carto::GenericObject*> attributedObjects() const;
  std::string attrib() const { return( _att ); }
  QTreeWidgetItem* item() const;
  std::set<QTreeWidgetItem*> items() const;

  ///	Receive input from a browser (click on a hierarchy)
  void receiveValue( const std::string & val );

public slots:
  void accept();
  void reject();

protected:
  void drawContents( const std::string & text, int x, int y, unsigned w,
                     unsigned h, const char* name );

  QObjectBrowser	*_browser;
  QCancelLineEdit	*_le;
  std::string		_att;

private:
  struct Private;
  Private *d;
};


#endif
