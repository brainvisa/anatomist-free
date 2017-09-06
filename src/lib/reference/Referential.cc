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


//--- header files ------------------------------------------------------------

#include <anatomist/reference/Referential.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/commands/cAssignReferential.h>
#include <anatomist/commands/cLoadTransformation.h>
#include <anatomist/processor/Processor.h>
#include <anatomist/reference/transfSet.h>
#include <anatomist/reference/Transformation.h>
#include <anatomist/graph/pythonAObject.h>
#include <aims/resampling/standardreferentials.h>
#include <cartobase/exception/file.h>
#include <cartobase/stream/fileutil.h>
#include <cartobase/config/paths.h>

using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;

namespace
{

  void genuuid( PythonHeader & ph )
  {
    carto::UUID	uuid;
    uuid.generate();
    ph.setProperty( "uuid", uuid.toString() );
  }

  Referential*& mniref()
  {
    static Referential *ref = 0;
    return ref;
  }

  Referential*& giftiTalairachRef()
  {
    static Referential *ref = 0;
    return ref;
  }

}


//--- methods -----------------------------------------------------------------

Referential::Referential()
  : _indCol( 0 ), _destroying( false ), _header( new PythonHeader )
{
  _color = NewColor();
  genuuid( *_header );
  theAnatomist->registerReferential(this);
}


Referential::Referential( const string & filename )
  : _indCol( 0 ), _destroying( false ), _header( 0 )
{
  _color = NewColor();
  if( !load( filename ) )
    genuuid( *_header );
  theAnatomist->registerReferential(this);
}


Referential::Referential(set<AObject*>& objL)
  : _indCol( 0 ), _destroying( false ), _header( new PythonHeader )
{
  _anaObj = objL;
  _color = NewColor();
  genuuid( *_header );
  theAnatomist->registerReferential(this);
}

Referential::Referential(set<AWindow*>& winL)
  : _indCol( 0 ), _destroying( false ), _header( new PythonHeader )
{
  _anaWin = winL;
  _color = NewColor();
  genuuid( *_header );
  theAnatomist->registerReferential(this);
}


Referential::Referential( const Referential& ref )
  : _indCol( 0 ), _destroying( false ), _header( new PythonHeader )
{
  *this = ref;
  _indCol = 0;
  _color = NewColor();
  genuuid( *_header );
  theAnatomist->registerReferential(this);
}

Referential::~Referential()
{
  _destroying = true;
  if( this == mniref() )
    mniref() = 0;
  else if( this == giftiTalairachRef() )
    giftiTalairachRef() = 0;
  AObject  *o;
  while( !_anaObj.empty() )
    {
      o = *_anaObj.begin();
      if( o->referentialInheritance() == 0 )
        o->setReferential( 0 );
      else
        o->setReferentialInheritance( o->referentialInheritance() );
    }

  while( !_anaWin.empty() )
    (*_anaWin.begin())->setReferential( 0 );

  theAnatomist->unregisterReferential(this);
  delete _header;
}

Referential & Referential::operator = ( const Referential & r )
{
  if( this != &r )
    {
      _anaObj = r._anaObj;
      _anaWin = r._anaWin;
      // _color = r._color;
      // _indCol = r._indCol; // should not be copied ?
      string	uuid;
      _header->getProperty( "uuid", uuid );
      _header->copy( *r._header );
      if( !uuid.empty() ) // keep old UUID
        _header->setProperty( "uuid", uuid );
    }
  return *this;
}


void Referential::setColor( const AimsRGB & col )
{
  _color = col;
}


namespace
{

  void fillColorList( vector<Point3df> & color_list )
  {
    color_list.reserve( 14 );
    color_list.push_back( Point3df( 1., 0., 0. ) );
    color_list.push_back( Point3df( 0., 1., 0. ) );
    color_list.push_back( Point3df( 1., 1., 0. ) );
    color_list.push_back( Point3df( 0., 0., 1. ) );
    color_list.push_back( Point3df( 1., 0., 1. ) );
    color_list.push_back( Point3df( 0., 1., 1. ) );
    color_list.push_back( Point3df( 1., 1., 1. ) );
    color_list.push_back( Point3df( 0., 0., 0. ) );
    color_list.push_back( Point3df( 1., 0.5, 0. ) );
    color_list.push_back( Point3df( 0.5, 1., 0. ) );
    color_list.push_back( Point3df( 0., 1., 0.5 ) );
    color_list.push_back( Point3df( 0., 0.5, 1. ) );
    color_list.push_back( Point3df( 1., 0., 0.5 ) );
    color_list.push_back( Point3df( 0.5, 0., 1. ) );
  }

}


AimsRGB Referential::NewColor()
{
  static vector<Point3df> color_list;
  if( color_list.empty() )
    fillColorList( color_list );

  set<Referential*> lisref;
  set<Referential*>::iterator ref;
  AimsRGB col;
  size_t nobject = color_list.size();

  lisref = theAnatomist->getReferentials();

  if( lisref.empty() )
    _indCol = 0;
  else
    {
      set<int>			indices;
      set<int>::iterator	ii, ei = indices.end();

      for( ref=lisref.begin(); ref!=lisref.end(); ++ref )
        if( *ref != this )
          indices.insert( (*ref)->index() );
      for( ii=indices.begin(), _indCol=0; ii!=ei && _indCol==*ii; 
           ++ii, ++_indCol ) {}
    }


  float saturation = 1. - std::exp( -float( _indCol / nobject ) / 3. );
  float brightness = std::exp( -float( _indCol / nobject ) / 3. );
  int	indCol = _indCol % nobject;
  Point3df base_col = color_list[indCol], colf;

  colf = ( base_col * (1. - saturation ) + Point3df( saturation ) )
    * brightness;

  col.red() = (unsigned short)( 255. * colf[0] );
  col.green() = (unsigned short)( 255. * colf[1] );
  col.blue() = (unsigned short)( 255. * colf[2] );

  return col;
}

void Referential::AddObject(AObject* obj)
{
  set<AObject*>::iterator iobj;
  bool trouve = false;

  iobj = _anaObj.find( obj );
  if ( iobj != _anaObj.end() )
    trouve = true;

  if (!trouve) _anaObj.insert(obj);
}

void Referential::AddWindow(AWindow* win)
{
  set<AWindow*>::iterator iwin;
  bool trouve = false;

  iwin = _anaWin.find( win );
  if ( iwin != _anaWin.end() )
    trouve = true;


  if (!trouve) _anaWin.insert(win);
}

void Referential::RemoveObject(AObject* obj)
{
  set<AObject*>::iterator iobj;


  iobj = _anaObj.find( obj );
  if ( iobj != _anaObj.end() )
    _anaObj.erase( iobj );
}

void Referential::RemoveWindow(AWindow* win)
{
  set<AWindow*>::iterator iwin;

  iwin = _anaWin.find( win );
  if ( iwin != _anaWin.end() )
    _anaWin.erase( iwin );
}


carto::UUID Referential::uuid() const
{
  string	id;
  if( _header->getProperty( "uuid", id ) )
    return carto::UUID( id );
  return carto::UUID();
}


Referential* Referential::referentialOfUUID( const carto::UUID & uuid )
{
  return referentialOfUUID( uuid.toString() );
}


Referential* Referential::referentialOfUUID( const string & uuid )
{
  set<Referential *>	refs = theAnatomist->getReferentials();
  set<Referential *>::const_iterator	i, e = refs.end();
  string		id;

  for( i=refs.begin(); i!=e; ++i )
    if( (*i)->header().getProperty( "uuid", id ) 
        && id == uuid )
      return *i;
  return 0;
}


bool Referential::load( const string & filename )
{
  PythonHeader  *ph = new PythonHeader;
  try
  {
    ph->readMinf( filename );
    delete _header;
    _header = ph;
    return true;
  }
  catch( file_error & )
  {
  }
  catch( exception & e )
  {
    cerr << e.what() << endl;
  }
  catch( ... )
  {
    cerr << "Unknown exception" << endl;
  }
  delete ph;
  if( !_header )
    _header = new PythonHeader;
  return false;
}


string Referential::filename() const
{
  return _filename;
}


bool Referential::isDirect() const
{
  bool  direct = false; // default is not direct
  try
  {
    Object d = _header->getProperty( "direct_referential" );
    direct = (bool) d->getScalar();
  }
  catch( ... )
  {
    Referential     *cr = acPcReferential();
    ATransformSet   *ts = ATransformSet::instance();
    Transformation  *t = ts->transformation( cr, this );
    if( t )
      direct = cr->isDirect() ^ !t->isDirect();
    else
    {
      Referential *ref2;
      bool      found = false;
      set<Transformation *> tset = ts->transformationsWith( this );
      set<Transformation *>::const_iterator i, e = tset.end();
      for( i=tset.begin(); !found && i!=e; ++i )
      {
        ref2 = (*i)->source();
        if( ref2 == this )
          ref2 = (*i)->destination();
        try
        {
          Object d = ref2->header().getProperty( "direct_referential" );
          direct = (bool) d->getScalar() ^ !(*i)->isDirect();
          found = true;
          t = *i;
          break;
        }
        catch( ... )
        {
        }
      }
    }
  }
  return direct;
}


Referential* Referential::acPcReferential()
{
  return theAnatomist->centralReferential();
}


Referential* Referential::mniTemplateReferential()
{
  // cout << "Referential::mniTemplateReferential()\n";
  Referential * & ref = mniref();
  if( !ref )
  {
    set<AObject *> so;
    set<AWindow *> sw;
    char           sep = FileUtil::separator();
    string mniref = Paths::findResourceFile( string( "registration" ) + sep
      + "Talairach-MNI_template-SPM.referential" );
    if( !mniref.empty() )
    {
      AssignReferentialCommand  *c
          = new AssignReferentialCommand( 0, so, sw, -1, 0, mniref );
      // exec command even if recursively
      if( theProcessor->idle() )
        theProcessor->execute( c );
      else
        c->execute();
      ref = c->ref();
    }
    if( ref )
    {
      ref->setColor( AimsRGB( 128, 128, 255 ) );
      ref->header().setProperty( "name",
        StandardReferentials::mniTemplateReferential() );
      string acpcmni = Paths::findResourceFile( string( "transformation" )
        + sep + "talairach_TO_spm_template_novoxels.trm" );
      if( !acpcmni.empty() )
      {
        LoadTransformationCommand *c2
            = new LoadTransformationCommand( acpcmni, acPcReferential(), ref );
        theProcessor->execute( c2 );
      }
    }
  }
  return ref;
}


Referential* Referential::giftiTalairachReferential()
{
  // cout << "Referential::giftiTalairachReferential()\n";
  Referential * & ref = giftiTalairachRef();
  if( !ref )
  {
    set<AObject *> so;
    set<AWindow *> sw;
    char           sep = FileUtil::separator();
    carto::UUID uuid;
    uuid.generate();
    AssignReferentialCommand  *c
        = new AssignReferentialCommand( 0, so, sw, -1, 0, "",
                                        uuid.toString() );
    // exec command even if recursively
    if( theProcessor->idle() )
      theProcessor->execute( c );
    else
      c->execute();
    ref = c->ref();
    ref->setColor( AimsRGB( 130, 50, 50 ) );
    ref->header().setProperty( "name", "Talairach" );
    vector<float> trm( 12, 0. ); // transformation inverting all axes
    trm[3] = -1;
    trm[7] = -1;
    trm[11] = -1;
    LoadTransformationCommand *c2
      = new LoadTransformationCommand( trm, acPcReferential(), ref );
    if( theProcessor->idle() )
      theProcessor->execute( c2 );
    else
      c2->execute();
  }
  return ref;
}


Referential* Referential::referentialOfNameOrUUID( const AObject* o,
                                                   const string & refname )
{
  const PythonAObject *po = dynamic_cast<const PythonAObject *>( o );
  if( !po )
    return 0;
  vector<string>  refs;
  Object          transs;
  const GenericObject *go = po->attributed();

  if( !go->getProperty( "referentials", refs )
      || !(transs = go->getProperty( "transformations" ) ) || transs.isNull() )
    return 0;
  Referential *ref = 0;
  unsigned  i, n = refs.size();
  Object  it = transs->objectIterator();

  Referential *mni_ref = Referential::mniTemplateReferential();
  if( refname == StandardReferentials::mniTemplateReferential()
      || (mni_ref && refname == mni_ref->uuid().toString() ) )
  {
    ref = mni_ref;
    // another special case where referential property is set to MNI: then
    // no matter what referentials property contains, just check if there is
    // a transformation between o and MNI, because SPM may not indicate MNI
    // in destination referentials
    string uuid;
    if( go->getProperty( "referential", uuid )
        && uuid == ref->uuid().toString()
        && theAnatomist->getTransformation( o->getReferential(), ref ) )
      return ref;
  }
  for( i=0; i<n; ++i )
  {
    if( !it->isValid() )
      return 0;

    string cur = refs[i];
    if( ref && cur == StandardReferentials::mniTemplateReferential() )
      return ref;
    // TODO: finish this

    it->next();
  }
  return 0;
}


Referential* Referential::referentialOfName( const string & refname )
{
  set<Referential *>    refs = theAnatomist->getReferentials();
  set<Referential *>::const_iterator    i, e = refs.end();
  string                name;

  for( i=refs.begin(); i!=e; ++i )
    if( (*i)->header().getProperty( "name", name )
          && name == refname )
      return *i;
  return 0;
}


bool Referential::hidden() const
{
  try
  {
    Object h = _header->getProperty( "hidden" );
    if( h )
    {
      return (bool) h->getScalar();
    }
  }
  catch( ... )
  {
  }
  return false;
}


void Referential::clearUnusedReferentials()
{
  set<Referential *> refs = theAnatomist->getReferentials();
  set<Referential *>::iterator i, e = refs.end();
  Referential *ref;
  set<Referential *> usedrefs;
  for( i=refs.begin(); i!=e; ++i )
  {
    ref = *i;
    if( ref == Referential::acPcReferential()
        || ref == Referential::mniTemplateReferential()
        || !ref->AnaWin().empty() || !ref->AnaObj().empty() )
      usedrefs.insert( ref );
  }
  // check other referentials
  set<Referential *>::iterator j, k, unused = usedrefs.end();
  ATransformSet *ts = ATransformSet::instance();
  for( i=refs.begin(); i!=e; )
  {
    ref = *i;
    if( usedrefs.find( ref ) == unused )
    {
      // get the connected component this ref is in
      set<Referential *>
          cc = ts->connectedComponent( ref );
      // check whether there are any useful ref in this CC
      for( j=cc.begin(), k=cc.end(); j!=k; ++j )
        if( usedrefs.find( *j ) != unused )
          break;
      if( j == k ) // no useful ref: we can delete the entire CC
      {
        for( j=cc.begin(), k=cc.end(); j!=k; ++j )
          if( *j != ref ) // don't delete ref yet
          {
            refs.erase( *j );
            delete *j;
          }
        ++i; // increment iterator
        delete ref; // then we can delete ref safely
      }
      else
      {
        // ref is linked to a "useful connected component"
        // we must check whether it would break the CC if we remove it
        set<anatomist::Transformation *> trs = ts->transformationsWith( ref );
        set<anatomist::Transformation *>::iterator it, jt, et = trs.end();
        // filter out generated transformations
        for( it=trs.begin(), et=trs.end(); it!=et; )
        {
          if( (*it)->isGenerated() )
          {
            jt = it;
            ++it;
            trs.erase( jt );
          }
          else
            ++it;
        }
        if( trs.size() <= 1 ) // then no link goes through ref
        {
          ++i;
          delete ref;
        }
        else // now trs *must* contain exactly 2 transfos
        {
          //debug
          if( trs.size() != 2 )
            cerr << "BUG in ReferentialWindow::clearUnusedReferentials: more "
                "than 2 connections to a ref inside a connected component"
                << endl;
          int usedcc = 0;
          for( it=trs.begin(), et=trs.end(); it!=et; ++it )
          {
            anatomist::Transformation *tr = *it;
            // get other end
            Referential *ref2 = tr->source();
            if( ref2 == ref )
              ref2 = tr->destination();
            // temporarily disable the transformation
            tr->unregisterTrans();
            // get CC of other end
            set<Referential *> cc2 = ts->connectedComponent( ref2 );
            // if cc2 has useful refs, then ref is useful
            if( cc2.size() != cc.size() )
            {
              for( j=cc2.begin(), k=cc2.end(); j!=k; ++j )
                if( usedrefs.find( *j ) != unused )
                  break;
              if( j == k ) // no useful ref: we can delete the entire CC2
              {
                for( j=cc2.begin(), k=cc2.end(); j!=k; ++j )
                  if( *j != ref ) // don't delete ref yet
                  {
                    refs.erase( *j );
                    delete *j;
                  }
                tr = 0;
              }
              else
              {
                tr->registerTrans();
                ++usedcc;
              }
            }
            else
              ++usedcc;
            if( tr )
              tr->registerTrans();
          }
          if( usedcc <= 1 ) // ref is not useful
          {
            ++i;
            delete ref;
          }
          else
            ++i;
        }
      }
    }
    else
      ++i;
  }
}


bool Referential::mergeReferentials( Referential* r1, Referential *r2 )
{
  // cout << "mergeReferentials " << r1 << ", " << r2 << endl;
  Transformation *tr = theAnatomist->getTransformation( r1, r2 );
  if( tr && !tr->motion().isIdentity() )
    return false;

  if( tr )
    delete tr;
  tr = theAnatomist->getTransformation( r2, r1 );
  if( tr )
    delete tr;
  tr = 0;

  if( r2 == Referential::acPcReferential()
      || r2 == Referential::mniTemplateReferential() )
  {
    // swap r1 and r2
    Referential *r3 = r1;
    r1 = r2;
    r2 = r3;
  }

  // move objects and windows in r2 to r1
  set<AObject *> objs = r2->AnaObj();
  set<AObject *>::iterator io, eo = objs.end();
  for( io=objs.begin(); io!=eo; ++io )
  {
    if( !(*io)->referentialInheritance() )
      (*io)->setReferential( r1 );
  }
  set<AWindow *> wins = r2->AnaWin();
  set<AWindow *>::iterator iw, ew = wins.end();
  for( iw=wins.begin(); iw!=ew; ++iw )
    (*iw)->setReferential( r1 );

  // reassign transformations involving r2 to r1
  ATransformSet *ts = ATransformSet::instance();
  set<Transformation *> trans = ts->transformationsWith( r2 );
  set<Transformation *>::iterator it, et = trans.end();
  Transformation *tr2;
  for( it=trans.begin(); it!=et; ++it )
  {
    tr = *it;
    if( ts->hasTransformation( tr ) )
    {
      tr->unregisterTrans();
      if( !tr->isGenerated() )
      {
        if( tr->source() == r2 )
          tr2 = new Transformation( r1, tr->destination(), false, false );
        else
          tr2 = new Transformation( tr->source(), r1, false, false );
        tr2->motion() = tr->motion();
        // motion history ?
        tr2->registerTrans();
      }
      delete tr;
    }
  }

  delete r2;

  return true;
}


void Referential::mergeIdenticalReferentials()
{
  ATransformSet *ts = ATransformSet::instance();
  bool todo = true;

  while( todo )
  {
    todo = false;
    const set<Transformation *> trans = ts->allTransformations();
    set<Transformation *> done;
    set<Transformation *>::const_iterator it, et = trans.end();
    Transformation *tr;

    for( it=trans.begin(); it!=et; ++it )
    {
      tr = *it;
      if( done.find( tr ) == done.end()
          && ts->hasTransformation( tr )
          && tr->motion().isIdentity()
          && theAnatomist->hasReferential( tr->source() )
          && theAnatomist->hasReferential( tr->destination() ) )
      {
        done.insert( tr );
        mergeReferentials( tr->source(), tr->destination() );
        todo = true; // some new trans may have been created
      }
    }
  }
}


#include <cartobase/object/object_d.h>
INSTANTIATE_GENERIC_OBJECT_TYPE( Referential * )
INSTANTIATE_GENERIC_OBJECT_TYPE( set<Referential *> )
INSTANTIATE_GENERIC_OBJECT_TYPE( vector<Referential *> )
INSTANTIATE_GENERIC_OBJECT_TYPE( list<Referential *> )

