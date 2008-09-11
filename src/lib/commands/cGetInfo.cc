/* Copyright (c) 1995-2006 CEA
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

#include <anatomist/commands/cGetInfo.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/processor/Registry.h>
#include <anatomist/processor/unserializer.h>
#include <anatomist/processor/Serializer.h>
#include <anatomist/processor/context.h>
#include <anatomist/color/paletteList.h>
#include <anatomist/color/palette.h>
#include <anatomist/object/Object.h>
#include <anatomist/selection/selectFactory.h>
#include <anatomist/reference/transfSet.h>
#include <anatomist/application/module.h>
#include <graph/tree/tree.h>
#include <cartobase/object/syntax.h>
#include <cartobase/object/object.h>
#include <cartobase/object/pythonwriter.h>
#include <cartobase/config/info.h>
#include <fstream>

using namespace anatomist;
using namespace carto;
using namespace std;


GetInfoCommand::GetInfoCommand( const string & fname, CommandContext* context, 
				bool objects, bool windows, bool refs, 
				bool trans, bool pals, const string & nameobj,
				bool namewin, bool selects, bool link, 
				Referential* linkref, bool nameref, 
                                bool nametrans, const string & requestid,
                                bool version, bool listcommands,
                                bool aimsinfo, bool modinfo )
  : RegularCommand(), SerializingCommand( context ), _filename( fname ), 
    _objects( objects ), _windows( windows ), _refs( refs ), _trans( trans ), 
    _palettes( pals ), _nameobj( nameobj ), _namewin( namewin ), 
    _nameref( nameref ), _nametrans( nametrans ), 
    _selects( selects ), _link( link ), _linkref( linkref ), 
    _requestid( requestid ), _version( version ),
    _listcommands( listcommands ), _aimsinfo( aimsinfo ), _modinfo( modinfo )
{
}


GetInfoCommand::~GetInfoCommand()
{
}


bool GetInfoCommand::initSyntax()
{
  SyntaxSet	ss;
  Syntax	& s = ss[ "GetInfo" ];

  s[ "filename"               ].type = "string";
  s[ "objects"                ].type = "int";
  s[ "windows"                ].type = "int";
  s[ "referentials"           ].type = "int";
  s[ "transformations"        ].type = "int";
  s[ "palettes"               ].type = "int";
  s[ "name_objects"           ].type = "string";
  s[ "name_windows"           ].type = "int";
  s[ "name_referentials"      ].type = "int";
  s[ "name_transformations"   ].type = "int";
  s[ "selections"             ].type = "int";
  s[ "linkcursor_lastpos"     ].type = "int";
  s[ "linkcursor_referential" ].type = "int";
  s[ "request_id"             ].type = "string";
  s[ "version"                ] = Semantic( "int", false );
  s[ "list_commands"          ] = Semantic( "int", false );
  s[ "aims_info"              ] = Semantic( "int", false );
  s[ "modules_info"           ] = Semantic( "int", false );
  Registry::instance()->add( "GetInfo", &read, ss );
  return( true );
}


void
GetInfoCommand::doit()
{
  ofstream	f;
  ostream	*filep = &f;
  if( !_filename.empty() )
#if defined( __GNUC__ ) && ( __GNUC__ - 0 == 3 ) && ( __GNUC_MINOR__ - 0 < 3 )
    // ios::app doesn't work on pipes on gcc 3.0 to 3.2
    f.open( _filename.c_str(), ios::out );
#else
    f.open( _filename.c_str(), ios::out | ios::app );
#endif
  else
  {
    filep = context()->ostr;
    if( !filep )
    {
      cerr << "GetInfoCommand: no stream to write to\n";
      return;
    }
  }

  ostream	& file = *filep;
#ifndef _WIN32
  // on windows, a fdstream opened on a socket seems to return wrong state
  if( !file )
  {
    cerr << "warning: could not open output file " << _filename << endl;
    return;
  }
#endif

  Object	ex( (GenericObject *) new ValueObject<Dictionary> );

  if( _version )
  {
    ex->setProperty( "anatomist_version", theAnatomist->versionString() );
  }

  if( _aimsinfo )
  {
    ostringstream  s;
    Info::print( s );
    ex->setProperty( "aims_info", s.str() );
  }

  if( _modinfo )
  {
    ModuleManager			*mm = ModuleManager::instance();
    ModuleManager::const_iterator	im, em=mm->end();
    ex->setProperty( "modules", Dictionary() );
    Dictionary & mods = ex->getProperty( "modules" )->value<Dictionary>();

    for( im=mm->begin(); im!=em; ++im )
    {
      Dictionary  modinfo;
      modinfo[ "description" ] = Object::value( (*im)->description() );
      mods[ (*im)->name() ] = Object::value( modinfo );
    }
  }

  if( _listcommands )
  {
    const SyntaxSet & syntax = Registry::instance()->syntax();
    SyntaxSet::const_iterator ist, est = syntax.end();
    Syntax::const_iterator  is, es;
    ex->setProperty( "commands", Dictionary() );
    Dictionary & cmds = ex->getProperty( "commands" )->value<Dictionary>();
    for( ist=syntax.begin(); ist!=est; ++ist )
    {
      if( ist->first != "EXECUTE" && ist->first != "UNDO"
          && ist->first != "REDO" )
      {
        Dictionary atts;
        for( is=ist->second.begin(), es=ist->second.end(); is!=es; ++is )
        {
          const Semantic  & s = is->second;
          if( !s.internal )
          {
            Dictionary descr;
            descr[ "type" ] = Object::value( s.type );
            descr[ "needed" ] = Object::value( (int) s.needed );
            atts[ is->first ] = Object::value( descr );
          }
        }
        cmds[ ist->first ] = Object::value( atts );
      }
    }
  }

  if( context()->unserial )
    {
      const map<int, void *>		& ids = context()->unserial->ids();
      map<int, void *>::const_iterator	io, eo = ids.end();
      int				nameobj = 0;

      if( _nameobj == "all" || _nameobj == "1" || _nameobj == "yes" )
	nameobj = 2;
      else if( _nameobj == "top" )
	nameobj = 1;

      context()->unserial->garbageCollect();
      if( _objects )
	{
	  set<AObject *>	doneobj;
	  vector<int>		objl;

	  // known objects first
	  for( io=ids.begin(); io!=eo; ++io )
	    if( context()->unserial->type( io->second ) == "AObject" )
	      {
		objl.push_back( io->first );
		doneobj.insert( (AObject *) io->second );
	      }

	  // now new objects
	  if( nameobj >= 1 )
	    {
	      set<AObject *>		obj = theAnatomist->getObjects();
	      set<AObject *>::iterator	iob, eob = obj.end(), 
		todo = doneobj.end();
	      int			id;

	      for( iob=obj.begin(); iob!=eob; ++iob )
		if( doneobj.find( *iob ) == todo 
		    && ( nameobj > 1 || (*iob)->Parents().empty() ) )
		  {
		    id = context()->unserial->makeID( *iob, "AObject" );
		    objl.push_back( id );
		  }
	    }

	  ex->setProperty( "objects", Object::value( objl ) );
	}
      if( _windows )
	{
	  io = ids.begin();

	  set<AWindow *>	donewin;
	  vector<int>		winl;

	  // known windows first
	  for( io=ids.begin(); io!=eo; ++io )
	    if( context()->unserial->type( io->second ) == "AWindow"  )
	      {
		winl.push_back( io->first );
		donewin.insert( (AWindow *) io->second );
	      }

	  // now new windows
	  if( _namewin )
	    {
	      set<AWindow *>		win = theAnatomist->getWindows();
	      set<AWindow *>::iterator	iw, ew = win.end(), 
		todo = donewin.end();
	      int			id;

	      for( iw=win.begin(); iw!=ew; ++iw )
		if( donewin.find( *iw ) == todo )
		  {
		    id = context()->unserial->makeID( *iw, "AWindow" );
		    winl.push_back( id );
		  }
	    }

	  ex->setProperty( "windows", Object::value( winl ) );
	}
      if( _refs )
	{
	  vector<int>		refl;
	  set<Referential *>	doneref;

	  io = ids.begin();

	  // known refs first
	  for( io=ids.begin(); io!=eo; ++io )
	    if(  context()->unserial->type( io->second ) == "Referential" )
              {
                refl.push_back( io->first );
                doneref.insert( (Referential *) io->second );
              }

	  // now new refs
	  if( _nameref )
	    {
	      set<Referential *>	ref = theAnatomist->getReferentials();
	      set<Referential *>::iterator	ir, er = ref.end(), 
		todo = doneref.end();
	      int			id;

	      for( ir=ref.begin(); ir!=er; ++ir )
		if( doneref.find( *ir ) == todo )
		  {
		    id = context()->unserial->makeID( *ir, "Referential" );
		    refl.push_back( id );
		  }
	    }

	  ex->setProperty( "referentials", Object::value( refl ) );
	}
      if( _trans )
	{
	  vector<int>		trnl;
	  set<Transformation *>	donetrans;

	  // known trans first
	  io = ids.begin();
	  for( io=ids.begin(); io!=eo; ++io )
	    if( context()->unserial->type( io->second ) == "Transformation" )
              {
                trnl.push_back( io->first );
                donetrans.insert( (Transformation *) io->second );
              }

	  // now new transformations
	  if( _nametrans )
	    {
	      set<Transformation *> 
                trans = ATransformSet::instance()->allTransformations();
	      set<Transformation *>::iterator	it, et = trans.end(), 
		todo = donetrans.end();
	      int			id;

	      for( it=trans.begin(); it!=et; ++it )
		if( donetrans.find( *it ) == todo )
		  {
		    id = context()->unserial->makeID( *it, "Transformation" );
		    trnl.push_back( id );
		  }
	    }

	  ex->setProperty( "transformations", Object::value( trnl ) );
	}
      if( _palettes )
	{
	  vector<string>	pall;
	  const list<rc_ptr<APalette> >
            &pl = theAnatomist->palettes().palettes();
	  list<rc_ptr<APalette> >::const_iterator	ip, ep = pl.end();
	  for( ip=pl.begin(); ip!=ep; ++ip )
	    pall.push_back( (*ip)->name() );
	  ex->setProperty( "palettes", Object::value( pall ) );
	}
      if( _selects )
	{
	  SelectFactory	& sf = *SelectFactory::factory();
	  const map<unsigned, set<AObject *> >	& sels = sf.selected();
	  map<unsigned, set<AObject *> >::const_iterator is, es = sels.end();
	  Object	selid = Object::value( IntDictionary() );

	  for( io=ids.begin(); io!=eo; ++io )
	    if( context()->unserial->type( io->second ) == "AObject" )
	      for( is=sels.begin(); is!=es; ++is )
		if( is->second.find( (AObject *) io->second ) 
		    != is->second.end() )
		  {
		    Object s = selid->getArrayItem( is->first );
		    if( !s.get() )
                      {
                        s = Object::value( vector<int>() );
                        selid->setArrayItem( is->first, s );
                      }
		    vector<int>	& v 
                      = s->value<vector<int> >();
		    v.push_back( io->first );
		  }
	  ex->setProperty( "selections", Object::value( selid ) );
	}
    }

  if( _link )
    {
      vector<float>	lpos( 3 );
      Point3df		pos = theAnatomist->lastPosition( _linkref );
      lpos[0] = pos[0];
      lpos[1] = pos[1];
      lpos[2] = pos[2];
      ex->setProperty( "linkcursor_position", Object::value( lpos ) );
    }

  _result = ex;
  if( !_requestid.empty() )
    ex->setProperty( "request_id", _requestid );

  file << "'GetInfo'" << endl;
  PythonWriter	pw;
  pw.setSingleLineMode( true );
  pw.attach( file );
  pw.write( *ex, false, false );
  file << endl << flush;
}


Command* GetInfoCommand::read( const Tree & com, CommandContext* context )
{
  string	fname, nameobj, rid;
  int		obj = 0, win = 0, refs = 0, trans = 0, pal = 0, namewin = 0, 
    sel = 0, link = 0, linkref = -1, nameref = 0, nametrans = 0, version = 0,
    listcommands = 0, aimsinfo = 0, modinfo = 0;
  Referential	*ref = 0;

  com.getProperty( "objects", obj );
  com.getProperty( "windows", win );
  com.getProperty( "referentials", refs );
  com.getProperty( "transformations", trans );
  com.getProperty( "palettes", pal );
  com.getProperty( "filename", fname );
  com.getProperty( "name_objects", nameobj );
  com.getProperty( "name_windows", namewin );
  com.getProperty( "name_referentials", nameref );
  com.getProperty( "name_transformations", nametrans );
  com.getProperty( "selections", sel );
  com.getProperty( "linkcursor_lastpos", link );
  com.getProperty( "linkcursor_referential", linkref );
  com.getProperty( "request_id", rid );
  com.getProperty( "version", version );
  com.getProperty( "list_commands", listcommands );
  com.getProperty( "aims_info", aimsinfo );
  com.getProperty( "modules_info", modinfo );
  if( link )
  {
    if( linkref >= 0 && context->unserial )
      ref = (Referential *) context->unserial->pointer( linkref, 
                                                        "Referential" );
    else
      ref = theAnatomist->centralReferential();
  }

  return new GetInfoCommand( fname, context, (bool) obj, (bool) win,
                             (bool) refs, (bool) trans, (bool) pal, nameobj,
                             (bool) namewin, (bool) sel, (bool) link, ref,
                             (bool) nameref, (bool) nametrans, rid,
                             (bool) version, (bool) listcommands,
                             (bool) aimsinfo, (bool) modinfo
                           );
}


void GetInfoCommand::write( Tree & com, Serializer* ser ) const
{
  Tree	*t = new Tree( true, name() );

  if( !_filename.empty() )
    t->setProperty( "filename", _filename );
  if( _objects )
    t->setProperty( "objects", (int) 1 );
  if( _windows )
    t->setProperty( "windows", (int) 1 );
  if( _refs )
    t->setProperty( "referentials", (int) 1 );
  if( _trans )
    t->setProperty( "transformations", (int) 1 );
  if( _palettes )
    t->setProperty( "palettes", (int) 1 );
  if( !_nameobj.empty() && _nameobj != "0" && _nameobj != "no" )
    t->setProperty( "name_objects", _nameobj );
  if( _namewin )
    t->setProperty( "name_windows", (int) 1 );
  if( _nameref )
    t->setProperty( "name_referentials", (int) 1 );
  if( _nametrans )
    t->setProperty( "name_transformations", (int) 1 );
  if( _selects )
    t->setProperty( "selects", (int) 1 );
  if( !_requestid.empty() )
    t->setProperty( "request_id", _requestid );
  if( _link )
  {
    t->setProperty( "linkcursor_lastpos", (int) 1 );
    if( _linkref && ser )
      t->setProperty( "linkcursor_referential",
                        ser->serialize( _linkref ) );
  }
  if( _version )
    t->setProperty( "version", (int) 1 );
  if( _listcommands )
    t->setProperty( "list_commands", (int) 1 );
  if( _aimsinfo )
    t->setProperty( "aims_info", (int) 1 );
  if( _modinfo )
    t->setProperty( "modules_info", (int) 1 );
  com.insert( t );
}


Object GetInfoCommand::result()
{
  return _result;
}



