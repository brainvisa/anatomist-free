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

#include <anatomist/commands/cLoadObjects.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/misc/error.h>
#include <anatomist/processor/Registry.h>
#include <anatomist/processor/Serializer.h>
#include <anatomist/processor/unserializer.h>
#include <anatomist/processor/context.h>
#include <anatomist/processor/event.h>
#include <anatomist/object/oReader.h>
#include <anatomist/object/loadevent.h>
#include <anatomist/surface/Shader.h>
#include <cartobase/stream/fileutil.h>
#include <cartobase/object/syntax.h>
#include <graph/tree/tree.h>
#include <cartobase/thread/thread.h>
#include <cartobase/thread/mutex.h>
#include <cartobase/thread/threadedLoop.h>
#include <cartobase/thread/loopContext.h>

#include <qthread.h>
#include <unistd.h>


using namespace anatomist;
using namespace carto;
using namespace std;


class LoadObjectsCommand::LoadObjectsCommandThread : public Thread
{
public:
  LoadObjectsCommandThread( LoadObjectsCommand *cmd )
    : Thread(), cmd( cmd ), finished( false ), canSuicide( true )
  {}
  virtual ~LoadObjectsCommandThread() {}

protected:
  virtual void doRun();

private:
  friend class LoadObjectsCommand;

  LoadObjectsCommand *cmd;
  bool finished;
  bool canSuicide;
};


struct LoadObjectsCommand::Private
{
  Private( const vector<string> & filename,
           const vector<int> & id,
           const vector<string> & objname,
           Object options,
           CommandContext* context,
           bool threaded )
    : _filename( filename ),
      _id( id ), _objectname( objname ),
      _options( options ),
      _threaded( threaded ), _loaded( 0 ),
      _mutex( Mutex::Recursive ), load_thread( 0 ), cmd_id( 0 ),
      filenames_toload( 0 )
  {
    static int cnum = 0;
    cmd_id = cnum;
    ++cnum;
  }

  vector<string>            _filename;
  vector<int>               _id;
  vector<list<AObject *> >  _obj;
  vector<string>            _objectname;
  Object                    _options;
  bool                      _threaded;
  int                       _loaded;
  Mutex                     _mutex;
  LoadObjectsCommandThread  *load_thread;
  int                       cmd_id;
  int                       filenames_toload;
};


class LoadObjectsCommand::ThreadLoadContext : public LoopContext
{
public:
  ThreadLoadContext( LoadObjectsCommand* cmd )
    : LoopContext(), cmd( cmd )
  {
  }

  virtual void doIt( int startIndex, int countIndex );

  LoadObjectsCommand *cmd;
};


//-----------------------------------------------------------------------------

LoadObjectsCommand::LoadObjectsCommand( const vector<string> & filename,
                                        const vector<int> & id,
                                        const vector<string> & objname,
                                        Object options,
                                        CommandContext* context,
                                       bool threaded )
  : QObject(), WaitCommand(), SerializingCommand( context ),
    d( new Private( filename, id, objname, options, context, threaded ) )
{
}


LoadObjectsCommand::~LoadObjectsCommand()
{
  delete d;
}


bool LoadObjectsCommand::initSyntax()
{
  SyntaxSet	ss;
  Syntax	& s = ss[ "LoadObjects" ];
  
  s[ "filenames"    ].type = "string_vector";
  s[ "filenames"    ].needed =  true;
  s[ "res_pointers" ].type = "int_vector";
  s[ "res_pointers" ].needed =  true;
  s[ "names"        ].type = "string_vector";
  s[ "names"        ].needed =  false;
  s[ "options"      ] = Semantic( "dictionary" );
  s[ "threaded"     ] = Semantic( "int" );
  Registry::instance()->add( "LoadObjects", &read, ss );
  return( true );
}


int LoadObjectsCommand::id() const
{
  return d->cmd_id;
}


void
LoadObjectsCommand::doit()
{
  // cout << "LoadObjectsCommand::doit, " << d->_filename.size() << " obj, threaded: " << d->_threaded << endl;

  d->_id.resize( d->_filename.size(), -1 );
  d->_objectname.resize( d->_filename.size() );
  d->_obj.clear();
  d->_obj.resize( d->_filename.size() );
  d->_loaded = 0;
  d->filenames_toload = d->_filename.size();

  bool async = false;
  if( d->_options )
  {
    try
    {
      Object x = d->_options->getProperty( "asynchronous" );
      if( x && (bool) x->getScalar() )
        async = true;
    }
    catch( ... )
    {
    }
  }

  // make sure sharers are initialized from main thread
  Shader::isSupported();

  QObject::connect( ObjectReaderNotifier::notifier(),
    SIGNAL( objectLoaded( AObject*,
                          const ObjectReader::PostRegisterList & ,
                          void *, bool ) ),
    this, SLOT( objectLoadedCallback( AObject*,
                                      const ObjectReader::PostRegisterList &,
                                      void *, bool ) ) );

  if( async )
  {
    // asynchronous read: use a thread.
    LoadObjectsCommandThread *thread = new LoadObjectsCommandThread( this );
    thread->setSuicideSafe( true );
    d->load_thread = thread;
    thread->launch();
    // ObjectReader will emit the signal when done.
  }
  else
  {
    doLoad();
  }
}


Mutex & LoadObjectsCommand::mutex()
{
  return d->_mutex;
}


void LoadObjectsCommand::doLoad()
{
  // cout << "doLoad\n" << flush;
  if( !d->_threaded )
  {
    vector<string>::const_iterator i, e = d->_filename.end();
    vector<string>::iterator
      ion = d->_objectname.begin(), eon = d->_objectname.end();
    int index = 0;

    for( i=d->_filename.begin(); i!=e; ++i, ++index )
    {
      string objectname;
      if( ion != eon )
      {
        objectname = *ion;
        ++ion;
      }

      ObjectReader::PostRegisterList prl;
      list<AObject *> obj
        = ObjectReader::reader()->load( *i, prl, true, d->_options, this );

      if( !obj.empty() )
      {
        d->_mutex.lock();
        d->_obj[index] = obj;
        d->_loaded += obj.size();
        d->_mutex.unlock();
        AObject *last = *obj.rbegin();
        list<AObject *>::iterator io, eo = obj.end();

        for( io=obj.begin(); io!=eo; ++io )
          objectLoadDone( *io, prl, index );
      }
    }
    emit loadFinished( id() );
  }
  else
  {
    // cout << "doLoad threaded...\n" << flush;
    ThreadLoadContext lc( this );
    ThreadedLoop tl( &lc, 0, d->_filename.size() );
    tl.launch();
  }
}


void LoadObjectsCommand::LoadObjectsCommandThread::doRun()
{
  cmd->doLoad();
  LoadObjectsCommand *c = cmd;
  struct timespec ts;
  cmd->mutex().lock();
  finished = true;

  while( !canSuicide )
  {
    cmd->mutex().unlock();
    ts.tv_sec = 0;
    ts.tv_nsec = 20000;  // 20 ms
    nanosleep( &ts, &ts );
    cmd->mutex().lock();
  }

  cmd->d->load_thread = 0;
  delete this; // suicide
  cmd->mutex().unlock();
}


void LoadObjectsCommand::ThreadLoadContext::doIt( int startIndex, int count )
{
  // cout << "ThreadLoadContext::doIt " << startIndex << ", " << count << endl << flush;
  int index, ie = startIndex + count;
  for( index=startIndex; index<ie; ++index )
  {
    ObjectReader::PostRegisterList subObjectsToRegister;
    Object options = Object::value( Dictionary() );
    cmd->mutex().lock();
    string filename = cmd->d->_filename[index];
    // cout << "loading: " << index << ": " << filename << endl << flush;
    if( cmd->d->_options )
      options->copyProperties( cmd->d->_options );
    cmd->mutex().unlock();
    options->setProperty( "asynchronous", 0 );
    list<AObject *> object
      = ObjectReader::reader()->load( filename, subObjectsToRegister,
                                      true, options, cmd );

    // record it in the list now.
    cmd->mutex().lock();
    cmd->d->_loaded += object.size();
    --cmd->d->filenames_toload;
    bool last = ( cmd->d->filenames_toload == 0 );
    cmd->d->_obj[index] = object;
    cmd->mutex().unlock();
    pair<LoadObjectsCommand *, int> id( cmd, index );
    list<AObject *>::iterator io, eo = object.end();
    for( io=object.begin(); io!=eo; ++io )
      cmd->objectLoadDone( *io, subObjectsToRegister, index );
    if( last )
      emit cmd->loadFinished( cmd->id() );
  }
}


bool LoadObjectsCommand::loading() const
{
  bool loading = false;
  d->_mutex.lock();
  LoadObjectsCommandThread *thread = d->load_thread;
  if( thread )
    loading = ( !thread->finished );
  d->_mutex.unlock();

  return loading;
}


void LoadObjectsCommand::wait()
{
  d->_mutex.lock();
  LoadObjectsCommandThread *thread = d->load_thread;
  if( thread && !thread->finished )
    thread->canSuicide = false;  // prevent it from suiciding right now
  d->_mutex.unlock();
  if( thread )
  {
    struct timespec ts;
    d->_mutex.lock();
    while( !thread->finished )
    {
      d->_mutex.unlock();
      ts.tv_sec = 0;
      ts.tv_nsec = 20000;  // 20 ms
      nanosleep( &ts, &ts );
      d->_mutex.lock();
    }
    thread->canSuicide = true;
    d->_mutex.unlock();
    return;
  }
}


void LoadObjectsCommand::objectLoadedCallback( AObject* obj,
  const ObjectReader::PostRegisterList &, void* clientid, bool )
{
  // cout << "objectLoadedCallback clientid: " << clientid << endl << flush;

  if( clientid != this )
    return;  // it's not for me

  // cleanup: we must disconnect the slot connected from doit()
  d->_mutex.lock();
  --d->_loaded;
  bool last = (d->_loaded == 0 );
  d->_mutex.unlock();
  if( last )
//   { cout << "last - disconnect objectLoadedCallback signal\n" << flush;
    QObject::disconnect( ObjectReaderNotifier::notifier(),
      SIGNAL( objectLoaded( AObject*,
                            const ObjectReader::PostRegisterList &, void*,
                            bool ) ),
      this, SLOT( objectLoadedCallback(
        AObject*, const ObjectReader::PostRegisterList &, void*, bool ) ) );
//   }
}


void LoadObjectsCommand::objectLoadDone( AObject* obj,
  const ObjectReader::PostRegisterList &, int index )
{
  /* slot called after loading is done in ObjectReader, either from a
     different thread, or in the main thread */
//   cout << "objectLoadDone index: " << index << endl << flush;
  if( Thread::currentIsMainThread() )
  {
    bool visible = true;
    if( d->_options )
      try
      {
        visible = !bool( d->_options->getProperty( "hidden" )->getScalar() );
      }
      catch( ... )
      {
      }
    theAnatomist->registerObject( obj, visible );
  }

  if( obj )
  {
    if( context() && context()->unserial )
      context()->unserial->registerPointer( obj, d->_id[index], "AObject" );
    // send event
    Object        ex = Object::value( Dictionary() );
    ex->setProperty( "_object", Object::value( obj ) );
    ex->setProperty( "filename", Object::value( d->_filename[index] ) );
    ex->setProperty( "type",
                      Object::value
                      ( AObject::objectTypeName( obj->type() ) ) );
    OutputEvent   ev( "LoadObject", ex );
    ev.send();
  }

  // emit even if obj is null to notify the load failure.
  emit objectLoaded( obj, QString( d->_filename[index].c_str() ) );
}


Command* 
LoadObjectsCommand::read( const Tree & com, CommandContext* context )
{
  vector<string> filename;
  vector<int>    id;
  int            threaded = 1;


  if( !com.getProperty( "filenames", filename )
      || !com.getProperty( "res_pointers", id ) )
    return( 0 );

  vector<string>  objname;
  Object          options;
  com.getProperty( "names", objname );
  com.getProperty( "options", options );
  com.getProperty( "threaded", threaded );

  return( new LoadObjectsCommand( filename, id, objname,
                                  options, context, bool( threaded ) ) );
}


void LoadObjectsCommand::write( Tree & com, Serializer* ser ) const
{
  Tree	*t = new Tree( true, name() );

  t->setProperty( "filenames", d->_filename );
  vector<int> id( d->_filename.size(), -1 );
  if( !d->_obj.empty() )
  {
    vector<list<AObject *> >::const_iterator io, eo = d->_obj.end();
    int i = 0;
    for( io=d->_obj.begin(); io!=eo; ++io, ++i )
      if( !io->empty() )
        id[i] = ser->serialize( *io->begin() );
      // FIXME only 1st object has an ID
  }
  t->setProperty( "res_pointers", id );
  if( !d->_objectname.empty() )
    t->setProperty( "names", d->_objectname );
  if( !d->_options.isNull() )
    t->setProperty( "options", d->_options );
  if( !d->_threaded )
    t->setProperty( "threaded", 0 );
  com.insert( t );
}


vector<AObject *> LoadObjectsCommand::loadedObjects()
{
  vector<AObject *> lo;
  lo.reserve( d->_obj.size() );
  vector<list<AObject *> >::iterator iol, eol = d->_obj.end();
  list<AObject *>::iterator io, eo;

  for( iol=d->_obj.begin(); iol!=eol; ++iol )
    for( io=iol->begin(), eo=iol->end(); io!=eo; ++io )
      lo.push_back( *io );

  // cout << "loadedObjects: " << d->_obj.size() << " slots, obj: " << lo.size() << endl;
  return lo;
}

