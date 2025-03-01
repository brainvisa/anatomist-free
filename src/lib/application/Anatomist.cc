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


#ifdef _WIN32
/* Windows OpenGL uses strange macros WINGDIAPI and APIENTRY, 
   they are undefined and gl.h makes lots of errors, unless qgl.h is 
   included first (qgl seems to correctly define these macros)
*/
#include <cstdlib>
#include <qgl.h>
#endif

#include <anatomist/application/Anatomist.h>

#include <anatomist/control/wControl.h>
#include <anatomist/control/wcontrolevents.h>
#include <anatomist/commands/cCreateControlWindow.h>
#include <anatomist/commands/cLoadObject.h>
#include <anatomist/commands/cServer.h>
#include <anatomist/object/Object.h>
#include <anatomist/object/oReader.h>
#include <anatomist/processor/pipeReader.h>
#include <anatomist/processor/server.h>
#include <anatomist/processor/event.h>
#include <anatomist/processor/Writer.h>
#include <anatomist/window/winFactory.h>
#include <anatomist/mobject/ObjectList.h>
#include <anatomist/misc/error.h>
#include <anatomist/mobject/MObject.h>
#include <anatomist/color/paletteList.h>
#include <anatomist/reference/wReferential.h>
#include <anatomist/reference/Referential.h>
#include <anatomist/reference/transfSet.h>
#include <anatomist/reference/Transformation.h>
#include <anatomist/application/globalConfig.h>
#include <anatomist/application/settings.h>
#include <anatomist/control/controlConfig.h>
#include <anatomist/control/whatsNew.h>
#include <anatomist/stdmod/stdmod.h>
#include <anatomist/window/Window.h>
#include <anatomist/window3D/cursor.h>
#include <anatomist/window/qwinblock.h>
#include <anatomist/application/syntax.h>
#include <anatomist/application/settings.h>
#include <anatomist/application/listDir.h>
#include <anatomist/window/glwidgetmanager.h>
#include <anatomist/observer/observcreat.h>
#include <anatomist/object/objectmenuCallbacks.h>
#include <anatomist/fusion/fusionFactory.h>
#include <anatomist/selection/selectFactory.h>
#include <anatomist/object/objectutils.h>
#include "anatomistprivate.h"
#include <aims/def/path.h>
#include <cartobase/plugin/plugin.h>
#include <cartobase/stream/directory.h>
#include <cartobase/stream/fileutil.h>
#include <anatomist/config/version.h>
#include <cartobase/config/paths.h>
#include <cartobase/smart/rcptrtrick.h>
#include <cartobase/thread/mutex.h>
#include <cartobase/exception/file.h>
#include <cartobase/thread/thread.h>
#include <string.h>
#include <algorithm>
#include <stdio.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <qapplication.h>
#include <qpixmap.h>
#include <qtranslator.h>

// Includes for output/error redirection
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

//#define ANA_DEBUG
#ifdef ANA_DEBUG
#include <typeinfo>
#include <cartobase/smart/rcptrtrick.h>
#endif

using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;


//--- Variables

Anatomist* theAnatomist = 0;

namespace anatomist
{

string Anatomist::versionString()
{
  static string	ver;
  if( ver.empty() )
    {
      ostringstream	s;
      s << ANATOMIST_VERSION_MAJOR << '.' << ANATOMIST_VERSION_MINOR << '.' 
        << ANATOMIST_VERSION_TINY;
      ver = s.str();
    }
  return ver;
}




string Anatomist::libraryVersionString()
{
  static string	ver;
  if( ver.empty() )
    {
      ostringstream	s;
      s << ANATOMIST_VERSION_MAJOR << '.' << ANATOMIST_VERSION_MINOR;
      ver = s.str();
    }
  return ver;
}


  class StaticInitializers
  {
    public:
      static bool init();
      static void cleanup();
  };
}


namespace
{

  /* filter events to catch windows open events and set a default icon on 
     such windows */
  class EventFilter : public QObject
  {
  public:
    virtual ~EventFilter();

  protected:
    virtual bool eventFilter( QObject *watched, QEvent *e );
  };

  
  EventFilter::~EventFilter()
  {
  }


  bool EventFilter::eventFilter( QObject *watched, QEvent *e )
  {
    if( e->type() == QEvent::Show && watched->parent() == 0 
        && watched->inherits( "QWidget" ) )
    {
      QWidget	*w = (QWidget *) watched;
      if( w->windowIcon().isNull() )
      {
        static QPixmap icn( Settings::findResourceFile(
          "icons/icon.xpm" ).c_str() );
        if( !icn.isNull() )
          w->setWindowIcon( icn );
      }
    }
    return false;
  }

}


namespace anatomist
{

//	Private data structure

struct Anatomist::Anatomist_privateData
{
  Anatomist_privateData();
  ~Anatomist_privateData();

  /* the map and double pointers is for finding windows that could be
  already deleted, so we cannot take a smart pointer on them */
  map<const AWindow *, carto::shared_ptr<AWindow> > anaWin;
  /* the map and double pointers is for finding objects that could be
    already deleted, so we cannot take a smart pointer on them */
  map<const AObject *, carto::shared_ptr<AObject> > anaObj;
  set<Referential*>		anaRef;
  map<std::string, AObject*>	anaNameObj;
  CommandWriter 			*historyW;
  PaletteList				*paletteList;
  static ReferentialWindow		*referentialWindow;
  bool			initialized;
  bool			cursorChanged;
  GlobalConfiguration	*config;
  string		objectsFileFilter;
  string		allObjectsFilter;
  string		specificFilters;
  string		allFilesFilter;
  Referential		*centralRef;
  int			userLevel;
  vector<float>		lastpos;
  mutable Referential	*lastref;
  bool                  destroying;
  Mutex                 objectsLock;
  // This QWidget is the parent of all windows in Anatomist. 
  // This enable to close all windows of Anatomist even when Anatomist is embedded in another Qt application, like Axon for example.
  QWidget* qWidgetAncestor;
  int argc;
  const char **argv;
  bool exitOnQuit;
  QWidget* defaultWindowsBlock;
};


ReferentialWindow* Anatomist::Anatomist_privateData::referentialWindow = 0;


Anatomist::Anatomist_privateData::Anatomist_privateData()
  : historyW( new CommandWriter ), paletteList( 0 ),
    initialized( false ), cursorChanged( false ), config( 0 ), centralRef( 0 ),
    userLevel( 0 ), lastpos( 4, 0. ),
    lastref( 0 ), destroying( false ), objectsLock( Mutex::Recursive ),
    qWidgetAncestor( 0 ), argc( 0 ), argv( 0 ), exitOnQuit( false ),
    defaultWindowsBlock( 0 )
{
}


Anatomist::Anatomist_privateData::~Anatomist_privateData()
{
  delete historyW;
  delete centralRef;
  centralRef = 0;
  if( cursorChanged )
    {
      QApplication::restoreOverrideCursor();
    }
  delete config;
  delete paletteList;
  delete qWidgetAncestor;
}


//	Constructor

Anatomist::Anatomist( int argc, const char **argv, 
		      const std::string &documentation ) 
  : AimsApplication( argc, argv, documentation ), 
    _privData( new Anatomist::Anatomist_privateData )
{
  _privData->argc = argc;
  _privData->argv = argv;
  // parse command line options to see standartd output redirection is 
  // required. This is done first because plugins loaders display messages.
  CommandLineArguments::const_iterator arg = cla.begin();
  string coutName, cerrName;
  ++arg;
  while( arg != cla.end() ) {
    if ( *arg == "--cout" ) {
      ++arg;
      if ( arg != cla.end() ) {
        coutName = *arg++;
      }
    } else if ( *arg == "--cerr" ) {
      ++arg;
      if ( arg != cla.end() ) {
        cerrName = *arg++;
      }
    } else {
      ++arg;
    }
  }

  if ( ! coutName.empty() ) {
#ifdef _WIN32
    int out = open( coutName.c_str(), O_CREAT | O_TRUNC | O_WRONLY );
#else
    int out = open( coutName.c_str(), O_CREAT | O_TRUNC | O_WRONLY, 
                    S_IRUSR | S_IWUSR |S_IRGRP );
#endif
    if ( out == -1 ) {
      cerr << "Failed redirection of output to " << coutName << endl;
    } else {
      dup2( out, 1 );
      close( out );
    }
  }

  if ( ! cerrName.empty() ) {
    if ( cerrName != coutName ) {
#ifdef _WIN32
      int out = open( cerrName.c_str(), O_CREAT | O_TRUNC | O_WRONLY );
#else
      int out = open( cerrName.c_str(), O_CREAT | O_TRUNC | O_WRONLY, 
                      S_IRUSR | S_IWUSR |S_IRGRP );
#endif
      if ( out == -1 ) {
        cerr << "Failed redirection of output to " << cerrName << endl;
      } else {
        dup2( out, 2 );
        close( out );
      }
    } else {
      dup2( 1, 2 );
    }
  }

  cout << "Starting Anatomist....." << endl;

  assert( theProcessor );
  assert( theAnatomist == 0 );
  theAnatomist = this;

  _privData->config = new GlobalConfiguration;

  _privData->paletteList = new PaletteList;
  qApp->setAttribute(Qt::AA_DontShowIconsInMenus, false);
  //   qApp->setQuitOnLastWindowClosed(true);// default is already true
//   cout<<"quitOnLastWindowClosed() (when true the qApp will be quitted) : "<<QApplication::quitOnLastWindowClosed()<<endl;

}


//	Destructor

Anatomist::~Anatomist()
{
  // send event
  Object	ex( (GenericObject *) 
		    new ValueObject<Dictionary> );
  OutputEvent	ev( "Exit", ex );
  ev.send();

  cout << "Exiting Anatomist --- Goodbye.\n";

  _privData->destroying = true;

  while( !_privData->anaWin.empty() )
    delete _privData->anaWin.begin()->second.get();

  ::anatomist::Cursor::cleanStatic();

  // cout << "deleting objects... " << _privData->anaObj.size() << endl;
  set<AObject *> objs;
  map<const AObject *, carto::shared_ptr<AObject> >::iterator
    io, eo = _privData->anaObj.end();

  for( io=_privData->anaObj.begin(); io!=eo; ++io )
    objs.insert( io->second.get() );

  objs = destroyObjects( objs );

  _privData->anaObj.clear();

  set<AObject *>::iterator iso, eso = objs.end();
  AObject	*obj;

  for( iso=objs.begin(); iso!=eso; ++iso )
  {
    obj = *iso;
    cerr << "object " << obj->name()
          << " refuses to die peacefully - killing it (better crash than "
          << "loop endlessly)." << endl;
#ifdef ANA_DEBUG
    cerr << "  + ref counter: " << rc_ptr_trick::refCount( *obj )
          << "\n  + weak counter: " << rc_ptr_trick::weakCount( *obj )
          << ", type:" << obj->objectFullTypeName() << endl;
#endif
    delete obj;
  }
  // cout << "objects deleted.\n";

  // cleanup some global variables
  if( getControlWindow() && QApplication::instance() )
  {
    // avoid ctrl win to quit again
    getControlWindow()->enableClose( false );
    delete getControlWindow();
  }
  delete FusionFactory::factory();
  AObject::cleanStatic();
  delete SelectFactory::factory();

  ObjectReader::cleanup();

  StaticInitializers::cleanup();

  ModuleManager::shutdown();

  delete _privData;
  _privData = 0;
  theAnatomist = 0;
}

}


namespace
{

  struct Args
  {
    vector<string>	pipenames;
    bool		batch;
    int		        userlevel;
    vector<string>	files;
    int			port;
    string 		userprof;
  };


  void parseArgs( Anatomist & a, Args & args )
  {
  string	dummy;

  args.batch = false;
  args.userlevel = -1;
  args.port = 0;

  a.addOptionSeries( args.files, "-i", "loads these files (conataining either "
                     "Anatomist/AIMS objects, or .ana commands files)" );
  a.addOptionSeries( args.pipenames, "-p", "open named pipe to read commands " 
                     "from (differs from normal .ana files loading in that " 
                     "the pipe is not closed once read, the pipe is reopened " 
                     "once flushed - using -p for a regular file results in "
                     "infinite rereading of the same commands file)" );
  a.addOption( args.batch, "-b", "batch mode: no control window (for remote " 
               "control only, normally used with -p or a .ana input)", true );
  a.addOption( args.port, "-s", "server mode: anatomist will listen on the " 
               "given TCP port for command stream connections", true );
  a.addOption( Settings::userProfile(), "-u", "user profile: all personal " 
               "data will use the directory " 
               "<home_dir>/.anatomist-<userprofile> rather than the default " 
               "<home_dir>/.anatomist", true );
  a.addOption( args.userlevel, "--userLevel",
               "sets the user level: 0: basic, 1: advanced, 2: expert, >=3: "
               "debugger (enables unstable features which may make Anatomist "
               "badly crash)", true );
  a.addOption( dummy, "--cout", "redirect outpout messages to a file", true );
  a.addOption( dummy, "--cerr", "redirect error messages to a file", true );

  a.alias( "--userprofile", "-u" );
  a.alias( "--pipe", "-p" );
  a.alias( "--server", "-s" );
  a.alias( "--batch", "-b" );
  a.alias( "--input", "-i" );

  a.AimsApplication::initialize();

  if( !args.userprof.empty() )
    Settings::setUserProfile( args.userprof );
  }

}


namespace anatomist
{

//	Inherited functions

void Anatomist::initialize()
{
  // cout << "Anatomist::initialize\n" << std::flush;
  assert(_privData->anaWin.empty());
  assert(_privData->anaObj.empty());
  assert(_privData->anaRef.empty());

  char		sep = FileUtil::separator();
  string	ssep;
  ssep += sep;

  //PluginLoader::load();

  Args	args;
  ::parseArgs( *this, args );

  if( QApplication::instance()
    && !dynamic_cast<QApplication *>( QApplication::instance() ) )
  {
    /* there is a QCoreApplication, which is not a QApplication: delete it
       and recreate a full QApplication. */
    delete QApplication::instance();
  }
  if( !QApplication::instance() )
  {
    // needed in WebEngine helps and other extensions in python
    QCoreApplication::setAttribute( Qt::AA_ShareOpenGLContexts );

    new QApplication( _privData->argc,
                      const_cast<char **>( _privData->argv ) );
  }
  _privData->qWidgetAncestor = new QWidget;

  // palettes
  /* palettes should be read before the config because they might be used by 
     config options, like cursors */

  list<string> palettespaths = Paths::findResourceFiles( "rgb", "anatomist",
    libraryVersionString() );
  list<string>::const_reverse_iterator ipl, epl = palettespaths.rend();
  for( ipl=palettespaths.rbegin(); ipl!=epl; ++ipl )
    _privData->paletteList->load( *ipl );
  // cout << _privData->paletteList->size() << " palettes loaded\n";

  //	config
  _privData->config->registerLocalConfiguration( new ControlConfiguration );
  _privData->config->load();

  // Fichier history
  string history_name = Settings::localPath();
  // in case ~/.anatomist is a dead link: read the destination of the link and
  // create the directory there.
  char linkname[1024];
  ssize_t ln = readlink( history_name.c_str(), linkname, 1024 );
  if( ln > 0 && ln < 1024 ) {
    linkname[ln] = '\0';
    history_name = linkname;
  }

  Directory	dir( history_name );
  bool write_hist = true;
  try
  {
    dir.mkdir();
  }
  catch( file_error & e )
  {
    // do nothing...
    write_hist = false;
  }
  history_name += ssep + "history.ana";
  // keep backup
  if( write_hist )
  {
    rename( history_name.c_str(), (history_name + "~").c_str() );
    _privData->historyW->open( history_name.c_str() );
  }

  if ( !*_privData->historyW )
  {
    AWarning("Unable to open history file. No output will be written");
  }
  else
  {
    theProcessor->addObserver( _privData->historyW );
  }

  //	Language

  string lang = language();
  string langpath = Settings::findResourceFile( string( "po" ) + sep + lang );
  list<string>	translations 
    = anatomist::listDirectory( langpath, "*.qm", QDir::Name, 
                                QDir::Files | QDir::Readable );

  if( translations.empty() && lang != "en" )
  {
    langpath = Settings::findResourceFile( string( "po" ) + sep + "en" );
    translations = anatomist::listDirectory( langpath, "*.qm", QDir::Name,
                                              QDir::Files | QDir::Readable );
  }

  list<string>::iterator	it, et = translations.end();
  for( it=translations.begin(); it!=et; ++it )
  {
    QTranslator	*tr = new QTranslator( qApp );
    tr->setObjectName( "Translator" );
    if( tr->load( it->c_str(), langpath.c_str() ) )
      qApp->installTranslator( tr );
    else
    {
      cerr << "warning: translation file not found\n";
      cerr << "path : " << langpath << "\nfile: " << *it << "\n";
      delete tr;
    }
  }

  string	ver;
  Object	over;
  try
  {
    over = _privData->config->getProperty( "anatomist_version" );
    if( !over.isNone() )
      ver = over->getString();
  }
  catch( ... )
  {
  }
  if( ver != versionString() )
  {
    string::size_type	p = ver.find( '.' );
    bool	wn = true;
    if( p != string::npos )
    {
      int		majv = 0, minv = 0, majo = 0, mino = 0;
      sscanf( versionString().c_str(), "%d.%d", &majv, &minv );
      sscanf( ver.c_str(), "%d.%d", &majo, &mino );
      if( majv < majo || ( majv == majo && minv < mino ) )
        wn = false;
    }

    if( wn )
    {
      WhatsNew	wn;
      wn.exec();
      _privData->config->save();
    }
  }
  // Init Menu objects (objects replacing optionTree menu)
  initMenuObjects();

  SyntaxRepository::scanExportedSyntaxes();
  SyntaxRepository::scanSyntaxesInDir( SyntaxRepository::internalSyntax(), 
                                       Path::singleton().syntax()
                                       + sep + "internal" );
  SyntaxRepository::scanSyntaxesInDir
    ( SyntaxRepository::internalSyntax(), Settings::localPath() 
      + sep + "syntax" + sep + "internal" );

  StdModule* stdmod = new StdModule;
  stdmod->init();

  _privData->centralRef = new Referential;	// create central referential
  string talref = carto::Paths::findResourceFile( string( "registration" )
    + sep + "Talairach-AC_PC-Anatomist.referential" );
  if( !talref.empty() )
    _privData->centralRef->load( talref );
  Referential::mniTemplateReferential(); // create SPM/MNI referential

  list<string> modfiles = Paths::findResourceFiles( string( "plugins" ) + sep
    + "anatomist.plugins", "anatomist", theAnatomist->libraryVersionString() );
  list<string>::iterator imf, emf = modfiles.end();
  for( imf=modfiles.begin(); imf!=emf; ++imf )
  {
    PluginLoader::pluginFiles().push_back
      ( PluginLoader::PluginFile( *imf, versionString() ) );
  }
  PluginLoader::load();	// load plugins in new path
  // file filters
  updateFileDialogObjectsFilter();

  // event filter to add windows icons
  qApp->installEventFilter( new EventFilter );

  // process args, load files etc.

  if( !args.batch )
  {
    Command* command = new CreateControlWindowCommand;
    theProcessor->execute(command);
    qApp->connect( qApp, SIGNAL( aboutToQuit() ),
                   getControlWindow(), SLOT( quit() ) );
  }

  if( args.port != 0 )
  {
    ServerCommand	*serv = new ServerCommand( args.port );
    theProcessor->execute( serv );
  }

  if( args.userlevel >= 0 )
    _privData->userLevel = args.userlevel;

  unsigned	i, n = args.pipenames.size();
  for( i=0; i<n; ++i )
  {
    cout << "pipe: " << args.pipenames[i] << endl;
    new APipeReader( args.pipenames[i], false );
  }

  for( i=0, n=args.files.size(); i<n; ++i )
  {
    cout << "file: " << args.files[i] << endl;
    if( args.files[i].length() >= 4
        && args.files[i].substr( args.files[i].length() - 4, 4 ) ==".ana" )
    {	// scenario script
      new APipeReader( args.files[i] );
    }
    else
    {	// standard object
      Command	*c = new LoadObjectCommand( args.files[i] );
      theProcessor->execute( c );
    }
  }

  _privData->initialized = true;
}


void Anatomist::updateFileDialogObjectsFilter()
{
  _privData->allObjectsFilter
    = string( ControlWindow::tr( "All Anatomist objects" ).toStdString() )
    + " (" + ObjectReader::allSupportedFileExtensions() + ");;";
  set<string> meshtypes;
  meshtypes.insert( "Mesh" );
  meshtypes.insert( "Mesh4" );
  meshtypes.insert( "Segments" );
  _privData->specificFilters
    = string( ControlWindow::tr( "Volumes" ).toStdString() ) + " ("
    + ObjectReader::supportedFileExtensions( "CartoVolume" ) + ");;"
    + ControlWindow::tr( "DICOM" ).toStdString() + " (*);;"
    + ControlWindow::tr( "Surfacic meshes" ).toStdString() + " ("
    + ObjectReader::supportedFileExtensions( meshtypes ) + ");;"
    + ControlWindow::tr( "Graphs/ROIs sets" ).toStdString() + " ("
    + ObjectReader::supportedFileExtensions( "Graph" ) + ");;"
    + ControlWindow::tr( "Voxels lists" ).toStdString() + " ("
    + ObjectReader::supportedFileExtensions( "Bucket" ) + ");;"
    + ControlWindow::tr( "Nomenclatures" ).toStdString() + " ("
    + ObjectReader::supportedFileExtensions( "Hierarchy" ) + ");;"
    + ControlWindow::tr( "Textures" ).toStdString() + " ("
    + ObjectReader::supportedFileExtensions( "Texture" ) + ");;"
    + ControlWindow::tr( "Sparse matrices" ).toStdString() + " ("
    + ObjectReader::supportedFileExtensions( "SparseMatrix" ) + ");;"
    + ControlWindow::tr( "Other object files" ).toStdString() + " ("
    + ObjectReader::anatomistSupportedFileExtensions() + ");;"
    + ControlWindow::tr( "Scripts" ).toStdString() + " (*.ana);;";

  _privData->allFilesFilter = string( ";;" )
    + ControlWindow::tr( "All files" ).toStdString() + " (*)";
  _privData->objectsFileFilter = _privData->allObjectsFilter
    + _privData->specificFilters + _privData->allFilesFilter;
}


string Anatomist::language() const
{
  string lang = "en";
  if( _privData->config->getProperty( "language", lang ) )
    return lang;

  // try to use system language settings
  char	*l = getenv( "LANG" );
  if( l )
    {
      lang = l;
      // look for string like "fr_FR@euro", keep only "fr"
      string::size_type	pos = lang.find( '_' );
      if( pos != string::npos )
	lang.erase( pos, lang.length() - pos );
    }
  return lang;
}


//	Registration

void Anatomist::registerWindow( AWindow* win )
{
  _privData->anaWin[ win ] = weak_shared_ptr<AWindow>( win );
}


void Anatomist::unregisterWindow( AWindow* win )
{
  using carto::shared_ptr;

  if( getControlWindow() != 0)
    getControlWindow()->unregisterWindow( win );

  map<const AWindow *, shared_ptr<AWindow> >::iterator i
      = _privData->anaWin.find( win );
  if( i != _privData->anaWin.end() )
    _privData->anaWin.erase( i );
}


void Anatomist::releaseWindow( AWindow* win )
{
  using carto::shared_ptr;

  map<const AWindow *, shared_ptr<AWindow> >::iterator
      i = _privData->anaWin.find( win );
  // change the smart pointer type to Weak
  if( i != _privData->anaWin.end() )
    i->second.reset( shared_ptr<AWindow>::Weak, win );
}


void Anatomist::takeWindowRef( AWindow* win )
{
  using carto::shared_ptr;

  map<const AWindow *, shared_ptr<AWindow> >::iterator
      i = _privData->anaWin.find( win );
  // change the smart pointer type to Weak
  if( i != _privData->anaWin.end() )
  {
    // reset() doesn't work. I don't know why.
//     i->second.reset( shared_ptr<AWindow>::WeakShared, win );
    _privData->anaWin.erase( i );
    _privData->anaWin[ win ] = weak_shared_ptr<AWindow>( win );
  }
}


void Anatomist::registerObject( AObject* obj, int inctrl )
{
  using carto::shared_ptr;

#ifdef ANA_DEBUG
  cout << "Anatomist::registerObject " << obj << " (" 
       << typeid( *obj ).name() << ") - " << obj->name() << endl;
#endif
  if( _privData->destroying )
    return;
  lockObjects( true );
  _privData->anaObj[ obj ]
      = shared_ptr<AObject>( shared_ptr<AObject>::WeakShared, obj );
  if( inctrl )
    mapObject( obj );
  lockObjects( false );
  // call the observable notifier
  ObservableCreatedNotifier::notifyCreated( obj );
}


void Anatomist::unregisterObject( AObject* obj )
{
  using carto::shared_ptr;

#ifdef ANA_DEBUG
  cout << "Anatomist::unregisterObject " << obj << " ("
      << typeid( *obj ).name() << ") - " << obj->name() << endl;
#endif

  lockObjects( true );

  unmapObject( obj );

  map<const AObject *, shared_ptr<AObject> >::iterator i
      = _privData->anaObj.find( obj );
  if( i != _privData->anaObj.end() )
    _privData->anaObj.erase( i );

  lockObjects( false );

#ifdef ANA_DEBUG
  else
    cerr << "  - nothing to unregister" << endl;
#endif 
}


void Anatomist::releaseObject( AObject* obj )
{
  using carto::shared_ptr;

  lockObjects( true );
  map<const AObject *, shared_ptr<AObject> >::iterator
      i = _privData->anaObj.find( obj );
  // change the smart pointer type to Weak
  if( i != _privData->anaObj.end() )
    i->second.reset( shared_ptr<AObject>::Weak, obj );
  lockObjects( false );
}


void Anatomist::takeObjectRef( AObject* obj )
{
  using carto::shared_ptr;

  lockObjects( true );
  map<const AObject *, shared_ptr<AObject> >::iterator
      i = _privData->anaObj.find( obj );
  // change the smart pointer type to Weak
  if( i != _privData->anaObj.end() )
  {
    // reset() doesn't work. I don't know why.
//     i->second.reset( shared_ptr<AObject>::WeakShared, obj );
    _privData->anaObj.erase( i );
    _privData->anaObj[ obj ]
      = shared_ptr<AObject>( shared_ptr<AObject>::WeakShared, obj );
  }
  lockObjects( false );
}


void Anatomist::registerReferential( Referential* ref )
{
  lockObjects( true );  // could be a different mutex
  _privData->anaRef.insert( ref );
  lockObjects( false );
  if( _privData->referentialWindow != 0 && Thread::currentIsMainThread() )
    _privData->referentialWindow->refresh();
  // else use postEvent
}


void Anatomist::unregisterReferential( Referential* ref )
{
  lockObjects( true );  // could be a different mutex
  set<Referential*>::iterator i = _privData->anaRef.find( ref );
  if( i != _privData->anaRef.end() )
    {
      ATransformSet::instance()->deleteTransformationsWith( ref );
      _privData->anaRef.erase( i );
      lockObjects( false );
      if( _privData->referentialWindow != 0 && Thread::currentIsMainThread() )
        _privData->referentialWindow->refresh();
      // else use postEvent
    }
  else
  {
    cerr << "Anatomist::unregisterReferential : ref not found\n";
    lockObjects( false );
  }
}


void Anatomist::mapObject( AObject* obj )
{
  if( getControlWindow() != 0 && !_privData->destroying )
  {
    if( Thread::currentIsMainThread() )
      getControlWindow()->registerObject( obj );
    else
      qApp->postEvent( getControlWindow(), new MapObjectEvent( obj ) );
  }
}


void Anatomist::unmapObject( AObject* obj )
{
  if( getControlWindow() != 0 && !_privData->destroying )
  {
    if( Thread::currentIsMainThread() )
      getControlWindow()->unregisterObject( obj );
    else
      qApp->postEvent( getControlWindow(), new UnmapObjectEvent( obj ) );
  }
}

void Anatomist::UpdateInterface()
{
  if( getControlWindow() != 0 && !_privData->destroying )
  {
    if( Thread::currentIsMainThread() )
      getControlWindow()->UpdateMenus();
    else
      qApp->postEvent( getControlWindow(), new UpdateControlWindowEvent );
  }
}

//	Object operations

list<AObject *> Anatomist::loadObject( const string & filename,
                                       const string & objname,
                                       Object options )
{
  string name;
  if( objname.empty() )
    name = FileUtil::basename( filename );
  else
    name = objname;

  list<AObject*> objects;
  ObjectReader::PostRegisterList subObjectsToRegister;
  try
  {
    objects = ObjectReader::reader()->load( filename, subObjectsToRegister,
                                           true, options );
  }
  catch( assert_error& e )
  {
    cerr << "cannot open file" << endl;
  }
  if( !objects.empty() )
  {
    if( !objname.empty() && objects.size() == 1 )
      (*objects.begin())->setName( makeObjectName( name ) );
    bool  visible = true;
    if( options )
    {
      try
      {
        Object x = options->getProperty( "hidden" );
        if( x && (bool) x->getScalar() )
          visible = false;
      }
      catch( ... )
      {
      }
    }
    list<AObject *>::iterator io, eo = objects.end();
    for( io=objects.begin(); io!=eo; ++io )
      registerObject( *io, visible );
    // register sub-objects also created (if any)
    ObjectReader::PostRegisterList::const_iterator ipr,
      epr = subObjectsToRegister.end();
    for( ipr=subObjectsToRegister.begin(); ipr!=epr; ++ipr )
      registerObject( ipr->first, ipr->second );
  }

  return( objects );
}


set<AObject *> Anatomist::destroyObjects( const set<AObject *> & obj,
                                          bool verbose )
{
  /* We may try several passes because they may have dependencies
     in their lives, and some of them will allow deletion after their parent
     objects are also deleted.
  */
  set<AObject *> objs( obj.begin(), obj.end() );
  set<AObject *>::iterator io, jo, eo = objs.end();

  while( !objs.empty() )
  {
    bool changed = false;

    for( io=objs.begin(); io!=eo; )
    {
      if( destroyObject( *io, false ) )
      {
        changed = true;
        jo = io;
        ++io;
        objs.erase( jo );
      }
      else
        ++io;
    }
    if( !changed )
      break;
  }
  if( verbose && !objs.empty() )
  {
    // retry just to print warnings
    for( io=objs.begin(); io!=eo; ++io )
      destroyObject( *io );
  }

  return objs;
}


int Anatomist::destroyObject( AObject *obj, bool verbose )
{
#ifdef ANA_DEBUG
  cout << "destroyObject: " << obj << " (" << typeid( *obj ).name() << ")\n";
#endif
  if( !hasObject( obj ) )
    return 1;  // already detroyed or released

  if( obj->CanBeDestroyed() )
  {
    // send event
    Object	ex( (GenericObject *)
          new ValueObject<Dictionary> );
    ex->setProperty( "_object", obj );
    set<string>	disc;
    disc.insert( "object" );
    OutputEvent	ev( "DeleteObject", ex, false, disc );
    ev.send();

    //assert( obj->tryDelete() );
    if ( !obj->tryDelete() )
    {
      return 0;
    }

#ifdef ANA_DEBUG
    cout << "done destroyObject: " << obj << endl;
#endif
    return 1;
  }
  else
  {
    stringstream	s;
    // count references not owned by the anatomist application
    int refs = rc_ptr_trick::refCount( *obj );
    carto::shared_ptr<AObject> p = _privData->anaObj.find( obj )->second;
    if( p.referenceType() != carto::shared_ptr<AObject>::Weak )
      --refs;
    if( verbose )
    {
      s << "Cannot delete object " << obj->name()
          << ",\ncheck for multi-objects which contain it. There are still "
          << refs << " other references to it\n";
      AWarning( s.str().c_str() );
    }
    return 0;
  }
}


//	Communication with objects

void Anatomist::Refresh()
{
  using carto::shared_ptr;

  map<const AWindow *, shared_ptr<AWindow> >::iterator	i;
  for( i=_privData->anaWin.begin(); i!=_privData->anaWin.end(); ++i )
    if( i->first->RefreshFlag() )
      i->second.get()->Refresh();
}


//	Windows manipulation


void Anatomist::NotifyMapWindow( AWindow* win )
{
  if( getControlWindow() != 0 && !_privData->destroying )
    getControlWindow()->registerWindow( win );
}


void Anatomist::NotifyUnmapWindow( AWindow* win )
{
  if( getControlWindow() != 0 )
    getControlWindow()->unregisterWindow( win );
}



//	Functions


string Anatomist::makeObjectName( const string & name )
{
  string::size_type	pos = name.rfind( '/' );
  if( pos == string::npos )
    pos = 0;
  else
    ++pos;
  //	remove path
  string	name2 = name.substr( pos, name.length() - pos );
  lockObjects( true );
  map<string, AObject *>::iterator	fn=_privData->anaNameObj.end();
  bool					e;
  unsigned				i = 2;
  char num[10];

  pos = name2.length();

  do
  {
    e = true;
    if( _privData->anaNameObj.find( name2 ) != fn )
    {
      e = false;
      name2.erase( pos, name2.length() - pos );
      sprintf( num, "%d", i );
      name2 += string( " (" ) + num + ")";
      ++i;
    }
  }
  while( !e );

  lockObjects( false );

  return name2;
}


void Anatomist::registerObjectName( const string& name, AObject* obj )
{
  lockObjects( true );
  if ( _privData->anaNameObj.find( name ) == _privData->anaNameObj.end() )
    _privData->anaNameObj[ name ] = obj;
  lockObjects( false );
}


void Anatomist::unregisterObjectName( const string& name )
{
  lockObjects( true );
  if ( _privData->anaNameObj.find( name ) != _privData->anaNameObj.end() )
    _privData->anaNameObj.erase( name );
  lockObjects( false );
}


int Anatomist::groupWindows( set<AWindow*> & winL, int g )
{
  using carto::shared_ptr;

  set<AWindow*>::iterator	iw, fw;
  map<const AWindow *, shared_ptr<AWindow> >::iterator
      is, es = _privData->anaWin.end();

  //	find a new unused group

  set<int>	groups;

  if( g < 0 )
  {
    for( is=_privData->anaWin.begin(); is!=es; ++is )
      groups.insert( is->first->Group() );

    for( g=1; groups.find( g )!=groups.end(); ++g ) {}
  }

  //	now set it on each given window

  for( iw=winL.begin(), fw=winL.end(); iw!=fw; ++iw )
  {
    if( getControlWindow() != 0)
      getControlWindow()->unregisterWindow( *iw );
    (*iw)->setGroup( g );
    if( getControlWindow() != 0)
      getControlWindow()->registerWindow( *iw );
  }

  return g;
}


void Anatomist::ungroupWindows( int group )
{
  using carto::shared_ptr;

  map<const AWindow *, shared_ptr<AWindow> >::iterator
      iw, fw=_privData->anaWin.end();

  if( group == 0 )
    {
      AWarning( "Attempt to unlink group 0 of unlinked windows" );
      return;
    }

  for( iw=_privData->anaWin.begin(); iw!=fw; ++iw )
  {
    AWindow *w = iw->second.get();
    if( w->Group() == group )
    {
      if( getControlWindow() != 0)
        getControlWindow()->unregisterWindow( w );
      w->setGroup( 0 );
      if( getControlWindow() != 0)
        getControlWindow()->registerWindow( w );
    }
  }
}


set<AWindow*> Anatomist::getWindowsInGroup( int group )
{
  using carto::shared_ptr;

  map<const AWindow *, shared_ptr<AWindow> >::iterator
      iw, fw=_privData->anaWin.end();
  set<AWindow*>		lw;

  for( iw=_privData->anaWin.begin(); iw!=fw; ++iw )
    if( iw->first->Group() == group )
      lw.insert( iw->second.get() );

  return( lw );
}


void Anatomist::NotifyObjectChange( AObject* obj )
{
  if (getControlWindow() != 0)
    getControlWindow()->NotifyObjectChange( obj );
}


void Anatomist::NotifyWindowChange( AWindow* win )
{
  if (getControlWindow() != 0)
    getControlWindow()->NotifyWindowChange( win );
}

anatomist::Transformation*
Anatomist::getTransformation( const Referential* src,
                              const Referential* dst )
{
  return( ATransformSet::instance()->transformation( src, dst ) );
}


const anatomist::Transformation* 
Anatomist::getTransformation( const Referential* src, 
			      const Referential* dst ) const
{
  return( ATransformSet::instance()->transformation( src, dst ) );
}


void Anatomist::createControlWindow()
{
  using carto::shared_ptr;

  if (!getControlWindow())
  {
    new ControlWindow;
    getControlWindow()->show();
    for( map<const AWindow *, shared_ptr<AWindow> >::iterator
         i = _privData->anaWin.begin();
         i != _privData->anaWin.end(); ++i )
      getControlWindow()->registerWindow( i->second.get() );
  }
}


ControlWindow* Anatomist::getControlWindow() const
{
  return ControlWindow::theControlWindow();
}

QWidget* Anatomist::getQWidgetAncestor() const{
  return _privData->qWidgetAncestor;
}

void Anatomist::createReferentialWindow()
{
  if( !_privData->referentialWindow )
  {
    _privData->referentialWindow
      = new ReferentialWindow( getQWidgetAncestor(), "referential",
                               Qt::Window );
    _privData->referentialWindow->setAttribute( Qt::WA_DeleteOnClose );
  }
  _privData->referentialWindow->show();
}


void Anatomist::unregisterReferentialWindow()
{
  if( _privData->referentialWindow )
  {
    _privData->referentialWindow = 0;
  }
}


ReferentialWindow* Anatomist::getReferentialWindow() const
{
  return _privData->referentialWindow;
}

void Anatomist::registerSubObject( MObject* parent, AObject* obj )
{
  registerObject( obj, false );
  if( getControlWindow() )
    getControlWindow()->registerSubObject( parent, obj );
}


PaletteList & Anatomist::palettes()
{
  return( *_privData->paletteList );
}


const PaletteList & Anatomist::palettes() const
{
  return( *_privData->paletteList );
}


bool Anatomist::initialized() const
{
  return( _privData->initialized );
}


void Anatomist::setCursor( Cursor c )
{
  if( _privData->cursorChanged )
    {
      _privData->cursorChanged = false;
      QApplication::restoreOverrideCursor();
    }

  switch( c )
    {
    case Normal:
      break;
    case Working:
      _privData->cursorChanged = true;
      // and Qt mechanism for Qt widgets
      QApplication::setOverrideCursor( Qt::WaitCursor );
      break;
    }
}


GlobalConfiguration* Anatomist::config()
{
  return _privData->config;
}


const string & Anatomist::objectsFileFilter() const
{
  return( _privData->objectsFileFilter );
}


void Anatomist::addObjectsFileFilter( const string & filter )
{
  _privData->specificFilters += ";;";
  _privData->specificFilters += filter;
  string::size_type	p1, p2;
  p1 = filter.find( '(' );
  if( p1 != string::npos )
    {
      p2 = filter.rfind( ')' );
      if( p2 != string::npos )
	_privData->allObjectsFilter.
	  insert( _privData->allObjectsFilter.length()-3, p1, p2-p1 );
    }

  _privData->objectsFileFilter = _privData->allObjectsFilter 
    + _privData->specificFilters + _privData->allFilesFilter;
}


bool Anatomist::glMakeCurrent()
{
  if( !qApp )
    return( false );	// no application: unable to create a context
  GLWidgetManager::sharedWidget()->makeCurrent();
  return( true );
}


const CommandWriter & Anatomist::historyWriter() const
{
  return( *_privData->historyW );
}


CommandWriter & Anatomist::historyWriter()
{
  return( *_privData->historyW );
}



Referential* Anatomist::centralReferential() const
{
  if( !_privData->centralRef || _privData->centralRef->destroying() )
    return 0;
  return _privData->centralRef;
}


int Anatomist::userLevel() const
{
  return _privData->userLevel;
}


void Anatomist::setUserLevel( int x )
{
  _privData->userLevel = x;
}


void Anatomist::setLastPosition( const Point3df & pos, Referential* fromref )
{
  _privData->lastpos[0] = pos[0];
  _privData->lastpos[1] = pos[1];
  _privData->lastpos[2] = pos[2];
  _privData->lastref = fromref;
  // try to go to a more persistent referential
  if( fromref )
  {
    Transformation *t = ATransformSet::instance()->transformation
      ( fromref, centralReferential() );
    if( t )
    {
      Point3df tpos = t->transform( pos );
      _privData->lastpos[0] = tpos[0];
      _privData->lastpos[1] = tpos[1];
      _privData->lastpos[2] = tpos[2];
      _privData->lastref = centralReferential();
    }
  }
}


void Anatomist::setLastPosition( const vector<float> & pos,
                                 Referential* fromref )
{
  // try to go to a more persistent referential
  Transformation *t = 0;
  if( fromref )
  {
    t = ATransformSet::instance()->transformation
      ( fromref, centralReferential() );
    if( t )
    {
      Point3df tpos( pos[0], pos[1], pos[2] );
      tpos = t->transform( tpos );
      _privData->lastpos[0] = tpos[0];
      _privData->lastpos[1] = tpos[1];
      _privData->lastpos[2] = tpos[2];
      _privData->lastref = centralReferential();
    }
  }
  if( !t )
  {
    _privData->lastpos[0] = pos[0];
    _privData->lastpos[1] = pos[1];
    _privData->lastpos[2] = pos[2];
    _privData->lastref = fromref;
  }
  if( pos.size() > 3 )
  {
    _privData->lastpos.resize( 3 );
    _privData->lastpos.reserve( pos.size() );
    unsigned i, n = pos.size();
    for( i=3; i<n; ++i )
      _privData->lastpos.push_back( pos[i] );
  }
}


Point3df Anatomist::lastPosition( const Referential* toref ) const
{
  if( _privData->lastref 
      && _privData->anaRef.find( _privData->lastref )
         == _privData->anaRef.end() )
    _privData->lastref = 0;
  return Transformation::transform(
    Point3df( _privData->lastpos[0], _privData->lastpos[1],
              _privData->lastpos[2] ),
    _privData->lastref, toref, Point3df( 1, 1, 1 ), Point3df( 1, 1, 1 ) );
}


vector<float> Anatomist::lastFullPosition( const Referential* toref ) const
{
  vector<float> pos = _privData->lastpos;
  if( _privData->lastref
      && _privData->anaRef.find( _privData->lastref )
         == _privData->anaRef.end() )
    _privData->lastref = 0;
  Referential *ref = _privData->lastref;
  if( !ref )
    ref = centralReferential();
  const Referential *dref = toref;
  if( !dref )
    dref = centralReferential();
  Transformation *t = ATransformSet::instance()->transformation( ref, dref );
  if( t )
  {
    Point3df pt( pos[0], pos[1], pos[2] );
    pt = t->transform( pt );
    pos[0] = pt[0];
    pos[1] = pt[1];
    pos[2] = pt[2];
  }

  return pos;
}


bool Anatomist::hasWindow( const AWindow* win ) const
{
  return( _privData->anaWin.find( win ) != _privData->anaWin.end() );
}


bool Anatomist::hasObject( const AObject* obj ) const
{
  return( _privData->anaObj.find( obj ) != _privData->anaObj.end() );
}


set<AObject* > Anatomist::getObjects() const
{
  using carto::shared_ptr;

  map<const AObject *, shared_ptr<AObject> >::const_iterator
      i, e = _privData->anaObj.end();
  set<AObject *>  objs;
  for( i=_privData->anaObj.begin(); i!=e; ++i )
    objs.insert( i->second.get() );
  return objs;
}


set<AWindow*> Anatomist::getWindows() const
{
  using carto::shared_ptr;

  map<const AWindow *, shared_ptr<AWindow> >::const_iterator
      i, e = _privData->anaWin.end();
  set<AWindow *>  wins;
  for( i=_privData->anaWin.begin(); i!=e; ++i )
    wins.insert( i->second.get() );
  return wins;
}


const set<Referential*> & Anatomist::getReferentials() const
{
  return _privData->anaRef;
}


bool Anatomist::hasReferential( const Referential * ref )
{
  return _privData->anaRef.find( const_cast<Referential *>( ref ) )
      != _privData->anaRef.end();
}


bool Anatomist::destroying() const
{
  return !theAnatomist || !_privData || _privData->destroying
    || ( qApp && qApp->closingDown() );
}


  namespace internal
  {

    void AnatomistPrivate::setAnatomistDestroying( bool x )
    {
      theAnatomist->_privData->destroying = x;
    }

  }


void Anatomist::lockObjects( bool locked )
{
  if( locked )
    _privData->objectsLock.lock();
  else
    _privData->objectsLock.unlock();
}


bool Anatomist::exitOnQuit() const
{
  return _privData->exitOnQuit;
}


void Anatomist::setExitOnQuit( bool x )
{
  _privData->exitOnQuit = x;
}


QWidget* Anatomist::defaultWindowsBlock() const
{
  return _privData->defaultWindowsBlock;
}


void Anatomist::setDefaultWindowsBlock( QWidget* wid )
{
  if( _privData->defaultWindowsBlock != wid )
  {
    if( _privData->defaultWindowsBlock )
    {
      QAWindowBlock *wb
        = dynamic_cast<QAWindowBlock *>( _privData->defaultWindowsBlock );
      if( wb )
        wb->setDefaultBlockGui( false );
    }
    _privData->defaultWindowsBlock = wid;
    if( wid )
    {
      QAWindowBlock *wb = dynamic_cast<QAWindowBlock *>( wid );
      if( wb )
        wb->setDefaultBlockGui( true );
    }
  }
}

}

