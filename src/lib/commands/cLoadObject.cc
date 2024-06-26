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

#include <anatomist/commands/cLoadObject.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/misc/error.h>
#include <anatomist/processor/Registry.h>
#include <anatomist/processor/Serializer.h>
#include <anatomist/processor/unserializer.h>
#include <anatomist/processor/context.h>
#include <anatomist/processor/event.h>
#include <anatomist/object/oReader.h>
#include <anatomist/window3D/cursor.h>
#include <anatomist/object/loadevent.h>
#include <cartobase/stream/fileutil.h>
#include <cartobase/object/syntax.h>
#include <graph/tree/tree.h>
#include <cartobase/thread/thread.h>
#include <qthread.h>


using namespace anatomist;
using namespace carto;
using namespace std;

class LoadObjectCommandThread : public Thread
{
public:
  LoadObjectCommandThread( LoadObjectCommand *cmd, const std::string & fn, carto::Object op ) : cmd( cmd ), _filename( fn ), _options( op )
  {}
  virtual ~LoadObjectCommandThread() {}

protected:
  virtual void doRun();

private:
  LoadObjectCommand *cmd;
  std::string _filename;
  carto::Object _options;

};


//-----------------------------------------------------------------------------

LoadObjectCommand::LoadObjectCommand( const string & filename, int id, 
				      const string & objname, bool ascursor, 
				      Object options, 
                                      CommandContext* context ) 
  : QObject(), WaitCommand(), SerializingCommand( context ),
    _filename( filename ),
    _id( id ), _objectname( objname ), _ascursor( ascursor ),
    _options( options )
{
}


LoadObjectCommand::~LoadObjectCommand()
{
}


bool LoadObjectCommand::initSyntax()
{
  SyntaxSet	ss;
  Syntax	& s = ss[ "LoadObject" ];
  
  s[ "filename"    ].type = "string";
  s[ "filename"    ].needed =  true;
  s[ "res_pointer" ].type = "int";
  s[ "res_pointer" ].needed =  true;
  s[ "name"        ].type = "string";
  s[ "name"        ].needed =  false;
  s[ "as_cursor"   ] = Semantic( "int", false );
  s[ "options"     ] = Semantic( "dictionary" );
  Registry::instance()->add( "LoadObject", &read, ss );
  return( true );
}


void
LoadObjectCommand::doit()
{
  if( _ascursor )
  {
    // could also use the async mode, but is it worth it ?
    _obj = AObject::load( _filename );
    if( !_obj.empty() )
    {
      string	name = _objectname;
      if( name.empty() )
        name = FileUtil::basename( _filename );
      list<AObject *>::iterator io, eo = _obj.end();
      for( io=_obj.begin(); io!=eo; ++io )
        Cursor::addCursor( name, *io );
    }
  }
  else
  {
    bool async = false;
    if( _options )
    {
      try
      {
        Object x = _options->getProperty( "asynchronous" );
        if( x && (bool) x->getScalar() )
          async = true;
      }
      catch( ... )
      {
      }
    }

    QObject::connect( ObjectReaderNotifier::notifier(),
      SIGNAL( objectLoaded( AObject*,
                            const ObjectReader::PostRegisterList & ,
                            void *, bool ) ),
      this, SLOT( objectLoadDone( AObject*,
                                  const ObjectReader::PostRegisterList &,
                                  void *, bool ) ) );

    if( async )
    {
      // asynchronous read: use a thread.
//       _options->setProperty( "_clientid", (long) (void *) this );
      LoadObjectCommandThread *thread = new LoadObjectCommandThread( this,
        _filename, _options );
      thread->setSuicideSafe( true );
      thread->launch();
      // ObjectReader will emit the signal when done.
    }
    else
    {
      list<AObject *> obj = theAnatomist->loadObject( _filename, _objectname,
                                                     _options );
      if( !obj.empty() )
      {
        AObject *last = *obj.rbegin();
        ObjectReader::PostRegisterList prl;
        list<AObject *>::iterator io, eo = obj.end();
        for( io=obj.begin(); io!=eo; ++io )
          objectLoadDone( *io, prl, this, *io == last );
      }
    }
  }
}


void LoadObjectCommand::doLoad()
{
}


void LoadObjectCommandThread::doRun()
{
  ObjectReader::PostRegisterList subObjectsToRegister;
  list<AObject *> object = ObjectReader::reader()->load( _filename,
                                                  subObjectsToRegister, true,
                                                  _options, cmd );
  list<AObject *>::iterator i, e = object.end();
  for( i=object.begin(); i!=e; ++i )
  {
    bool  visible = true;
    if( _options )
    {
      try
      {
        Object x = _options->getProperty( "hidden" );
        if( x && (bool) x->getScalar() )
          visible = false;
      }
      catch( ... )
      {
      }
    }
    theAnatomist->registerObject( *i, visible );

    // register sub-objects also created (if any)
    ObjectReader::PostRegisterList::const_iterator ipr,
      epr = subObjectsToRegister.end();
    for( ipr=subObjectsToRegister.begin(); ipr!=epr; ++ipr )
      theAnatomist->registerObject( ipr->first, ipr->second );
  }

//   if( !_objectname.empty() )
//     object->setName( theAnatomist->makeObjectName( _objectname ) );
  delete this; // suicide
}


void LoadObjectCommand::objectLoadDone( AObject* obj,
  const ObjectReader::PostRegisterList &, void* clientid, bool last )
{
  /* slot called after loading is done in ObjectReader, either from a
     different thread, or in the main thread */
  // check if we are the right listener
  if( clientid != this )
    return; // it's not for me.
  // cleanup: we must disconnect the slot connected from doit()
  if( last )
    QObject::disconnect( ObjectReaderNotifier::notifier(),
      SIGNAL( objectLoaded( AObject*,
                            const ObjectReader::PostRegisterList &, void*,
                            bool ) ),
      this, SLOT( objectLoadDone( AObject*,
                                const ObjectReader::PostRegisterList &, void*,
                                bool ) ) );
  _obj.push_back( obj );
  if( obj )
  {
    if( context() && context()->unserial )
      context()->unserial->registerPointer( obj, _id, "AObject" );
    // send event
    Object        ex = Object::value( Dictionary() );
    ex->setProperty( "_object", Object::value( obj ) );
    ex->setProperty( "filename", Object::value( _filename ) );
    ex->setProperty( "type",
                      Object::value
                      ( AObject::objectTypeName( obj->type() ) ) );
    OutputEvent   ev( "LoadObject", ex );
    ev.send();
  }
  // emit even if obj is null to notify the load failure.
  emit objectLoaded( obj, _filename );
}


Command* 
LoadObjectCommand::read( const Tree & com, CommandContext* context )
{
  string	filename;
  int		id, ascurs = 0;

  if( !com.getProperty( "filename", filename ) 
      || !com.getProperty( "res_pointer", id ) )
    return( 0 );

  string	objname;
  Object	options;
  com.getProperty( "name", objname );
  com.getProperty( "as_cursor", ascurs );
  com.getProperty( "options", options );

  return( new LoadObjectCommand( filename, id, objname, (bool) ascurs, 
                                 options, context ) );
}


void LoadObjectCommand::write( Tree & com, Serializer* ser ) const
{
  Tree	*t = new Tree( true, name() );

  t->setProperty( "filename", _filename );
//   vector<int> objs;
//   objs.reserve( _obj.size() );
//   list<AObject *>::iterator io, eo = _obj.end();
//   for( io=_obj.begin(); io!=eo; ++io )
//     objs.push_back( ser->serialize( *io ) );
  int id = -1;
  if( !_obj.empty() )
    id = ser->serialize( *_obj.begin() ); // FIXME only 1st object has an ID
  t->setProperty( "res_pointer", id );
  if( !_objectname.empty() )
    t->setProperty( "name", _objectname );
  if( _ascursor )
    t->setProperty( "as_cursor", (int) 1 );
  if( !_options.isNull() )
    t->setProperty( "options", _options );
  com.insert( t );
}

