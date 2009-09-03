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
#include <anatomist/application/syntax.h>
#include <anatomist/application/settings.h>
#include <anatomist/application/listDir.h>
#include <anatomist/window/glwidgetmanager.h>
#include <anatomist/observer/observcreat.h>
#include <anatomist/object/objectmenuCallbacks.h>
#include <anatomist/fusion/fusionFactory.h>
#include <anatomist/selection/selectFactory.h>
#include <aims/def/path.h>
#include <cartobase/plugin/plugin.h>
#include <cartobase/stream/directory.h>
#include <cartobase/stream/fileutil.h>
#include <cartobase/config/version.h>
#include <cartobase/config/paths.h>
#include <cartobase/smart/rcptrtrick.h>
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


string Anatomist::versionString() const
{
  return carto::cartobaseVersionString();
}




string Anatomist::libraryVersionString() const
{
  return carto::cartobaseShortVersion();
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
        if( w->icon() == 0 )
          {
            static QPixmap	icn( ( Settings::globalPath() 
                                       + "/icons/icon.xpm" ).c_str() );
            if( !icn.isNull() )
              w->setIcon( icn );
          }
      }
    return false;
  }

}




//	Private data structure

struct Anatomist::Anatomist_privateData
{
  Anatomist_privateData();
  ~Anatomist_privateData();

  /* the map and double pointers is for finding windows that could be
  already deleted, so we cannot take a smart pointer on them */
  map<const AWindow *, shared_ptr<AWindow> > anaWin;
  /* the map and double pointers is for finding objects that could be
    already deleted, so we cannot take a smart pointer on them */
  map<const AObject *, shared_ptr<AObject> > anaObj;
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
  Point3df		lastpos;
  mutable Referential	*lastref;
};


ReferentialWindow* Anatomist::Anatomist_privateData::referentialWindow = 0;


Anatomist::Anatomist_privateData::Anatomist_privateData()
  : historyW( new CommandWriter ), paletteList( 0 ),
    initialized( false ), cursorChanged( false ), config( 0 ), centralRef( 0 ),
    userLevel( 0 ), lastpos( 0, 0, 0 ),
    lastref( 0 )
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
}


//	Constructor

Anatomist::Anatomist( int argc, const char **argv, 
		      const std::string &documentation ) 
  : AimsApplication( argc, argv, documentation ), 
    _privData( new Anatomist::Anatomist_privateData )
{
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
  AObject	*obj;


  while( !_privData->anaWin.empty() )
    delete _privData->anaWin.begin()->second.get();

  
  anatomist::Cursor::cleanStatic();
  
  while( !_privData->anaObj.empty() )
  {
    obj = _privData->anaObj.begin()->second.get();
    if( !obj )
    {
      cerr << "BUG: null object in Anatomist list" << endl;
      continue;
    }
    while( !obj->Parents().empty() )
      obj = *obj->Parents().begin();
#ifdef ANA_DEBUG
    cout << "~Anatomist: destroy object " << obj << " (still "
         << _privData->anaObj.size()
         << " in memory) ..." << endl;
#endif
    if( !destroyObject( obj ) )
    {
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
#ifdef ANA_DEBUG
    cout << "~Anatomist: object " << obj << " destroyed" << endl;
#endif
    }

  // cleanup some global variables
  delete getControlWindow();
  delete FusionFactory::factory();
  AObject::cleanStatic();
  delete SelectFactory::factory();

  delete _privData;
  theAnatomist = 0;
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

  // palettes
  /* palettes should be read before the config because they might be used by 
     config options, like cursors */

  _privData->paletteList->load( Settings::globalPath() + "/rgb" );
  _privData->paletteList->load( Settings::localPath() + sep + "rgb" );
  // cout << _paletteList->size() << " palettes loaded\n";

  //	config
  _privData->config->registerLocalConfiguration( new ControlConfiguration );
  _privData->config->load();

  // Fichier history
  string history_name = Settings::localPath();
  Directory	dir( history_name );
  dir.mkdir();
  history_name += ssep + "history.ana";
  // keep backup
  rename( history_name.c_str(), (history_name + "~").c_str() );
  _privData->historyW->open( history_name.c_str() );
  if ( !*_privData->historyW )
    {
      AWarning("Unable to open history file. No output will be written");
    }
  else
    {
      theProcessor->addObserver( _privData->historyW );
    }

  //	Language

  string langpath = Settings::globalPath() + sep + "po" + sep;

  string lang = language();
  string langpathold = langpath;
  langpath += lang;
  list<string>	translations 
    = anatomist::listDirectory( langpath, "*.qm", QDir::Name, 
				QDir::Files | QDir::Readable );

  if( translations.empty() && lang != "en" )
    {
      langpath = langpathold + "en";
      translations = anatomist::listDirectory( langpath, "*.qm", QDir::Name, 
					       QDir::Files | QDir::Readable );
    }

  list<string>::iterator	it, et = translations.end();
  for( it=translations.begin(); it!=et; ++it )
    {
      QTranslator	*tr = new QTranslator( qApp, "Translator" );
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
      /* cout << "cast 1: " << dynamic_cast<ValueObject<string> *>( over.get() ) 
           << endl;
      cout << "cast 2: " << dynamic_cast<TypedObject<string> *>( over.get() ) 
      << endl; */
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
	  /* cout << "version : maj : " << majv << " . min : " << minv 
	       << endl;
	  cout << "previous version : maj : " << majo << " . min : " << mino 
	  << endl; */
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

  // file filters
  _privData->allObjectsFilter 
    = string( ControlWindow::tr( "All Anatomist objects" ).utf8().data() ) 
    + " (*.vimg *.vimg.Z *.vimg.gz *.img *.img.Z *.img.gz *.ima *.ima.Z" 
    + " *.ima.gz *.v *.i *.p *.fdf *.jpg *.tif *.png *.bmp *.gif *.mng *.pbm *.pgm "
    "*.ppm *.xbm *.xpm *.mnc *.mnc.Z *.mnc.gz *.nii *.nii.gz"
    " *.tri *.tri.Z *.tri.gz *.mesh *.mesh.Z *.mesh.gz"
    " *.arg *.bundles"
    " *.bck"
    " *.hie"
    " *.tex *.erp"
    " *.ana);;";
  _privData->specificFilters 
    = string( ControlWindow::tr( "Volumes" ).utf8().data() ) 
    + " (*.vimg *.vimg.Z *.vimg.gz *.img *.img.Z *.img.gz *.ima *.ima.Z "
    + "*.ima.gz *.v *.i *.p *.fdf *.jpg *.png *.bmp *.gif *.mng *.pbm *.pgm " 
    "*.ppm *.xbm *.xpm *.mnc *.mnc.Z *.mnc.gz *.nii *.nii.gz);;"
    + ControlWindow::tr( "DICOM" ).utf8().data() 
    + " (*);;"
    + ControlWindow::tr( "Triangular meshes" ).utf8().data() 
    + " (*.tri *.tri.Z *.tri.gz *.mesh *.mesh.Z *.mesh.gz);;"
    + ControlWindow::tr( "Graphs" ).utf8().data() 
    + " (*.arg *.bundles);;"
    + ControlWindow::tr( "ROIs" ).utf8().data() 
    + " (*.bck);;"
    + ControlWindow::tr( "Nomenclatures" ).utf8().data() 
    + " (*.hie);;"
    + ControlWindow::tr( "Textures" ).utf8().data() 
    + " (*.tex *.val *.erp);;" 
    + ControlWindow::tr( "Scripts" ).utf8().data() 
    + " (*.ana)";
  _privData->allFilesFilter = string( ";;" ) 
    + ControlWindow::tr( "All files" ).utf8().data() 
    + " (*)";
  _privData->objectsFileFilter = _privData->allObjectsFilter 
    + _privData->specificFilters + _privData->allFilesFilter;

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
  string talref = carto::Paths::shfjShared() + sep + "registration" + sep \
      + "Talairach-AC_PC-Anatomist.referential";
  _privData->centralRef->load( talref );
  Referential::mniTemplateReferential(); // create SPM/MNI referential

  string	modpath = Settings::localPath();
  if ( !modpath.empty() )
    {
      modpath += ssep + "plugins" + sep + "anatomist.plugins";
      PluginLoader::pluginFiles().push_back
        ( PluginLoader::PluginFile( modpath, versionString() ) );
    }
  modpath = Settings::globalPath();
  if ( !modpath.empty() )
    {
      modpath += ssep + "plugins" + sep + "anatomist.plugins";
      PluginLoader::pluginFiles().push_back
        ( PluginLoader::PluginFile( modpath, versionString() ) );
    }
  PluginLoader::load();	// load plugins in new path

  // event filter to add windows icons
  qApp->installEventFilter( new EventFilter );

  // process args, load files etc.

  if( !args.batch )
    {
      Command* command = new CreateControlWindowCommand;
      theProcessor->execute(command);
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
  if( getControlWindow() != 0)
    getControlWindow()->unregisterWindow( win );

  map<const AWindow *, shared_ptr<AWindow> >::iterator i
      = _privData->anaWin.find( win );
  if( i != _privData->anaWin.end() )
    _privData->anaWin.erase( i );
}


void Anatomist::releaseWindow( AWindow* win )
{
  map<const AWindow *, shared_ptr<AWindow> >::iterator
      i = _privData->anaWin.find( win );
  // change the smart pointer type to Weak
  if( i != _privData->anaWin.end() )
    i->second.reset( shared_ptr<AWindow>::Weak, win );
}


void Anatomist::registerObject( AObject* obj, int inctrl )
{
#ifdef ANA_DEBUG
  cout << "Anatomist::registerObject " << obj << " (" 
       << typeid( *obj ).name() << ") - " << obj->name() << endl;
#endif
  _privData->anaObj[ obj ]
      = shared_ptr<AObject>( shared_ptr<AObject>::WeakShared, obj );
  if( inctrl ) mapObject( obj );
  // call the observable notifier
  ObservableCreatedNotifier::notifyCreated( obj );
}


void Anatomist::unregisterObject( AObject* obj )
{
#ifdef ANA_DEBUG
  cout << "Anatomist::unregisterObject " << obj << " ("
      << typeid( *obj ).name() << ") - " << obj->name() << endl;
#endif
  unmapObject( obj );

  map<const AObject *, shared_ptr<AObject> >::iterator i
      = _privData->anaObj.find( obj );
  if( i != _privData->anaObj.end() )
    _privData->anaObj.erase( i );
#ifdef ANA_DEBUG
  else
    cerr << "  - nothing to unregister" << endl;
#endif 
}


void Anatomist::releaseObject( AObject* obj )
{
  map<const AObject *, shared_ptr<AObject> >::iterator
      i = _privData->anaObj.find( obj );
  // change the smart pointer type to Weak
  if( i != _privData->anaObj.end() )
    i->second.reset( shared_ptr<AObject>::Weak, obj );
}


void Anatomist::registerReferential( Referential* ref )
{
  _privData->anaRef.insert( ref );
  if( _privData->referentialWindow != 0 )
    _privData->referentialWindow->refresh();
}


void Anatomist::unregisterReferential( Referential* ref )
{
  set<Referential*>::iterator i = _privData->anaRef.find( ref );
  if( i != _privData->anaRef.end() )
    {
      ATransformSet::instance()->deleteTransformationsWith( ref );
      _privData->anaRef.erase( i );
      if( _privData->referentialWindow != 0 )
        _privData->referentialWindow->refresh();
    }
  else
    cerr << "Anatomist::unregisterReferential : ref not found\n";
}


void Anatomist::mapObject( AObject* obj )
{
  if( getControlWindow() != 0)
    getControlWindow()->registerObject( obj );
}


void Anatomist::unmapObject( AObject* obj )
{
  if( getControlWindow() != 0)
    getControlWindow()->unregisterObject( obj );
}

void Anatomist::UpdateInterface()
{
  if( getControlWindow() != 0)
    getControlWindow()->UpdateMenus();
}

//	Object operations

AObject* Anatomist::loadObject( const string & filename, 
                                const string & objname, 
                                Object options )
{
  string name;
  if( objname.empty() )
    name = FileUtil::basename( filename );
  else
    name = objname;

  AObject* object = 0;
  try
  {
    object = ObjectReader::reader()->load( filename, true, options );
  }
  catch( assert_error& e )
  {
    cerr << "cannot open file" << endl;
  }
  if( object )
  {
    if( !objname.empty() )
      object->setName( makeObjectName( name ) );
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
    registerObject( object, visible );
  }

  return( object );
}


//	Window operations

int Anatomist::destroyObject( AObject *obj )
{
#ifdef ANA_DEBUG
  cout << "destroyObject: " << obj << " (" << typeid( *obj ).name() << ")\n";
#endif
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
      s << "Cannot delete object " << obj->name()
          << ",\ncheck for multi-objects which contain it. There are still "
          << rc_ptr_trick::refCount( *obj )-1 << " other references to it\n";
      AWarning( s.str().c_str() );
      return 0;
    }
}


//	Communication with objects

void Anatomist::Refresh()
{
  map<const AWindow *, shared_ptr<AWindow> >::iterator	i;
  for( i=_privData->anaWin.begin(); i!=_privData->anaWin.end(); ++i )
    if( i->first->RefreshFlag() )
      i->second.get()->Refresh();
}


//	Windows manipulation


void Anatomist::NotifyMapWindow( AWindow* win )
{
  if( getControlWindow() != 0)
    getControlWindow()->registerWindow( win );
}


void Anatomist::NotifyUnmapWindow( AWindow* win )
{
  if( getControlWindow() != 0)
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

  return( name2 );
}


void Anatomist::registerObjectName( const string& name, AObject* obj )
{
  if ( _privData->anaNameObj.find( name ) == _privData->anaNameObj.end() )
    _privData->anaNameObj[ name ] = obj;
}


void Anatomist::unregisterObjectName( const string& name )
{
  if ( _privData->anaNameObj.find( name ) != _privData->anaNameObj.end() )
    _privData->anaNameObj.erase( name );
}


int Anatomist::groupWindows( set<AWindow*> & winL, int g )
{
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
      (*iw)->SetGroup( g );
      if( getControlWindow() != 0)
        getControlWindow()->registerWindow( *iw );
    }

  return g;
}


void Anatomist::ungroupWindows( int group )
{
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
	w->SetGroup( 0 );
	if( getControlWindow() != 0)
	  getControlWindow()->registerWindow( w );
      }
  }
}


set<AWindow*> Anatomist::getWindowsInGroup( int group )
{
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

Transformation* Anatomist::getTransformation( const Referential* src, 
					      const Referential* dst )
{
  return( ATransformSet::instance()->transformation( src, dst ) );
}


const Transformation* 
Anatomist::getTransformation( const Referential* src, 
			      const Referential* dst ) const
{
  return( ATransformSet::instance()->transformation( src, dst ) );
}


void Anatomist::createControlWindow()
{
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


void Anatomist::createReferentialWindow()
{
  if( !_privData->referentialWindow )
  {
    _privData->referentialWindow = new ReferentialWindow( 0, "referential" );
  }
  _privData->referentialWindow->show();
}


ReferentialWindow* Anatomist::getReferentialWindow() const
{
  return _privData->referentialWindow;
}

string Anatomist::catObjectNames( const set<AObject *> &setobj ) const
{
  set<AObject *>::const_iterator obj, fo = setobj.end();
  string  nameWin;
  AObject::ParentList::const_iterator	ip, fp;
  bool				first = true;

  if( setobj.empty() )
    return( nameWin );

  for( obj=setobj.begin(); obj!=fo; ++obj )
    {
      for( ip=(*obj)->Parents().begin(), fp=(*obj)->Parents().end(); 
           ip!=fp && setobj.find( *ip ) == fo; ++ip ) {}
      if( ip == fp )	// no parent in list
        {
          if( first )
            first = false;
          else
            nameWin += ", ";
          switch( (*obj)->type() )
	    {
	    case AObject::FUSION2D:
	    case AObject::FUSION3D:
	      {
		MObject::iterator	io, fo = ((MObject *) *obj)->end();
		nameWin += "(";
		io=((MObject *) *obj)->begin();
		if( !(*io)->name().empty() )
		  nameWin += (*io)->name();
		else
		  nameWin += "<unnamed>";
		for( ++io; io!=fo; ++io )
		  {
		    nameWin += ",";
		    if( !(*io)->name().empty() )
		      nameWin += (*io)->name();
		    else
		      nameWin += "<unnamed>";
		  }
		nameWin += ")";
	      }
	      break;
	    default:
	      if( !(*obj)->name().empty() )
		nameWin += (*obj)->name();
	      else
		nameWin += "<unnamed>";
	      break;
	    }
	}
    }

  return( nameWin );
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
      QApplication::setOverrideCursor( Qt::waitCursor );
      break;
    }
}


GlobalConfiguration* Anatomist::config()
{
  return( _privData->config );
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
  _privData->lastpos = pos;
  _privData->lastref = fromref;
  // try to go to a more persistent referential
  if( fromref )
    {
      Transformation *t = ATransformSet::instance()->transformation
	( fromref, centralReferential() );
      if( t )
	{
	  _privData->lastpos = t->transform( pos );
	  _privData->lastref = centralReferential();
	}
    }
}


Point3df Anatomist::lastPosition( const Referential* toref ) const
{
  if( _privData->lastref 
      && _privData->anaRef.find( _privData->lastref )
         == _privData->anaRef.end() )
    _privData->lastref = 0;
  return Transformation::transform( _privData->lastpos, _privData->lastref, 
				    toref, Point3df( 1, 1, 1 ), 
				    Point3df( 1, 1, 1 ) );
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
  map<const AObject *, shared_ptr<AObject> >::const_iterator
      i, e = _privData->anaObj.end();
  set<AObject *>  objs;
  for( i=_privData->anaObj.begin(); i!=e; ++i )
    objs.insert( i->second.get() );
  return objs;
}


set<AWindow*> Anatomist::getWindows() const
{
  map<const AWindow *, shared_ptr<AWindow> >::const_iterator
      i, e = _privData->anaWin.end();
  set<AWindow *>  wins;
  for( i=_privData->anaWin.begin(); i!=e; ++i )
    wins.insert( i->second.get() );
  return wins;
}


set<Referential*> Anatomist::getReferentials() const
{
  return _privData->anaRef;
}


bool Anatomist::hasReferential( const Referential * ref )
{
  return _privData->anaRef.find( const_cast<Referential *>( ref ) )
      != _privData->anaRef.end();
}

