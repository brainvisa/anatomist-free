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


#ifndef ANA_SELECTION_SELECTFACTORY_H
#define ANA_SELECTION_SELECTFACTORY_H


#include <aims/vector/vector.h>
#include <list>
#include <map>
#include <set>


class Tree;


namespace anatomist
{
  class AObject;
  class AWindow;
  class WSelectChooser;
  class Referential;
  class Selector;
  class PostSelector;


  /**	Central selection center. \\
	usage : a default factory is given as a static member
	(see factory() and setFactory()). Any instance creation of an 
	overloaded subclass of factory replaces this default factory by 
	itself.
  */
  class SelectFactory
  {
  public:
    enum SelectMode
    {
      Normal = 0, 
      Add = 1, 
      Toggle = 2
    };

    struct HColor
    {
      HColor() {}
      HColor( float red, float grn, float blu, float al = 1., 
	      bool ntalp = false )
	: r( red), g( grn ), b( blu ), a( al ), na( ntalp ) {}
      bool operator == ( const HColor & ) const;

      float	r;
      float	g;
      float	b;
      float	a;
      ///	neutral alpha
      bool	na;
    };

    SelectFactory();
    virtual ~SelectFactory();

    /**	Creates a selection choice window.
	The default factory returns a base object which is not really a window
	(see SelectChooser). This mechanism enables to switch between Motif 
	windows and Qt windows (see QSelectFactory) implementations
    */
    virtual WSelectChooser* 
    createSelectChooser( unsigned group, 
			 const std::set<AObject *> & objects ) const;
    const std::map<unsigned, std::set<AObject *> > & selected() const
    { return( _selected() ); }
    virtual void select( unsigned group, const std::set<AObject *> & obj, 
			 const HColor* col = 0 ) const;
    virtual void unselect( unsigned group, 
			   const std::set<AObject *> & obj ) const;
    virtual void unselectAll( unsigned group ) const;
    ///	Selects all selectable objects in a given window
    virtual void selectAll( AWindow* win, const HColor* col = 0 ) const;
    ///	Inverts state of given objects
    virtual void flip( unsigned group, const std::set<AObject *> & obj, 
		       const HColor* col = 0 ) const;
    virtual void select( SelectMode mode, unsigned group, 
			 const std::set<AObject *> & obj, 
			 const HColor* col = 0 ) const;
    virtual bool isSelected( unsigned group, AObject* obj ) const;
    virtual HColor highlightColor( AObject* obj ) const;
    virtual void setHighlightColor( AObject* obj, 
				    const HColor* col = 0 ) const;
    ///	Redraws objects in involved windows
    virtual void refresh() const;
    /**	draws / handles the selection menu (right click)
	@param	specific	if given, describes the menu tree for 
	window-specific actions. The callbacks must 
	be in the tree, like in AObject's option trees.
	Callbacks type is void (*func)( void * ), 
	with an optional "client_data" attribute passed
	to the function
    */
    virtual void handleSelectionMenu( AWindow *win, int x, int y, 
				      const Tree* specific = 0 );
    virtual void propagateSelection( unsigned group ) const;

    static void setFactory( SelectFactory* fac );
    static SelectFactory* factory();
    ///	tells if win sees obj or one of its ancestors
    static bool hasAncestor( const AWindow* win, AObject* obj );
    static AObject* objectAt( AObject* o, const Point3df & pos, float t, 
			      float tolerence, const Referential* wref, 
			      const Point3df & wgeom, 
			      const std::string & selector = "default" );
    static void select( AWindow* w, const Point3df & pos, float t, 
			float tolerence, int modifier, 
			const std::string & selector = "default" );
    static void findObjectsAt( AWindow* w, const Point3df & pos, float t, 
			       float tolerence, std::set<AObject *>& shown, 
			       std::set<AObject *>& hidden, 
			       const std::string & selector );
    static void registerSelector( const std::string & key, Selector* s );
    ///	Registers possible actions to be taken after object selection
    static void registerPostSelector( const std::string & key, 
				      PostSelector* s );
    /**	Activates a registered post-selector, it will be used after every 
	object selection with a given priority */
    static void activatePostSelector( int priority, const std::string & psel );
    ///	Dectivates a post-selector: won't be called anymore
    static void deactivatePostSelector( const std::string & psel );
    static void setSelectColor( const HColor & col );

    static HColor	& selectColor();
    static bool		& selectColorInverse();

    virtual void remove( anatomist::AWindow* win );
    virtual void removeFromThisWindow( anatomist::AWindow* win );

  protected:

  private:
    ///	Map object -> highlight color
    static std::map<AObject*, HColor> & _highlightColors();
    ///	Windows to refresh
    static std::set<AWindow *> & _winToRefresh();
    static std::map<unsigned, std::set<AObject *> > & _selected();
  };



  inline void SelectFactory::handleSelectionMenu( AWindow *, int, int, 
						  const Tree* )
  {
  }

}

#endif
