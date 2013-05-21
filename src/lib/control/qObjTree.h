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


#ifndef ANAQT_CONTROL_QOBJTREE_H
#define ANAQT_CONTROL_QOBJTREE_H


#include <qtreewidget.h>
#include <map>
#include <string>
#include <set>

class QPixmap;
class QTreeWidget;
class QTreeWidgetItem;

namespace anatomist
{
  class AObject;
  class MObject;
}


/**	Qt object tree widget for the control wondow
 */
class QObjectTree : public QWidget
{
  Q_OBJECT

public:
  QObjectTree( QWidget *parent, const char *name );
  virtual ~QObjectTree();

  ///	adds the object at the base level of this tree
  virtual void RegisterObject( anatomist::AObject* obj );
  virtual void UnregisterObject( anatomist::AObject* obj );
  virtual void NotifyObjectChange( anatomist::AObject* obj );
  /**	adds the object in a sub-tree. \\
	(the corresponding sub-tree has to be searched for first)
	@param	mobj	immediate parent of obj
	@param	obj	object to map
  */
  virtual void RegisterSubObject( anatomist::MObject *mobj, 
				  anatomist::AObject *obj );
  virtual void UnregisterSubObject( anatomist::MObject *mobj, 
				    anatomist::AObject *obj );

  virtual std::set<anatomist::AObject *> *SelectedObjects() const;
  virtual anatomist::AObject* ObjectOfNumber( unsigned pos ) const;
  virtual void SelectObject( anatomist::AObject *obj );
  virtual bool isObjectSelected( anatomist::AObject* obj ) const;
  virtual void UnselectAll();

  ///	Are reference colors markers visible ?
  virtual bool ViewingRefColors() const;
  virtual void ToggleRefColorsView();
  virtual void DisplayRefColors();
  virtual void UndisplayRefColors();


  static void setObjectTypeName(int type, const std::string &name);
  static void setObjectTypeIcon(int type, const std::string &img);

  static unsigned 			RefPixSize;
  static std::map<int, QPixmap>		TypeIcons;
  static std::map<int, std::string>	TypeNames;

signals:
  void selectionChanged();
  void rightButtonPressed( anatomist::AObject*, const QPoint & );

public slots:
  virtual void startDragging( QTreeWidgetItem*, Qt::ButtonState );
  void rightButtonPressed( QTreeWidgetItem*, const QPoint & );
  void objectRenamed( QTreeWidgetItem*, int );
  void sortIndicatorChanged( int, Qt::SortOrder );

protected slots:
  void unselectInvisibleItems();

protected:
  virtual void registerSubObjects( QTreeWidgetItem* li, 
				   anatomist::MObject* mobj );
  virtual void unregisterSubObjects( QTreeWidgetItem* li );
  virtual QTreeWidgetItem* insertObject( QTreeWidgetItem* item, 
				        anatomist::AObject* obj );
  virtual QTreeWidgetItem* insertObject( QTreeWidget* lview, 
				        anatomist::AObject* obj );
  virtual void decorateItem( QTreeWidgetItem* item, anatomist::AObject* obj );
  virtual void dragEnterEvent( QDragEnterEvent* );
  virtual void dragMoveEvent( QDragMoveEvent* );
  virtual void dropEvent( QDropEvent* );

  static void initIcons();

  std::multimap<anatomist::AObject *, QTreeWidgetItem *> _objects;
  std::map<QTreeWidgetItem *, anatomist::AObject *>	_items;
  QTreeWidget				                *_lview;
  bool							_viewRefCol;
  unsigned _count;

private:
};


#endif
