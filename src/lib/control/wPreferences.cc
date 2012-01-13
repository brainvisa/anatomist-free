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


#include <qslider.h>

#include <anatomist/control/wPreferences.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/application/settings.h>
#include <anatomist/application/fileDialog.h>
#include <anatomist/control/wControl.h>
#include <anatomist/window/Window.h>
#include <anatomist/window3D/cursor.h>
#include <anatomist/application/globalConfig.h>
#include <anatomist/application/listDir.h>
#include <anatomist/reference/Referential.h>
#include <anatomist/reference/wChooseReferential.h>
#include <anatomist/commands/cLoadObject.h>
#include <anatomist/processor/Processor.h>
#include <qtabbar.h>
#include <aims/qtcompat/qvgroupbox.h>
#include <aims/qtcompat/qvbuttongroup.h>
#include <aims/qtcompat/qhbuttongroup.h>
#include <qlayout.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <aims/qtcompat/qvbox.h>
#include <aims/qtcompat/qhbox.h>
#include <qlineedit.h>
#include <qcolordialog.h>
#include <qcombobox.h>
#include <qtooltip.h>
#include <qradiobutton.h>
#include <aims/qtcompat/qhgroupbox.h>
#include <qvalidator.h>
#include <aims/qtcompat/qgrid.h>
#include <aims/qtcompat/qfiledialog.h>
#include <cartobase/config/paths.h>
#include <vector>
#include <map>
#include <iostream>


using namespace anatomist;
using namespace carto;
using namespace std;


struct PreferencesWindow::Private
{
  Private()
    : tab( 0 ), cursEdit( 0 ), cursSlider( 0 ), cursColBtn( 0 ), 
      defobjref( 0 ), defwinref( 0 ), browsattlen( 0 ), texmax( 0 ) {}

  unsigned		tab;
  vector<QWidget *>	tabs;
  map<int, unsigned>	tabnum;
  QComboBox		*cursShape;
  QLineEdit		*cursEdit;
  QSlider		*cursSlider;
  QPushButton		*cursColBtn;
  QComboBox		*userLevel;
  QLineEdit		*winszed;
  QCheckBox             *disppos;
  QPushButton		*defobjref;
  QPushButton		*defwinref;
  QLineEdit             *browsattlen;
  QComboBox             *texmax;
};


namespace
{

  void setRefColor( Referential* ref, QPushButton* pb )
  {
    QPixmap	pix;
    if( ref )
      {
        pix.resize( 30, 20 );
        AimsRGB	col = ref->Color();
        pix.fill( QColor( col.red(), col.green(), col.blue() ) );
      }
#if QT_VERSION >= 0x040000
    pb->setIconSet( QIcon( pix ) );
#else
    pb->setIconSet( QIconSet( pix, pix ) );
#endif
  }


  void updateCursorsCombo( QComboBox* cb )
  {
    set<string>			cs = Cursor::cursors();
    set<string>::iterator	ic, ec = cs.end();
    int				cur = 0, ci = 0;

    cb->clear();
    for( ic=cs.begin(); ic!=ec; ++ic, ++ci )
      {
        cb->insertItem( PreferencesWindow::tr( ic->c_str() ) );
        if( ic->c_str() == Cursor::currentCursorName() )
          cur = ci;
      }
    cb->setCurrentItem( cur );
  }

}


PreferencesWindow::PreferencesWindow()
  : QWidget( 0, "prefWin", Qt::WDestructiveClose ), 
    _pdat( new Private )
{
  setCaption( tr( "Anatomist global settings" ) );

  QVBoxLayout	*mainlay = new QVBoxLayout( this, 10, 10 );
  QTabBar	*tbar = new QTabBar( this );

  _pdat->tabnum[ tbar->addTab( tr( "Application" ) )    ] = 0;
  _pdat->tabnum[ tbar->addTab( tr( "Linked cursor" ) )  ] = 1;
  _pdat->tabnum[ tbar->addTab( tr( "Windows" ) )        ] = 2;
  _pdat->tabnum[ tbar->addTab( tr( "Control window" ) ) ] = 3;
  _pdat->tabnum[ tbar->addTab( tr( "Volumes" ) )        ] = 4;
  _pdat->tabnum[ tbar->addTab( tr( "OpenGL" ) )         ] = 5;

  GlobalConfiguration	*cfg = theAnatomist->config();

  //	Application tab

  QVGroupBox	*app = new QVGroupBox( this );
  _pdat->tabs.push_back( app );
  QHBox	*applang = new QHBox( app );
  applang->setSpacing( 5 );
  QLabel	*l = new QLabel( tr( "Language :" ), applang );
  QToolTip::add( l, tr( "Language options will apply the next time you start " 
			"Anatomist (save preferences !)" ) );
  QComboBox	*langbox = new QComboBox( applang );
  string	lang;
  cfg->getProperty( "language", lang );
  langbox->insertItem( tr( "default" ) );

  list<string>  langdl = Paths::findResourceFiles( "po", "anatomist",
    theAnatomist->libraryVersionString() );
  list<string> langdirs;
  list<string>::iterator il, el = langdl.end();
  for( il=langdl.begin(); il!=el; ++il )
  {
    list<string> ll = listDirectory( *il, "", QDir::Name,
                                     QDir::Dirs | QDir::NoSymLinks );
    langdirs.insert( langdirs.end(), ll.begin(), ll.end() );
  }
  list<string>::iterator	id, ed = langdirs.end();
  unsigned			i = 0, langind = 0;

  for( id=langdirs.begin(); id!=ed; ++id, ++i )
    {
      langbox->insertItem( (*id).c_str() );
      if( lang == *id )
	langind = i+1;
    }
  langbox->setCurrentItem( langind );

  QHBox	*appbrow = new QHBox( app );
  appbrow->setSpacing( 5 );
  new QLabel( tr( "HTML browser command line :" ), appbrow );
  QComboBox	*htmlbox = new QComboBox( true, appbrow );
  htmlbox->setDuplicatesEnabled( false );
  htmlbox->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, 
                                       QSizePolicy::Fixed ) );
  // htmlbox->setMaximumWidth( 400 );
  string	htmltxt;
  if( cfg->getProperty( "html_browser", htmltxt ) )
    htmlbox->insertItem( htmltxt.c_str() );
#ifdef __APPLE__
  htmlbox->insertItem( "/Applications/Safari.app/Contents/MacOS/Safari %1" );
  htmlbox->insertItem( "/Applications/Internet\\ Explorer.app/Contents/" 
                       "MacOS/Internet\\ Explorer %1" );
#else
#ifdef _WIN32
  htmlbox->insertItem( "explorer.exe %1" );
#endif
#endif
  htmlbox->insertItem( "konqueror %1 &" );
  htmlbox->insertItem( "firefox %1 &" );
  htmlbox->insertItem( "mozilla %1 &" );
  htmlbox->insertItem( "netscape -noraise -remote openBrowser\\(file:%1\\) "
      "2> /dev/null || netscape -no-about-splash file:%1 &" );
  htmlbox->insertItem( "kfmclient openURL=%1" );
  htmlbox->setCurrentItem( 0 );

  QVGroupBox *unst = new QVGroupBox( tr( "User level" ), app );
  QComboBox *au = new QComboBox( true, unst );
  _pdat->userLevel = au;
  au->insertItem( "Basic" );
  au->insertItem( "Advanced" );
  au->insertItem( "Expert" );
  au->setEditable( true );
  au->setInsertionPolicy( QComboBox::NoInsertion );
  au->setValidator( new QRegExpValidator( QRegExp(
    "\\d*|Basic|Advanced|Expert|Debugger", false ), au ) );
  resetUserLevel();
  connect( au, SIGNAL( activated( const QString & ) ), this,
    SLOT( setUserLevel( const QString & ) ) );

  QVGroupBox	*refs = new QVGroupBox( tr( "Default referentials" ), app );
  QGrid		*refg = new QGrid( 2, refs );
  new QLabel( tr( "Default referential for loaded objects" ), refg );
  _pdat->defobjref = new QPushButton( refg );
  setRefColor( theAnatomist->getControlWindow()->defaultObjectsReferential(), 
               _pdat->defobjref );
  _pdat->defobjref->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, 
                                                QSizePolicy::Fixed ) );
  new QLabel( tr( "Default referential for new windows" ), refg );
  _pdat->defwinref = new QPushButton( refg );
  setRefColor( theAnatomist->getControlWindow()->defaultWindowsReferential(), 
               _pdat->defwinref );
  _pdat->defwinref->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, 
                                                QSizePolicy::Fixed ) );
  connect( _pdat->defobjref, SIGNAL( clicked() ), this, 
           SLOT( changeDefObjectsRef() ) );
  connect( _pdat->defwinref, SIGNAL( clicked() ), this, 
           SLOT( changeDefWindowsRef() ) );

  //	Linked cursor tab

  QVBox	*lkcur = new QVBox( this );
  lkcur->hide();
  lkcur->setSpacing( 5 );
  _pdat->tabs.push_back( lkcur );
  QCheckBox	*cursEnable 
    = new QCheckBox( tr( "Display linked cursor" ), lkcur );
  cursEnable->setChecked( AWindow::hasGlobalCursor() );
  QHGroupBox	*cursShape = 
    new QHGroupBox( tr( "Cursor shape :" ), lkcur );
  _pdat->cursShape = new QComboBox( cursShape );
  QPushButton	*cursShapeBtn = new QPushButton( tr( "Load..." ), cursShape );
  cursShapeBtn->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, 
                                            QSizePolicy::Fixed ) );
  updateCursorsCombo( _pdat->cursShape );

  QHBox	*cursSzBox = new QHBox( lkcur );
  cursSzBox->setSpacing( 10 );
  new QLabel( tr( "Size :" ), cursSzBox );
  _pdat->cursEdit 
    = new QLineEdit( QString::number( AWindow::cursorSize() ), cursSzBox );
  _pdat->cursSlider = new QSlider( 0, 256, 1, AWindow::cursorSize(), 
				   Qt::Horizontal, lkcur );
  QVButtonGroup	*cursCol = new QVButtonGroup( tr( "Cursor color :" ), lkcur );
  new QRadioButton( tr( "Use default color" ), cursCol );
  new QRadioButton( tr( "Custom color :" ), cursCol );
  cursCol->setButton( 1 - AWindow::useDefaultCursorColor() );
  _pdat->cursColBtn = new QPushButton( cursCol );
  AimsRGB	col = AWindow::cursorColor();
  _pdat->cursColBtn->setPalette( QPalette( QColor( col.red(), col.green(), 
						   col.blue() ) ) );

  //	windows tab

  QVBox		*winbox = new QVBox( this );
  _pdat->tabs.push_back( winbox );
  winbox->hide();
  QVGroupBox	*flip 
    = new QVGroupBox( tr( "Axial/coronal slices orientation" ), winbox );
  QVButtonGroup  *flipbx = new QVButtonGroup( flip );
  new QRadioButton( tr( "Radioligical convention (seen from bottom, " 
			"L/R flipped)" ), flipbx );
  new QRadioButton( tr( "Neurological convention (seen from top)" ), flipbx );
  int     btn = 0;
  string  axconv;
  cfg->getProperty( "axialConvention", axconv );
  if( axconv == "neuro" )
    btn = 1;
  flipbx->setButton( btn );
  QCheckBox	*flipDisplay 
    = new QCheckBox( tr( "Display L/R in corners" ), flip );
  flipDisplay->setChecked( AWindow::leftRightDisplay() );
  flipDisplay->setEnabled( false );
  _pdat->disppos = new QCheckBox( tr( "Display cursor position by default" ),
                                  winbox );
  int dispposfg = 1;
  try
  {
    Object  x = cfg->getProperty( "displayCursorPosition" );
    if( !x.isNull() )
      dispposfg = (int) x->getScalar();
  }
  catch( ... )
  {
  }
  _pdat->disppos->setChecked( dispposfg );
  QHGroupBox	*winsz 
    = new QHGroupBox( tr( "Default windows size" ), winbox );
  _pdat->winszed = new QLineEdit( winsz );
  new QLabel( tr( "pixel / mm" ), winsz );
  float	wf = 1.5;
  theAnatomist->config()->getProperty( "windowSizeFactor", wf );
  _pdat->winszed->setText( QString::number( wf ) );
  _pdat->winszed->setValidator( new QDoubleValidator( 0.001, 1e6, 3, 
                                                      _pdat->winszed ) );
  QVGroupBox *brows = new QVGroupBox( tr( "Browsers" ), winbox );
  QHBox *brattlen = new QHBox( brows );
  new QLabel( tr( "limit browsers attribute values to: " ), brattlen );
  _pdat->browsattlen = new QLineEdit( brattlen );
  new QLabel( tr( " characters" ), brattlen );
  _pdat->browsattlen->setValidator( new QIntValidator( 0, 1000000,
    _pdat->browsattlen ) );
  int bal = 0;
  try
  {
    Object  x = cfg->getProperty( "clipBrowserValues" );
    if( !x.isNull() )
      bal = (int) x->getScalar();
  }
  catch( ... )
  {
  }
  _pdat->browsattlen->setText( QString::number( bal ) );

  //	control window tab

  QVGroupBox	*cwin = new QVGroupBox( this );
  cwin->hide();
  _pdat->tabs.push_back( cwin );
  QCheckBox	*cwlogo = new QCheckBox( tr( "Display nice logo" ), cwin );
  ControlWindow	*cw = theAnatomist->getControlWindow();
  cwlogo->setChecked( cw->logoEnabled() );

  //	Volumes tab

  QVGroupBox	*tvol = new QVGroupBox( this );
  tvol->hide();
  _pdat->tabs.push_back( tvol );
  QCheckBox	*tvint = new QCheckBox( tr( "Interpolation on volumes when "
                                            "changing referential" ), tvol );
  int	tvich = 1;
  cfg->getProperty( "volumeInterpolation", tvich );
  tvint->setChecked( tvich );
  QCheckBox	*tvspm 
    = new QCheckBox( tr( "Use referential / transformations "
      "information found in objects headers (SPM, NIFTI...)" ), tvol );
  int	tvuspm = 0;
  try
  {
    Object  x = cfg->getProperty( "setAutomaticReferential" );
    if( !x.isNull() )
      tvuspm = (int) x->getScalar();
  }
  catch( ... )
  {
  }
  tvspm->setChecked( tvuspm );

  QCheckBox     *refscan
    = new QCheckBox( tr(
      "Assume all 'scanner-based' referentials are the same " ), tvol );
  int   refscanu = 0;
  try
  {
    Object  x = cfg->getProperty( "commonScannerBasedReferential" );
    if( !x.isNull() )
      refscanu = (int) x->getScalar();
  }
  catch( ... )
  {
  }
  refscan->setChecked( refscanu );
  
  //    OpenGL tab

  QVGroupBox    *tgl = new QVGroupBox( this );
  tgl->hide();
  _pdat->tabs.push_back( tgl );

  QHBox *btexmax = new QHBox( tgl );
  new QLabel( tr( "limit number of textures: " ), btexmax );
  QComboBox *texmax = new QComboBox( btexmax );
  _pdat->texmax = texmax;
  texmax->setDuplicatesEnabled( false );
  texmax->setSizePolicy( QSizePolicy( QSizePolicy::Expanding,
                                      QSizePolicy::Fixed ) );
  texmax->insertItem( tr( "Unlimited" ) );
  texmax->insertItem( "0" );
  texmax->insertItem( "1" );
  texmax->insertItem( "2" );
  texmax->insertItem( "3" );
  texmax->insertItem( "4" );
  texmax->insertItem( "5" );
  texmax->insertItem( "6" );
  texmax->insertItem( "7" );
  texmax->insertItem( "8" );
  texmax->setEditable( true );
  texmax->setValidator( new QRegExpValidator( QRegExp(
    "\\d*|" + tr( "Unlimited" ) + "|-1", false ), texmax ) );
  #ifdef _WIN32
  /* On Windows for an unknown reason (probably a bug well hidden somewhere in
     anatomist), allowing more than 3 textures results to nothing being 
     displayed, whatever the 3D hardware... So we limit to 3 textures by 
     default by now.
  */
  int   ntexmax = 3;
  #else
  int   ntexmax = -1;
  #endif
  try
  {
    Object  x = cfg->getProperty( "maxTextureUnitsUsed" );
    if( !x.isNull() )
      ntexmax = (int) x->getScalar();
  }
  catch( ... )
  {
  }
  texmax->setCurrentItem( ntexmax + 1 );
  btexmax->setToolTip( tr( "Try this option if you encounter OpenGL rendering "
  "problems.\n"
  "Such problems have been seen on Windows machines, where rendering was not "
  "performed at all\n"
  "if more than 3 texture units were enabled (even on non-tetured objects).\n"
  "Use 'Unlimited' if rendering is OK."
  ) );

  QCheckBox *glselect = new QCheckBox( tr( "Use OpenGL selection" ), tgl );
  int   useglsel = 1;
  try
  {
    Object  x = cfg->getProperty( "disableOpenGLSelection" );
    if( !x.isNull() )
      useglsel = (int) !x->getScalar();
  }
  catch( ... )
  {
  }
  glselect->setChecked( useglsel );
  glselect->setToolTip( tr( "Disabling OpenGL-based selection (in selection "
  "control, and 3D windows tooltips)\n"
  "may be needed with some buggy OpenGL implementations which may cause "
  "Anatomist to crash.\n"
  "The \"Surface Paint\" tool also makes use of it in an unconditional way, "
  "so this module\n"
  "might still crash with such an OpenGL implementation." ) );

  //	top-level widget setting

  mainlay->addWidget( tbar );
  mainlay->addWidget( app );
  mainlay->addWidget( lkcur );
  mainlay->addWidget( winbox );
  mainlay->addWidget( cwin );
  mainlay->addWidget( tvol );
  mainlay->addWidget( tgl );

  connect( tbar, SIGNAL( selected( int ) ), this, SLOT( enableTab( int ) ) );
  connect( htmlbox, SIGNAL( activated( const QString & ) ), this, 
	   SLOT( htmlBrowserChanged( const QString & ) ) );
  connect( langbox, SIGNAL( activated( const QString & ) ), this, 
	   SLOT( languageChanged( const QString & ) ) );
  connect( cursEnable, SIGNAL( toggled( bool ) ), this, 
	   SLOT( enableCursor( bool ) ) );
  connect( _pdat->cursShape, SIGNAL( activated( const QString & ) ), this, 
	   SLOT( setCursorShape( const QString & ) ) );
  connect( cursShapeBtn, SIGNAL( clicked() ), this, SLOT( loadCursor() ) );
  connect( cursCol, SIGNAL( clicked( int ) ), this, 
	   SLOT( setCursorColorMode( int ) ) );
  connect( _pdat->cursEdit, SIGNAL( returnPressed() ), this, 
	   SLOT( cursorEditChanged() ) );
  connect( _pdat->cursSlider, SIGNAL( valueChanged( int ) ), this, 
	   SLOT( cursorSliderChanged( int ) ) );
  connect( _pdat->cursColBtn, SIGNAL( clicked() ), this, 
	   SLOT( choseCursorColor() ) );

  connect( flipDisplay, SIGNAL( toggled( bool ) ), this, 
	   SLOT( enableLRDisplay( bool ) ) );
  connect( flipbx, SIGNAL( clicked( int ) ), this, 
	   SLOT( setAxialConvention( int ) ) );
  connect( _pdat->winszed, SIGNAL( returnPressed() ), this, 
           SLOT( defaultWinSizeChanged() ) );
  connect( _pdat->disppos, SIGNAL( toggled( bool ) ), this,
           SLOT( enableDisplayCursorPosition( bool ) ) );
  connect( _pdat->browsattlen, SIGNAL( returnPressed() ), this,
           SLOT( browserAttributeLenChanged() ) );

  connect( cwlogo, SIGNAL( toggled( bool ) ), 
	   theAnatomist->getControlWindow(), SLOT( enableLogo( bool ) ) );

  connect( tvint, SIGNAL( toggled( bool ) ), this, 
	   SLOT( enableVolInterpolation( bool ) ) );
  connect( tvspm, SIGNAL( toggled( bool ) ), this, 
	   SLOT( enableAutomaticReferential( bool ) ) );
  connect( refscan, SIGNAL( toggled( bool ) ), this,
           SLOT( commonScannerBasedReferential( bool ) ) );

  connect( texmax, SIGNAL( activated( const QString & ) ), this,
           SLOT( setMaxTextures( const QString & ) ) );
  connect( glselect, SIGNAL( toggled( bool ) ), this,
           SLOT( enableOpenGLSelection( bool ) ) );
}


PreferencesWindow::~PreferencesWindow()
{
  if( theAnatomist->getControlWindow() )
    theAnatomist->getControlWindow()->enablePreferencesMenu( true );
}


void PreferencesWindow::enableTab( int tabid )
{
  unsigned tab = _pdat->tabnum[ tabid ];
  if( tab != _pdat->tab )
    {
      _pdat->tabs[ _pdat->tab ]->hide();
      _pdat->tab = tab;
      _pdat->tabs[ tab ]->show();
    }
}


void PreferencesWindow::updateWindows()
{
  set<AWindow*> liswin = theAnatomist->getWindows();
  set<AWindow*>::iterator iw, fw=liswin.end();

  for( iw=liswin.begin(); iw!=fw; ++iw )
    (*iw)->SetRefreshFlag();
  theAnatomist->Refresh();
}


void PreferencesWindow::enableCursor( bool state )
{
  AWindow::setGlobalHasCursor( state );
  updateWindows();
}


void PreferencesWindow::setCursorShape( const QString & shape )
{
  set<string>	cs = Cursor::cursors();
  set<string>::iterator	ic, ec = cs.end();
  for( ic=cs.begin(); ic!=ec && tr( ic->c_str() ) != shape; ++ic ) {}
  if( ic != ec )
    {
      Cursor::setCurrentCursor( *ic );
      updateWindows();
    }
}


void PreferencesWindow::loadCursor()
{
  QString filt = theAnatomist->objectsFileFilter().c_str();
  QString capt = tr( "Load Anatomist objects" );
  
  QFileDialog	& fd = fileDialog();
  fd.setFilters( filt );
  fd.setCaption( capt );
  fd.setMode( QFileDialog::ExistingFiles );
  if( !fd.exec() )
    return;

  QStringList		filenames = fd.selectedFiles();
  list<QString>		scenars;
  set<AObject *>	loaded;

  for ( QStringList::Iterator it = filenames.begin(); it != filenames.end(); 
	++it )
    {
      LoadObjectCommand *command = new LoadObjectCommand( (*it).utf8().data(), -1, 
                                                          "", true );
      theProcessor->execute( command );
    }

  updateCursorsCombo( _pdat->cursShape );
}


void PreferencesWindow::setCursorColorMode( int state )
{
  if( state < 2 )
    {
      AWindow::setUseDefaultCursorColor( 1 - state );
      updateWindows();
    }
}


void PreferencesWindow::cursorSliderChanged( int val )
{
  _pdat->cursEdit->setText( QString::number( val ) );
  AWindow::setCursorSize( val );
  updateWindows();
}


void PreferencesWindow::cursorEditChanged()
{
  unsigned	val = _pdat->cursEdit->text().toUInt();
  _pdat->cursSlider->setValue( val );
  AWindow::setCursorSize( val );
  updateWindows();
}


void PreferencesWindow::choseCursorColor()
{
  AimsRGB		rc = AWindow::cursorColor();
  QColor	col = QColor( rc.red(), rc.green(), rc.blue() );
  col = QColorDialog::getColor( col );
  if( col.isValid() )
    {
      rc.red() = col.red();
      rc.green() = col.green();
      rc.blue() = col.blue();
      AWindow::setCursorColor( rc );
      _pdat->cursColBtn->setPalette( QPalette( col ) );
      updateWindows();
    }
}


void PreferencesWindow::enableLRDisplay( bool state )
{
  AWindow::setLeftRightDisplay( state );
  updateWindows();
}


void PreferencesWindow::setAxialConvention( int x )
{
  switch( x )
    {
    case 1:
      theAnatomist->config()->setProperty( "axialConvention", 
                                            string( "neuro" ) );
      break;
    default:
      if( theAnatomist->config()->hasProperty( "axialConvention" ) )
        theAnatomist->config()->removeProperty( "axialConvention" );
    }
}


void PreferencesWindow::enableVolInterpolation( bool x )
{
  theAnatomist->config()->setProperty( "volumeInterpolation", (int) x );
}


void PreferencesWindow::enableAutomaticReferential( bool x )
{
  if( x )
    theAnatomist->config()->setProperty( "setAutomaticReferential", (int) 1 );
  else
  {
    if( theAnatomist->config()->hasProperty( "setAutomaticReferential" ) )
      theAnatomist->config()->removeProperty( "setAutomaticReferential" );
    if( theAnatomist->config()->hasProperty( "useSpmOrigin" ) )
      theAnatomist->config()->removeProperty( "useSpmOrigin" );
  }
}


void PreferencesWindow::languageChanged( const QString & lang )
{
  if( lang != tr( "default" ) )
    theAnatomist->config()->setProperty( "language", string( lang.utf8().data() ) );
  else
    theAnatomist->config()->removeProperty( "language" );
}


void PreferencesWindow::htmlBrowserChanged( const QString & brows )
{
  theAnatomist->config()->setProperty( "html_browser", 
					string( brows.utf8().data() ) );
}


void PreferencesWindow::setUserLevel( const QString & x )
{
  bool ok = true;
  unsigned y = x.toUInt( &ok );
  if( !ok )
  {
    QString s = x.lower();
    if( s == "basic" )
      y = 0;
    else if( s == "advanced" )
      y = 1;
    else if( s == "expert" )
      y = 2;
    else if( s == "debugger" )
      y = 3;
    else
    {
      resetUserLevel();
      return;
    }
  }
  QComboBox *au = _pdat->userLevel;
  if( y <= 2 )
  {
    theAnatomist->setUserLevel( y );
    au->setCurrentItem( y );
  }
  else if( y == 3 )
  {
    theAnatomist->setUserLevel( 3 );
    if( au->count() <= 3 || au->text( 3 ) != "Debugger" )
      au->insertItem( "Debugger", 3 );
    au->setCurrentItem( 3 );
  }
  else
  {
    theAnatomist->setUserLevel( y );
  }
}


void PreferencesWindow::resetUserLevel()
{
  setUserLevel( QString::number( theAnatomist->userLevel() ) );
}


void PreferencesWindow::defaultWinSizeChanged()
{
  bool	ok = true;
  float	f = _pdat->winszed->text().toFloat( &ok );
  if( !ok )
    {
      f = 1.5;
      theAnatomist->config()->getProperty( "windowSizeFactor", f );
    }
  _pdat->winszed->blockSignals( true );
  _pdat->winszed->setText( QString::number( f ) );
  theAnatomist->config()->setProperty( "windowSizeFactor", f );
  _pdat->winszed->blockSignals( false );
}


void PreferencesWindow::changeDefObjectsRef()
{
  set<AObject *>	glub;
  ChooseReferentialWindow	crw( glub, 
                                     tr( "Default objects referential" ), 
                                     Qt::WType_Modal );
  if( crw.exec() )
    {
      setRefColor( crw.selectedReferential(), _pdat->defobjref );
      theAnatomist->getControlWindow()
        ->setDefaultObjectsReferential( crw.selectedReferential() );
    }
}


void PreferencesWindow::changeDefWindowsRef()
{
  set<AObject *>		glub;
  ChooseReferentialWindow	crw( glub, 
                                     tr( "Default windows referential" ), 
                                     Qt::WType_Modal );
  if( crw.exec() )
    {
      setRefColor( crw.selectedReferential(), _pdat->defwinref );
      theAnatomist->getControlWindow()
        ->setDefaultWindowsReferential( crw.selectedReferential() );
    }
}


void PreferencesWindow::browserAttributeLenChanged()
{
  bool  ok = true;
  int len = _pdat->browsattlen->text().toInt( &ok );
  if( !ok )
  {
    len = 0;
    theAnatomist->config()->getProperty( "clipBrowserValues", len );
  }
  _pdat->browsattlen->blockSignals( true );
  _pdat->browsattlen->setText( QString::number( len ) );
  theAnatomist->config()->setProperty( "clipBrowserValues", len );
  _pdat->browsattlen->blockSignals( false );
}


void PreferencesWindow::enableDisplayCursorPosition( bool x )
{
  if( x )
  {
    if( theAnatomist->config()->hasProperty( "displayCursorPosition" ) )
      theAnatomist->config()->removeProperty( "displayCursorPosition" );
  }
  else
  {
    theAnatomist->config()->setProperty( "displayCursorPosition", int(0) );
  }
}


void PreferencesWindow::setMaxTextures( const QString & mt )
{
  GlobalConfiguration   *cfg = theAnatomist->config();

  int imt = -1;
  try
  {
    Object  x = cfg->getProperty( "maxTextureUnitsUsed" );
    if( !x.isNull() )
      imt = (int) x->getScalar();
  }
  catch( ... )
  {
  }
  cout << "setMaxTextures: " << mt.lower().utf8().data() << " / " << tr( "Unlimited" ).utf8().data() << endl;
  if( mt.lower() == tr( "Unlimited" ).lower() )
    imt = -1;
  else
  {
    bool ok = false;
    int imt2 = mt.toInt( &ok );
    if( ok )
      imt = imt2;
  }
  
  cfg->setProperty( "maxTextureUnitsUsed", imt );

  // As we are not sure to be in valid OpenGL context
  // we only invalidate current number of textures used.
  // Correct texture number will be processed during next 
  // call to updateTextureUnits in a valid OpenGL context
  GLCaps::mustRefresh() = true;

  QString mt2;
  if( imt < 0 )
  {
    mt2 = tr( "Unlimited" );
    imt = -1;
  }
  else
    mt2 = QString::number( imt );
  if( mt2 != mt )
    _pdat->texmax->setCurrentItem( imt + 1 );
}


void PreferencesWindow::enableOpenGLSelection( bool x )
{
  if( x )
  {
    if( theAnatomist->config()->hasProperty( "disableOpenGLSelection" ) )
      theAnatomist->config()->removeProperty( "disableOpenGLSelection" );
  }
  else
  {
    theAnatomist->config()->setProperty( "disableOpenGLSelection", int(1) );
  }
}


void PreferencesWindow::commonScannerBasedReferential( bool x )
{
  if( !x )
  {
    if( theAnatomist->config()->hasProperty(
      "commonScannerBasedReferential" ) )
      theAnatomist->config()->removeProperty(
        "commonScannerBasedReferential" );
  }
  else
  {
    theAnatomist->config()->setProperty( "commonScannerBasedReferential",
                                         int(1) );
  }
}


