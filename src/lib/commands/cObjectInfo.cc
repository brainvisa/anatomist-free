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

#include <anatomist/surface/glcomponent.h>
#include <anatomist/commands/cObjectInfo.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/processor/Registry.h>
#include <anatomist/processor/unserializer.h>
#include <anatomist/processor/Serializer.h>
#include <cartobase/object/syntax.h>
#include <graph/tree/tree.h>
#include <fstream>
#include <anatomist/mobject/MObject.h>
#include <anatomist/window/Window.h>
#include <anatomist/window/winFactory.h>
#include <anatomist/window3D/window3D.h>
#include <anatomist/window/glwidget.h>
#include <anatomist/color/objectPalette.h>
#include <anatomist/color/palette.h>
#include <anatomist/reference/Referential.h>
#include <anatomist/reference/Transformation.h>
#include <anatomist/reference/transfSet.h>
#include <anatomist/processor/context.h>
#include <aims/resampling/quaternion.h>
#include <cartobase/object/object.h>
#include <cartobase/object/pythonwriter.h>
#include <cartobase/stream/fileutil.h>

using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;


ObjectInfoCommand::ObjectInfoCommand( const string & fname, 
				      CommandContext* context, 
				      const vector<int> & objects, 
				      bool namechildren, bool nameref, 
                                      const string & requestid,
                                      const set<string> & objfilenames ) 
  : RegularCommand(), SerializingCommand( context ), _filename( fname ), 
    _objects( objects ), _nameChildren( namechildren ), _nameref( nameref ), 
    _requestid( requestid ), _objfilenames( objfilenames )
{
}


ObjectInfoCommand::~ObjectInfoCommand()
{
}


bool ObjectInfoCommand::initSyntax()
{
  SyntaxSet	ss;
  Syntax	& s = ss[ "ObjectInfo" ];
  
  s[ "filename"          ] = Semantic( "string" );
  s[ "objects"           ] = Semantic( "int_vector", false );
  s[ "name_children"     ] = Semantic( "int" );
  s[ "name_referentials" ] = Semantic( "int" );
  s[ "request_id"        ] = Semantic( "string" );
  s[ "objects_filenames" ] = Semantic( "string", false );
  Registry::instance()->add( "ObjectInfo", &read, ss );
  return( true );
}


namespace
{

  void printObject( AObject* ao, Object ex, map<void *, int> & ptrs, 
                    Unserializer & unser, bool namechild, bool nameref )
  {
    if( !theAnatomist->hasObject( ao ) )
      {
        ex->setProperty( "dead", (int) 1 );
        return;
      }

    map<void *, int>::iterator		ipt, ept = ptrs.end();
    string				fname = ao->fileName();
    const set<AWindow *>			& wl = ao->WinList();
    set<AWindow *>::const_iterator	iw, ew = wl.end();
    int					id;

    ex->setProperty( "objectType", AObject::objectTypeName( ao->type() ) );
    ex->setProperty( "name", ao->name() );
    if( !fname.empty() )
      ex->setProperty( "filename", fname );
    ex->setProperty( "multiObject", (int) ao->isMultiObject() );
    ex->setProperty( "loadDate", ao->loadDate() );
    ex->setProperty( "copy", (int) ao->isCopy() );

    vector<int>	winl;
    for( iw=wl.begin(); iw!=ew; ++iw )
      if( ( ipt = ptrs.find( *iw ) ) != ept )
        winl.push_back( ipt->second );
    if( !winl.empty() )
      ex->setProperty( "windows", winl );
    //	referential
    Referential	*ref = ao->getReferential();
    if( ref )
      {
        ipt = ptrs.find( ref );
        id = -1;
        if( ipt != ept )
          id = ipt->second;
        else if( nameref )
          {
            id = unser.makeID( ref, "Referential" );
            ptrs[ ref ] = id;
          }
        if( id >= 0 )
          ex->setProperty( "referential", id );
      }

    // parents
    AObject::ParentList::const_iterator 
      ipl, epl = ao->Parents().end();
    vector<int>	parl;
    for( ipl=ao->Parents().begin(); ipl!=epl; ++ipl )
      if( ( ipt = ptrs.find( *ipl ) ) != ept )
        parl.push_back( ipt->second );
    if( !parl.empty() )
      ex->setProperty( "parents", parl );

    // children
    MObject	*mo = dynamic_cast<MObject *>( ao );
    if( mo )
      {
        vector<int>	chdl;
        MObject::const_iterator	im, em = mo->end();
        for( im=mo->begin(); im!=em; ++im )
          {
            ipt = ptrs.find( *im );
            if( ipt == ept && namechild )
              {
                int	id = unser.makeID( *im, "AObject" );
                ipt = ptrs.insert( pair<void*,int>( *im, id ) ).first;
              }
            if( ipt != ept )
              {
                chdl.push_back( ipt->second );
              }
          }
        if( !chdl.empty() )
          ex->setProperty( "children", chdl );
      }

    // material
    Material	& mat = ao->GetMaterial();
    Object      m = mat.genericDescription();
    ex->setProperty( "material", m );
    GLComponent  *gl = ao->glAPI();
    Object	t( (GenericObject *) 
		   new ValueObject<Dictionary> );
    ex->setProperty( "texture", t );

    if( gl && gl->glNumTextures() > 0 )
      {
        GLComponent::TexExtrema  & te = gl->glTexExtrema( 0 );
        if( !te.minquant.empty() )
          t->setProperty( "textureMin", te.minquant[0] );
        if( !te.maxquant.empty() )
          t->setProperty( "textureMax", te.maxquant[0] );
      }
  
    // palette
    AObjectPalette	*pal = ao->palette();
    if( pal )
      {
        Object	p( (GenericObject *) new ValueObject<Dictionary> );
        ex->setProperty( "palette", p );
        p->setProperty( "palette", pal->refPalette()->name() );
        if( pal->refPalette2() )
          p->setProperty( "palette2", pal->refPalette2()->name() );
        p->setProperty( "min", pal->min1() );
        p->setProperty( "max", pal->max1() );
        p->setProperty( "min2", pal->min2() );
        p->setProperty( "max2", pal->max2() );
        p->setProperty( "mixMethod", pal->mixMethodName() );
        p->setProperty( "linMixFactor", pal->linearMixFactor() );
        p->setProperty( "colorMixSize", pal->colors()->dimX() );
      }

    // selected
    SelectFactory				*sf = SelectFactory::factory();
    const map<unsigned, set<AObject *> >	& sel = sf->selected();
    map<unsigned, set<AObject *> >::const_iterator	is, es = sel.end();
    set<unsigned>	selgroups;

    for( is=sel.begin(); is!=es; ++is )
      if( is->second.find( ao ) != is->second.end() )
        selgroups.insert( is->first );
    if( !selgroups.empty() )
      ex->setProperty( "selected_in_groups", selgroups );
  }


  void printWindow( AWindow* w, Object ex, map<void *, int> & ptrs, 
                    Unserializer & unser, bool namechild, bool nameref )
  {
    if( !theAnatomist->hasWindow( w ) )
      {
        ex->setProperty( "dead", (int) 1 );
        return;
      }

    ex->setProperty( "windowType", 
                     AWindowFactory::typeString( w->type(), w->subtype() ) );
    ex->setProperty( "group", w->Group() );
    Point3df	pos = w->GetPosition();
    vector<float>	posv( 4 );
    posv[0] = pos[0];
    posv[1] = pos[1];
    posv[2] = pos[2];
    posv[3] = w->GetTime();
    ex->setProperty( "position", posv );

    map<void *, int>::const_iterator	ipt, ept = ptrs.end();

    //	referential
    Referential	*ref = w->getReferential();
    int					id;
    if( ref )
      {
        ipt = ptrs.find( ref );
        id = -1;
        if( ipt != ept )
          id = ipt->second;
        else if( nameref )
          {
            id = unser.makeID( ref, "Referential" );
            ptrs[ ref ] = id;
          }
        if( id >= 0 )
          ex->setProperty( "referential", id );
      }

    QAWindow	*qw = dynamic_cast<QAWindow *>( w );
    if( qw )
      {
        vector<int>	geo( 4 );
        geo[0] = qw->x();
        geo[1] = qw->y();
        geo[2] = qw->width();
        geo[3] = qw->height();
        ex->setProperty( "geometry", geo );
      }

    AWindow3D	*w3 = dynamic_cast<AWindow3D *>( w );
    if( w3 )
      {
        Point4df	q = w3->sliceQuaternion().vector();
        vector<float>	vc( 4 );
        vc[0] = q[0];
        vc[1] = q[1];
        vc[2] = q[2];
        vc[3] = q[3];
        ex->setProperty( "slice_quaternion", vc );
        GLWidgetManager *v = dynamic_cast<GLWidgetManager *>( w3->view() );
        if( v )
          {
            vector<int>	vi( 2 );
            vi[0] = v->width();
            vi[1] = v->height();
            ex->setProperty( "view_size", vi );
            ex->setProperty( "zoom", v->zoom() );
            Point3df	p = v->rotationCenter();
            vc = vector<float>(3);
            vc[0] = p[0];
            vc[1] = p[1];
            vc[2] = p[2];
            ex->setProperty( "observer_position", vc );
            Point3df	bbmin = v->windowBoundingMin(), 
              bbmax = v->windowBoundingMax();
            vc[0] = bbmin[0];
            vc[1] = bbmin[1];
            vc[2] = bbmin[2];
            ex->setProperty( "boundingbox_min", vc );
            vc[0] = bbmax[0];
            vc[1] = bbmax[1];
            vc[2] = bbmax[2];
            ex->setProperty( "boundingbox_max", vc );
            q = v->quaternion().vector();
            vc[0] = q[0];
            vc[1] = q[1];
            vc[2] = q[2];
            vc.push_back( q[3] );
            ex->setProperty( "view_quaternion", vc );
          }
      }

    const set<AObject *>			& ol = w->Objects();
    set<AObject *>::const_iterator	io, eo = ol.end();
    vector<int>				objl;

    for( io=ol.begin(); io!=eo; ++io )
      {
        if( ( ipt = ptrs.find( *io ) ) == ept && namechild )
          {
            int	id = unser.makeID( *io, "AObject" );
            ipt = ptrs.insert( pair<void*,int>( *io, id ) ).first;
          }
        if( ipt != ept )
          objl.push_back( ipt->second );
      }
    if( !objl.empty() )
      ex->setProperty( "objects", objl );
  }


  void printTransformation( Transformation* t, Object ex, 
                            map<void *, int> & ptrs, Unserializer & unser, 
                            bool nameref )
  {
    if( !ATransformSet::instance()->hasTransformation( t ) )
      {
        ex->setProperty( "dead", (int) 1 );
        return;
      }

    vector<float>	r( 9 );
    r[0] = t->Rotation( 0, 0 );
    r[1] = t->Rotation( 0, 1 );
    r[2] = t->Rotation( 0, 2 );
    r[3] = t->Rotation( 1, 0 );
    r[4] = t->Rotation( 1, 1 );
    r[5] = t->Rotation( 1, 2 );
    r[6] = t->Rotation( 2, 0 );
    r[7] = t->Rotation( 2, 1 );
    r[8] = t->Rotation( 2, 2 );
    ex->setProperty( "rotation_matrix", r );
    vector<float>	tl( 3 );
    tl[0] = t->Translation( 0 );
    tl[1] = t->Translation( 1 );
    tl[2] = t->Translation( 2 );
    ex->setProperty( "translation", tl );

    int				id;
    Referential			*ref = t->source();
    map<void *, int>::iterator	i = ptrs.find( ref ), e = ptrs.end();

    id = -1;
    if( i == e )
      {
        id = unser.makeID( ref, "Referential" );
        ptrs[ ref ] = id;
      }
    else if( nameref )
      id = i->second;
    if( id >= 0 )
      ex->setProperty( "source", id );

    ref = t->destination();
    i = ptrs.find( t->destination() );
    id = -1;
    if( i == e )
      {
        id = unser.makeID( ref, "Referential" );
        ptrs[ ref ] = id;
      }
    else if( nameref )
      id = i->second;
    if( id >= 0 )
      ex->setProperty( "destination", id );

    const PythonHeader *hdr = t->motion().header();
    if( hdr )
    {
      Object i = hdr->objectIterator();
      for( ; i->isValid(); i->next() )
        ex->setProperty( i->key(), i->currentValue() );
    }
  }

  void printReferential( Referential* r, Object ex, set<Referential *> & refs )
  {
    if( refs.empty() )
      refs = theAnatomist->getReferentials();
    if( refs.find( r ) == refs.end() )
    {
      ex->setProperty( "dead", (int) 1 );
      return;
    }
    const PythonHeader & hdr = r->header();
    Object i = hdr.objectIterator();
    for( ; i->isValid(); i->next() )
      ex->setProperty( i->key(), i->currentValue() );
  }

} // namespace


void
ObjectInfoCommand::doit()
{
  // cout << "ObjectInfoCommand::doit\n";
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

  Object	ex( (GenericObject *) new ValueObject<map<Object, Object> > );
  map<Object, Object>	& exm = ex->value<map<Object, Object> >();

  if( context()->unserial )
  {
    vector<int>::const_iterator	ie, ee = _objects.end();
    void				*ptr;
    string				type;
    const map<int, void *>		& ids = context()->unserial->ids();
    map<void *, int>			ptrs;
    map<int, void *>::const_iterator	iid, eid = ids.end();

    // make reverse map
    for( iid=ids.begin(); iid!=eid; ++iid )
      ptrs[ iid->second ] = iid->first;

    set<Referential *>   refs;

    for( ie=_objects.begin(); ie!=ee; ++ie )
      {
        ptr = context()->unserial->pointer( *ie );
        if( ptr )
          {
            Object ex2( (GenericObject *)
                        new ValueObject<Dictionary> );
            exm[ Object::value( *ie ) ] = ex2;
            type = context()->unserial->type( ptr );
            ex2->setProperty( "type", type );

            if( type == "AObject" )
              printObject( (AObject *) ptr, ex2, ptrs,
                            *context()->unserial, _nameChildren, _nameref );
            else if( type == "AWindow" )
              printWindow( (AWindow *) ptr, ex2, ptrs,
                            *context()->unserial, _nameChildren, _nameref );
            else if( type == "Transformation" )
              printTransformation( (Transformation *) ptr, ex2, ptrs,
                                    *context()->unserial, _nameref );
            else if( type == "Referential" )
              printReferential( (Referential *) ptr, ex2, refs );
          }
      }

    if( !_objfilenames.empty() )
    {
      // cout << "filenames: " << _objfilenames.size() << endl;
      set<string>::const_iterator efo = _objfilenames.end();
      list<AObject *> selobj;
      set<AObject *> objs = theAnatomist->getObjects();
      set<AObject *>::const_iterator  io, eo = objs.end();
      string  fname;
      AObject *ao;
      int     id;
      bool    found;
      map<Object, Object>::const_iterator imoo, emoo = exm.end();
      list<AObject *>::const_iterator ilo, elo;

      for( io=objs.begin(); io!=eo; ++io )
      {
        fname = (*io)->fileName();
        if( !fname.empty() && _objfilenames.find( fname ) != efo )
          selobj.push_back( *io );
      }

      for( ilo=selobj.begin(), elo=selobj.end(); ilo!=elo; ++ilo )
      {
        ao = *ilo;

        try
        {
          id = context()->unserial->id( ao );
        }
        catch( exception & )
        {
          id = context()->unserial->makeID( ao, "AObject" );
        }
        found = false;
        for( imoo=exm.begin(); imoo!=emoo; ++imoo )
          try
          {
            if( imoo->first->value<int>() == id )
            {
              found = true;
              break;
            }
          }
          catch( ... )
          {
          }
        if( !found )
        {
          Object ex2( (GenericObject *)
              new ValueObject<Dictionary> );
          exm[ Object::value( id ) ] = ex2;
          ex2->setProperty( "type", string( "AObject" ) );
          printObject( ao, ex2, ptrs, *context()->unserial, _nameChildren,
                      _nameref );
        }
      }
    }
  }

  if( !_requestid.empty() )
    exm[ Object::value( string( "request_id" ) ) ] 
      = Object::value( _requestid );

  _result = ex;

  // cout << "ObjectInfoCommand::doit writing\n";

  file << "'ObjectInfo'\n";
  PythonWriter	pw;
  pw.setSingleLineMode( true );
  pw.attach( file );
  pw.write( *ex, false, false );
  file << endl << flush;

  // cout << "ObjectInfoCommand::doit done\n";
}


Command* ObjectInfoCommand::read( const Tree & com, CommandContext* context )
{
  // cout << "ObjectInfoCommand::read\n";
  string	fname;
  vector<int>	obj;
  int		namech = 0, nameref = 0;
  string	rid, fnames;
  set<string>   objfnames;

  com.getProperty( "filename", fname );
  com.getProperty( "objects", obj );
  com.getProperty( "name_children", namech );
  com.getProperty( "name_referentials", nameref );
  com.getProperty( "request_id", rid );
  com.getProperty( "objects_filenames", fnames );
  list<string> fnamesl = FileUtil::filenamesSplit( fnames, " " );
  objfnames.insert( fnamesl.begin(), fnamesl.end() );

  return new ObjectInfoCommand( fname, context, obj, (bool)
				namech, (bool) nameref, rid, objfnames );
}


void ObjectInfoCommand::write( Tree & com, Serializer* ) const
{
  Tree	*t = new Tree( true, name() );

  if( !_filename.empty() )
    t->setProperty( "filename", _filename );
  if( !_objects.empty() )
    t->setProperty( "objects", _objects );
  if( _nameChildren )
    t->setProperty( "name_children", (int) 1 );
  if( _nameref)
    t->setProperty( "name_referentials", (int) 1 );
  if( !_requestid.empty() )
    t->setProperty( "request_id", _requestid );
  if( !_objfilenames.empty() )
  {
    string  fnames;
    bool  first = true;
    set<string>::const_iterator is, es = _objfilenames.end();
    for( is=_objfilenames.begin(); is!=es; ++is )
    {
      if( first )
        first = false;
      else
        fnames += ' ';
      fnames += string( "\"" ) + *is + '"';
    }
    t->setProperty( "objects_filenames", fnames );
  }
  com.insert( t );
}


Object ObjectInfoCommand::result()
{
  return _result;
}


