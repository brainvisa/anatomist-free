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


#ifndef ANAQT_BROWSER_QOBJBROWSERWID_H
#define ANAQT_BROWSER_QOBJBROWSERWID_H


#include <cartobase/object/attributed.h>
#include <aims/listview/qalistview.h>

class Graph;


namespace anatomist
{
  class AObject;
  class Hierarchy;
}


/**	Specialized QListView class for AObjects / GenericObjects / 
	attributes visualization / selection / modification
*/
class QObjectBrowserWidget : public aims::gui::QAListView
{
  Q_OBJECT

public:
  typedef void (*ObjectHelper)( QObjectBrowserWidget*, 
				anatomist::AObject* object, 
				Q3ListViewItem* parent );
  ///	map type to descriptor function
  typedef std::map<int, ObjectHelper>	ObjectHelperSet;
  enum ItemType
  {
    UNKNOWN, 
    AOBJECT, 
    GOBJECT, 
    ATTRIBUTE, 
    OTHER
  };
  ///	Structure used for item description
  struct ItemDescr
  {
    ///	Item type
    ItemType		type;
    ///	Attribute name (if any)
    std::string		att;
    ///	Attributed object (if any)
    carto::GenericObject	*ao;
    ///	Anatomist object (if any)
    anatomist::AObject		*obj;
    ///	Direct parent type (OTHER if none)
    ItemType		ptype;
    ///	Direct parent attributed object (if any)
    carto::GenericObject	*pao;
    ///	Direct parent Anatomist object (if any)
    anatomist::AObject		*pobj;
    ///	Top parent type (or OTHER)
    ItemType		ttype;
    ///	Top parent attributed object (if any)
    carto::GenericObject	*tao;
    ///	Top parent Anatomist object (if any)
    anatomist::AObject		*tobj;
    
  };

  QObjectBrowserWidget( QWidget* parent, const char* name );
  virtual ~QObjectBrowserWidget();

  virtual void clear();
  /// Add an AObject to the objects
  virtual void registerObject( anatomist::AObject* object,
                               bool temporaryObject = false,
                               int position = -1 );
  /// Remove an AObject from the objects
  virtual void unregisterObject( anatomist::AObject* object );
  /// Updates an object contents (after a change)
  virtual void updateObject( anatomist::AObject* obj );
  /// Add an GenericObject to the objects
  virtual void registerObject( carto::GenericObject* object );
  /// Remove an GenericObject from the objects
  virtual void unregisterObject( carto::GenericObject* object );
  /// Updates an GenericObject contents (after a change)
  virtual void updateObject( carto::GenericObject* obj );
  /// Removes the given item and its children
  virtual void unregisterItem( Q3ListViewItem* item )
  { removeItem( item ); delete item; }
  const std::map<Q3ListViewItem *, anatomist::AObject *> & aObjects() const 
  { return( _aobjects ); }
  const std::map<Q3ListViewItem *, carto::GenericObject *> 
  & gObjects() const 
  { return( _gobjects ); }
  const std::map<Q3ListViewItem *, ItemType> & types() const
  { return( _itemTypes ); }
  ItemType typeOf( Q3ListViewItem * item ) const;
  virtual void describeAObject( anatomist::AObject* obj, 
				Q3ListViewItem* parent );
  virtual Q3ListViewItem* insertObject( Q3ListViewItem* parent, 
				        anatomist::AObject* obj );
  virtual void registerAttribute( Q3ListViewItem* item );
  virtual void registerAObject( Q3ListViewItem* item, anatomist::AObject* obj );
  virtual void registerGObject( Q3ListViewItem* item, 
				carto::GenericObject* obj );
  ///	Query for list view items: global search for AObject
  Q3ListViewItem* itemFor( const anatomist::AObject* obj );
  ///	Local search for AObject
  Q3ListViewItem* itemFor( Q3ListViewItem* parent, 
			   const anatomist::AObject* obj );
  ///	Global search for GenericObject
  Q3ListViewItem* itemFor( const carto::GenericObject* ao );
  ///	Local search for GenericObject
  Q3ListViewItem* itemFor( Q3ListViewItem* parent, 
			   const carto::GenericObject* ao, 
			   bool regist = true );
  ///	Local search for type/string
  Q3ListViewItem* itemFor( Q3ListViewItem* parent, ItemType type, 
			   const std::string & firstfield, bool regist = true );
  ///	Local search for string
  Q3ListViewItem* itemFor( Q3ListViewItem* parent, 
			   const std::string & firstfield );
  ///	Description of the given item
  void whatIs( Q3ListViewItem* item, ItemDescr & descr ) const;

  static ObjectHelperSet	objectHelpers;

public slots:

signals:

protected:
  virtual Q3ListViewItem* insertObject( anatomist::AObject* obj );
  virtual void removeObject( Q3ListViewItem* parent, anatomist::AObject* obj );
  /**	Only removes the item reference and its children in the internal list, 
	does not destroy the QListViewItem itself */
  virtual void removeItem( Q3ListViewItem* item );
  virtual void decorateItem( Q3ListViewItem* item, anatomist::AObject* obj );
  static void describeGraph( QObjectBrowserWidget* br, 
			     anatomist::AObject* obj, 
			     Q3ListViewItem* parent );
  static void describeHierarchy( QObjectBrowserWidget* br, 
				 anatomist::AObject* obj, 
				 Q3ListViewItem* parent );
  virtual void keyPressEvent( QKeyEvent* ev );

  std::map<Q3ListViewItem *, ItemType>			_itemTypes;
  std::map<Q3ListViewItem *, anatomist::AObject *>	_aobjects;
  std::map<Q3ListViewItem *, carto::GenericObject *>	_gobjects;
  bool							_recursive;

private:
};


#endif
