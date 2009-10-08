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


#include <anatomist/selection/selectFactory.h>
#include <anatomist/controler/view.h>
#include <anatomist/window/controlledWindow.h>
#include <anatomist/object/Object.h>
#include <anatomist/mobject/MObject.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/window/Window.h>
#include <anatomist/selection/selector.h>
#include <anatomist/selection/postSelector.h>
#include <anatomist/selection/wSelChooser.h>
#include <anatomist/commands/cSelect.h>
#include <anatomist/processor/Processor.h>
#include <anatomist/reference/Geometry.h>
#include <anatomist/commands/cRemoveObject.h>


using namespace anatomist;
using namespace std;

namespace
{

  map<string,Selector*>	& SelectFactory__selectors()
  {
    static map<string,Selector*> ss;
    return ss;
  }


  map<string, PostSelector *> & SelectFactory__postselectors()
  {
    static map<string, PostSelector *>	ps;
    return ps;
  }


  multimap<int, string> & SelectFactory__active_postsel()
  {
    static multimap<int, string>	aps;
    return aps;
  }


  SelectFactory*& _theFactory()
  {
    static SelectFactory	*facto = 0;
    return facto;
  }


  void SelectFactory__selectors__init()
  {
    if( SelectFactory__selectors().empty() )
      {
        SelectFactory__selectors()[ "default" ] = new Selector;
        SelectFactory__selectors()[ "lowestlevel" ] = new LowestLevelSelector;
      }
  }

}


bool SelectFactory::HColor::operator == ( const HColor & c ) const
{
  return c.r == r && c.g == g && c.b == b && c.na == c.a && ( na || c.a == a );
}


SelectFactory* SelectFactory::factory()
{
  if( !_theFactory() )
    _theFactory() = new SelectFactory;
  return _theFactory();
}


SelectFactory::HColor & SelectFactory::selectColor()
{
  static HColor scol( 0.90, 0.20, 0.15, 1., true );
  return scol;
}


bool & SelectFactory::selectColorInverse()
{
  static bool scolinv = false;
  return scolinv;
}


void SelectFactory::setFactory( SelectFactory* fac )
{
  _theFactory() = fac;
}


map<unsigned, set<AObject *> > & SelectFactory::_selected()
{
  static map<unsigned, set<AObject *> > sel;
  return sel;
}


map<AObject*, SelectFactory::HColor> & SelectFactory::_highlightColors()
{
  static map<AObject*, HColor>	hc;
  return hc;
}

set<AWindow *> & SelectFactory::_winToRefresh()
{
  static set<AWindow *>	wr;
  return wr;
}


SelectFactory::SelectFactory()
{
  setFactory( this );
}


SelectFactory::~SelectFactory()
{
  setFactory( 0 );
  map<string,Selector*>	& sels = SelectFactory__selectors();
  map<string,Selector*>::iterator i, e = sels.end();
  for( i=sels.begin(); i!=e; ++i )
    delete i->second;
  sels.clear();
  map<string,PostSelector*>	& psels = SelectFactory__postselectors();
  map<string,PostSelector*>::iterator ip, ep = psels.end();
  for( ip=psels.begin(); ip!=ep; ++ip )
    delete ip->second;
  psels.clear();
  SelectFactory__active_postsel().clear();
  _highlightColors().clear();
  _winToRefresh().clear();
  _selected().clear();
}


WSelectChooser* 
SelectFactory::createSelectChooser( unsigned, const set<AObject *> & ) const
{
  return( 0 );
}


void SelectFactory::select( unsigned group, const set<AObject *> & obj, 
                            const HColor* col ) const
{
  set<AObject *>::const_iterator	io, fo = obj.end();
  set<AObject *>& so = _selected()[ group ];
  set<AObject *>::iterator fs = so.end();
  set<AWindow *>::const_iterator	iw, fw;
  HColor hcol, acol;
  bool sel;
  set<AWindow *> gw = theAnatomist->getWindowsInGroup( group );


  for( io = obj.begin(); io != fo; ++io )
    {
      for( iw = gw.begin(), fw = gw.end(); iw != gw.end(); ++iw )
	if( !(*iw)->hasObject( *io ) )
	  {
	    if( hasAncestor( *iw, *io ) )
	      {
		(*iw)->registerObject( *io );
		_winToRefresh().insert( *iw );
	      }
	  }

      sel = true;
      if( so.find( *io ) != fs )	// already selected
	{
	  hcol = highlightColor( *io );
	  setHighlightColor( *io, col );
	  acol = highlightColor( *io );
	  if ( acol.r == hcol.r &&
	       acol.g == hcol.g &&
	       acol.b == hcol.b &&
	       acol.a == hcol.a )
	    sel = false;
	}
      if( sel )
	{
	  so.insert( *io );
	  setHighlightColor( *io, col );

	  const set<AWindow *>& win = (*io)->WinList();

	  for ( iw=win.begin(), fw=win.end(); iw!=fw; ++iw )
	    if ( (*iw)->Group() == (int) group )
	      _winToRefresh().insert( *iw );
	}
    }
}


void SelectFactory::unselect( unsigned group, 
			       const set<AObject *> & obj ) const
{
  set<AObject *>::const_iterator	io, fo = obj.end();
  set<AObject *>			& so = _selected()[ group ];
  set<AObject *>::iterator		is, fs = so.end();
  set<AWindow *>::const_iterator	iw, fw;
  bool					unhigh;

  for( io=obj.begin(); io!=fo; ++io )
    if( (is = so.find( *io )) != fs )	// already selected
      {
	so.erase( is );
	_highlightColors().erase( *io );

	unhigh = true;

	if( theAnatomist->hasObject( *io ) )
	  {
	    const set<AWindow *>	& win = (*io)->WinList();

	    for( iw=win.begin(), fw=win.end(); iw!=fw; ++iw )
	      if( (*iw)->Group() == (int) group )
		_winToRefresh().insert( *iw );
	      else
		unhigh = false;
	  }
	if( unhigh )
	  _highlightColors().erase( *io );
      }
}


void SelectFactory::selectAll( AWindow* win, const HColor* col ) const
{
  set<AObject *>			obj = win->Objects();
  set<AObject *>::const_iterator	io, fo=obj.end();
  set<AObject *>			tosel;
  set<AObject *>::iterator		is, fs=tosel.end();
  AObject::ParentList::const_iterator	ip, fp;

  for( io=obj.begin(); io!=fo; ++io )
    {
      // avoid to select an object and its parent
      const AObject::ParentList & par = (*io)->Parents();
      for( ip=par.begin(), fp=par.end(); ip!=fp; ++ip )
	{
	  is = tosel.find( *ip );
	  if( is != fs )
	    tosel.erase( is );	// remove parent from selection list
	}
      tosel.insert( *io );
    }

  select( win->Group(), tosel, col );
}


void SelectFactory::unselectAll( unsigned group ) const
{
  set<AObject *>			& tosel = _selected()[ group ];
  set<AObject *>::const_iterator	io, fo=tosel.end();
  set<AWindow *>::const_iterator	iw, fw;
  bool					unhigh;

  for( io=tosel.begin(); io!=fo; ++io )
    {
      unhigh = true;
      if( theAnatomist->hasObject( *io ) )
	{
	  const set<AWindow *>	& win = (*io)->WinList();

	  for( iw=win.begin(), fw=win.end(); iw!=fw; ++iw )
	    if( (*iw)->Group() == (int) group )
	      _winToRefresh().insert( *iw );
	    else
	      unhigh = false;
	}
      if( unhigh )
	_highlightColors().erase( *io );
    }

  _selected().erase( group );
}


void SelectFactory::flip( unsigned group, const set<AObject *> & obj,
                          const HColor* col ) const
{
  set<AObject *>::const_iterator  io, fo = obj.end();
  set<AObject *>                  & so = _selected()[ group ];
  set<AObject *>                  tosel;
  set<AObject *>                  tounsel;
  set<AObject *>::iterator        notsel = so.end();

  for( io=obj.begin(); io!=fo; ++io )
  {
    if( so.find( *io ) == notsel )
      tosel.insert( *io );
    else
      tounsel.insert( *io );
  }
  if( !tounsel.empty() )
    unselect( group, tounsel );
  if( !tosel.empty() )
    select( group, tosel, col );
}


bool SelectFactory::isSelected( unsigned group, AObject* obj ) const
{
  set<AObject *>	& so = _selected()[ group ];

  return( so.find( obj ) != so.end() );
}


void SelectFactory::setSelectColor( const HColor & col )
{
  SelectFactory	*sel = factory();
  if( sel->selectColor() == col )
    return;

  sel->selectColor() = col;
  const map<unsigned, set<AObject *> > 			& so = sel->selected();
  map<unsigned, set<AObject *> >::const_iterator	io, eo = so.end();
  set<AObject *>::iterator				iio, eio;
  set<AWindow *>::const_iterator			iw, fw;

  sel->_winToRefresh().clear();

  for( io=so.begin(); io!=eo; ++io )
    for( iio=io->second.begin(), eio=io->second.end(); iio!=eio; ++iio )
      if( theAnatomist->hasObject( *iio ) )
        {
	  sel->setHighlightColor( *iio, &col );

	  const set<AWindow *>	& win = (*iio)->WinList();

	  for( iw=win.begin(), fw=win.end(); iw!=fw; ++iw )
            sel->_winToRefresh().insert( *iw );
	}

  for( iw=sel->_winToRefresh().begin(), fw=sel->_winToRefresh().end(); 
       iw!=fw; ++iw )
    (*iw)->Refresh();

  sel->_winToRefresh().clear();
}


void SelectFactory::refresh() const
{
  map<unsigned, set<AObject *> >::iterator	is, fs=_selected().end();
  unsigned				group;
  set<AObject *>::iterator		io, fo, io2;
  bool					reset = false;
  set<AWindow *>::iterator		iw, fw=_winToRefresh().end();

  for( is=_selected().begin(); is!=fs; ++is )
    {
      group = (*is).first;
      set<AObject *> & so = (*is).second;

      reset = false;
      for( io=so.begin(), fo=so.end(); io!=fo; ++io )
	{
	  if( reset )
	    {
	      io = so.begin();
	      reset = false;
	    }
	  if( !theAnatomist->hasObject( *io ) )
	    {
	      // object no longer exists: delete it from list
	      _highlightColors().erase( *io );
	      if( io == so.begin() )
		{
		  so.erase( io );
		  io = so.begin();
		  reset = true;
		}
	      else
		{
		  io2 = io;
		  --io;
		  so.erase( io2 );
		}
	    }
	}
    }

  //	now refresh windows

  ControlledWindow * win = 0;
  for( iw=_winToRefresh().begin(); iw!=fw; ++iw )
    if( theAnatomist->hasWindow( *iw ) )
    {
      (*iw)->Refresh();
      win = dynamic_cast< ControlledWindow * >( *iw );
      if( win )
      {
        win->view()->controlSwitch()->setActivableControls();
        win->view()->controlSwitch()->selectionChangedEvent();
      }
    }

  _winToRefresh().clear();

  // and controls

/*  set<AWindow*>	windows = theAnatomist->getWindows();
  set<AWindow*>::const_iterator gwIter, gwLast( windows.end() );

  for( gwIter=windows.begin(); gwIter!=gwLast; ++gwIter )
  {
    win = dynamic_cast< ControlledWindow * >( *gwIter );

    if ( win != 0 )
    {
      win->view()->controlSwitch()->setActivableControls();
    }
  }*/
}


SelectFactory::HColor SelectFactory::highlightColor( AObject* obj ) const
{
  map<AObject*, HColor>::const_iterator	ic = _highlightColors().find( obj );

  if( ic != _highlightColors().end() )
    return( (*ic).second );
  else
    return( HColor( 1., 1., 0., 1., true ) );
}


void SelectFactory::setHighlightColor( AObject* obj, const HColor* col ) const
{
  if( obj->Is3DObject() )
    {
      Material & mat = obj->GetMaterial();
      if( &mat )	// ensure there is actually one
	{
	  HColor	& hc = _highlightColors()[obj];

	  if( col )
	    hc = *col;
	  else
	    {
	      GLfloat	*dif = mat.Diffuse();

	      if( selectColorInverse() )
	        {
	          hc.r = 1. - dif[0];
	          hc.g = 1. - dif[1];
	          hc.b = 1. - dif[2];
		  hc.a = dif[3];
		  hc.na = true;
		}
	      else
	        {
                  const HColor & sc = selectColor();
	          hc.r = sc.r;
	          hc.g = sc.g;
		  hc.b = sc.b;
		  hc.a = sc.a;
		  hc.na = sc.na;
		}
	    }
	}
      else cout << "no material\n";
    }
}


void SelectFactory::propagateSelection( unsigned group ) const
{
  //	add selected objects in windows which did not contain them before

  //cout << "SelectFactory::propagateSelection\n";
  set<AWindow *>		win = theAnatomist->getWindowsInGroup( group );
  set<AWindow *>::iterator		iw, fw=win.end();
  set<AObject *>			& sel = _selected()[ group ];
  set<AObject *>::const_iterator	io, fo=sel.end();
  bool					refr;

  for( iw=win.begin(); iw!=fw; ++iw )
    {
      //cout << "check win " << (*iw)->Title() << endl;
      refr = false;
      for( io=sel.begin(); io!=fo; ++io )
	if( !(*iw)->hasObject( *io ) )
	  {
	    (*iw)->registerObject( *io );
	    refr = true;
	    //cout << "register obj " << (*io)->name() << endl;
	  }
	/*else cout << "obj " << (*io)->name() << " already in win " 
		  << (*iw)->Title() << endl;*/
    }
  theAnatomist->Refresh();
}


bool SelectFactory::hasAncestor( const AWindow* win, AObject* obj )
{
  if( win->hasObject( obj ) )
    return( true );

  AObject::ParentList & par = obj->Parents();
  AObject::ParentList::const_iterator	ip, fp;

  for( ip=par.begin(), fp=par.end(); ip!=fp; ++ip )
    if( hasAncestor( win, *ip ) )
      return( true );
  return( false );
}


void SelectFactory::select( SelectMode mode, unsigned group, 
			    const set<AObject *> & obj, 
			    const HColor* col ) const
{
  switch( mode )
    {
    case Add:
      select( group, obj, col );
      break;
    case Toggle:
      flip( group, obj, col );
      break;
    default:
      unselectAll( group );
      select( group, obj, col );
    }
}


AObject* SelectFactory::objectAt( AObject* o, const Point3df & pos, float t, 
				  float tolerence, const Referential* wref, 
				  const Point3df & wgeom, const string & key )
{
  SelectFactory__selectors__init();

  map<string, Selector *>::const_iterator 
    is = SelectFactory__selectors().find( key );
  if( is == SelectFactory__selectors().end() )
    {
      cerr << "No Selectors in factory -- no selection possible\n";
      return( 0 );
    }
  return( (*is).second->objectAt( o, pos, t, tolerence, wref, wgeom ) );
}


void SelectFactory::select( AWindow* w, const Point3df & pos, float t, 
			    float tolerence, int modifier, 
			    const string & selector )
{
  enum mode{ SELECT, SHOW };

  set<AObject *>		selected, shown;
  mode				selmode;
  set<AObject*>::iterator	s;

  //cout << "select " << x << ", " << y << ", " << z << ", " << t << endl;

  findObjectsAt( w, pos, t, tolerence, selected, shown, selector );

  if ( selected.size() > 0 )	// selection case
  {
    shown.erase( shown.begin(), shown.end() );	// forget those to show
    selmode = SELECT;
  }
  else
    selmode = SHOW;

  if ( selected.size() > 1 || shown.size() > 1 )
  {
    // cout << "multiple selection\n";
    // get selection windows factory
    SelectFactory* fac = SelectFactory::factory();

    // create a shell for a new window
    WSelectChooser* wch;
    if ( selmode == SELECT )
      wch = fac->createSelectChooser( w->Group(), selected );
    else
      wch = fac->createSelectChooser( w->Group(), shown );

    if ( wch )
    {
      //cout << "open multiselect window\n";
      int res = wch->exec();
      if ( res )
      {
        set<AObject *>::const_iterator i, f;

        if ( selmode == SELECT )
        {
          selected = wch->selectedItems();
          i = selected.begin();
          f = selected.end();
          // cout << "select :\n";
        }
        else
        {
          shown = wch->selectedItems();
          i = shown.begin();
          f = shown.end();
          // cout << "show :\n";
        }
      }
      else // cancel
      {
        SelectFactory::factory()->refresh();
        return;
      }
      delete wch;
    }
    else
    {
      cout << "No selection window avalable, sorry..." 
           << "taking first object\n";
      if ( selmode == SELECT )
      {
        s = selected.begin();
        ++s;
        selected.erase( s, selected.end() );
      }
      else
      {
        s = shown.begin();
        ++s;
        shown.erase( s, shown.end() );
      }
    }
  }

  set<AObject *>* choice;
  switch ( selmode )
  {
    case SHOW:
      choice = &shown;
      break;
    default:
      choice = &selected;
      break;
  }

  set<AObject *>::iterator		es = choice->end();
  multimap<int,string>::iterator	ips, 
    eps = SelectFactory__active_postsel().end();
  PostSelector				*ps;
  set<AObject *>			addedsel;
  set<AObject *>::const_iterator	ino, eno;

  for( ips=SelectFactory__active_postsel().begin(); ips!=eps; ++ips )
    {
      ps = SelectFactory__postselectors()[ ips->second ];
      for( s=choice->begin(); s!=es; ++s )
	{
	  set<AObject *>	newobj = ps->execute( *s, pos, t );
	  for( ino=newobj.begin(), eno=newobj.end(); ino!=eno; ++ino )
	    addedsel.insert( *ino );
	}
    }

  for( ino=addedsel.begin(), eno=addedsel.end(); ino!=eno; ++ino )
    choice->insert( *ino );

  SelectCommand	*c = new SelectCommand( *choice, w->Group(), 
						modifier );
  theProcessor->execute( c );
}


void SelectFactory::findObjectsAt( AWindow* w, const Point3df & pos, float t, 
				   float tolerence, set<AObject *>& shown, 
				   set<AObject *>& hidden, 
				   const string & selector )
{
  set<AObject *>		obj = w->Objects();
  AObject* object = 0;
  set<AObject*>::iterator	i, f=obj.end();

  AObject::ParentList pl;
  bool mustcheck;
  AObject::ParentList::iterator ip, fp;
  Referential			*wref = w->getReferential();
  Geometry			*wgeom = w->windowGeometry();
  Point3df			sgeom;

  if( wgeom )
    sgeom = wgeom->Size();
  else
    sgeom = Point3df( 1, 1, 1 );

  //	check top-level objects in window
  for ( i = obj.begin(); i!=f; ++i )
  {
    pl = (*i)->Parents();
    mustcheck = true;
    for ( ip = pl.begin(), fp = pl.end(); ip != fp; ++ip )
      if ( w->hasObject( (AObject *) *ip ) )
      {
        mustcheck = false;
        break;
      }

    if ( mustcheck )	// top-level for this window only
    {
      object = objectAt( *i, pos, t, tolerence, wref, sgeom, selector );
      if ( object )
      {
        if( obj.find( object ) == obj.end() )
        // top-level object && not already in window
        {
          hidden.insert( object );
          //cout << "Show object " << object->name() << endl;
        }
        else  // top level && in window
        {
          shown.insert( object );
          //cout << "Select object " << object->name() << endl;
        }
      }
    }
  }
}


void SelectFactory::registerSelector( const string & key, Selector* s )
{
  SelectFactory__selectors__init();

  map<string, Selector *>::iterator 
    is = SelectFactory__selectors().find( key );
  if( is != SelectFactory__selectors().end() )
    delete (*is).second;
  SelectFactory__selectors()[ key ] = s;
}


void SelectFactory::registerPostSelector( const string & key, 
					  PostSelector* ps )
{
  SelectFactory__postselectors()[ key ] = ps;
}


void SelectFactory::activatePostSelector( int priority, const string & ps )
{
  if( SelectFactory__postselectors().find( ps ) 
      != SelectFactory__postselectors().end() )
    SelectFactory__active_postsel().insert( pair<int,string>( priority, ps ) );
  else
    cerr << "Can't activate postselector " << ps << ": not registered!\n";
}


void SelectFactory::deactivatePostSelector( const string & ps )
{
  multimap<int, string>::iterator	ip, 
    ep = SelectFactory__active_postsel().end();

  for( ip=SelectFactory__active_postsel().begin(); ip!=ep && ip->second!=ps; 
       ++ip ) {}
  if( ip != ep )
    SelectFactory__active_postsel().erase( ip );
}


void SelectFactory::remove( AWindow* win )
{
  //cout << "remove\n";

  const map<unsigned, set<AObject *> >	& mo = selected();
  map<unsigned, set<AObject *> >::const_iterator im = mo.find( win->Group() );

  if( im == mo.end() )
    return;

  const set<AObject *>& obj = (*im).second;

  if( obj.size() > 0 )
    {
      set<AWindow *>	w = theAnatomist->getWindowsInGroup( win->Group() );
      Command 				*cmd;
      set<AObject *> 			tl;
      set<AObject *>::const_iterator	s;

      for( s=obj.begin(); s!=obj.end(); ++s )
	     tl.insert( *s );

      unselectAll( win->Group() );

      cmd = new RemoveObjectCommand( tl, w );
      theProcessor->execute( cmd );
    }
}


void SelectFactory::removeFromThisWindow( AWindow* win )
{
  //cout << "remove from this win\n";

  const map<unsigned, set<AObject *> >	& mo = selected();
  map<unsigned, set<AObject *> >::const_iterator im = mo.find( win->Group() );

  if( im == mo.end() )
    return;

  const set<AObject *>			& obj = (*im).second;

  if( obj.size() > 0 )
    {
      set<AWindow *>			w;
      Command 				*cmd;
      set<AObject *> tl;
      set<AObject *>::const_iterator	s;

      w.insert( win );
      for( s=obj.begin(); s!=obj.end(); ++s )
	     tl.insert( *s );

      unselectAll( win->Group() );

      cmd = new RemoveObjectCommand( tl, w );
      theProcessor->execute( cmd );
      refresh();
    }
}


void SelectFactory::refreshSelectionRendering() const
{
  std::map<unsigned, std::set<AObject *> >::const_iterator
    is, es = _selected().end();
  set<AWindow *>::const_iterator iw, ew;

  for( is=_selected().begin(); is!=es; ++is )
  {
    set<AWindow *> gw = theAnatomist->getWindowsInGroup( is->first );
    for( iw=gw.begin(), ew=gw.end(); iw!=ew; ++iw )
      (*iw)->Refresh();
  }
  _winToRefresh().clear();
}

