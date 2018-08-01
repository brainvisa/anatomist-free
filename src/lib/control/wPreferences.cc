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
#include <anatomist/surface/Shader.h>
#include <anatomist/surface/glcomponent.h>
#include <qtabbar.h>
#include <QGLShaderProgram>
#include <qgroupbox.h>
#include <qbuttongroup.h>
#include <qlayout.h>
#include <qcheckbox.h>
#include <qwidgetaction.h>
#include <qspinbox.h>
#include <qmenu.h>
#include <qradiobutton.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qcolordialog.h>
#include <qcombobox.h>
#include <qtooltip.h>
#include <qradiobutton.h>
#include <qvalidator.h>
#include <qfiledialog.h>
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
      defobjref( 0 ), defwinref( 0 ), browsattlen( 0 ), texmax( 0 ),
      graphicsview( 0 ) {}

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
  QCheckBox             *graphicsview;
  QPushButton           *winbackgroundbt;
  QLineEdit             *limitpoly;
};


namespace
{

  void setRefColor( Referential* ref, QPushButton* pb )
  {
    if( ref )
    {
      QPixmap     pix( 30, 20 );
      AimsRGB	col = ref->Color();
      pix.fill( QColor( col.red(), col.green(), col.blue() ) );
      pb->setIcon( pix );
    }
    else
      pb->setIcon( QPixmap() );
  }


  void updateCursorsCombo( QComboBox* cb )
  {
    set<string>			cs = Cursor::cursors();
    set<string>::iterator	ic, ec = cs.end();
    int				cur = 0, ci = 0;

    cb->clear();
    for( ic=cs.begin(); ic!=ec; ++ic, ++ci )
      {
        cb->addItem( PreferencesWindow::tr( ic->c_str() ) );
        if( ic->c_str() == Cursor::currentCursorName() )
          cur = ci;
      }
    cb->setCurrentIndex( cur );
  }

}


PreferencesWindow::PreferencesWindow()
  : QWidget( theAnatomist->getQWidgetAncestor(), Qt::Window ), 
    _pdat( new Private )
{
  setWindowTitle( tr( "Anatomist global settings" ) );
  setAttribute( Qt::WA_DeleteOnClose );
  setObjectName( "prefWin" );

  QVBoxLayout	*mainlay = new QVBoxLayout( this );
  mainlay->setMargin( 5 );
  mainlay->setSpacing( 10 );
  QTabBar	*tbar = new QTabBar( this );

  _pdat->tabnum[ tbar->addTab( tr( "Application" ) )    ] = 0;
  _pdat->tabnum[ tbar->addTab( tr( "Linked cursor" ) )  ] = 1;
  _pdat->tabnum[ tbar->addTab( tr( "Windows" ) )        ] = 2;
  _pdat->tabnum[ tbar->addTab( tr( "Control window" ) ) ] = 3;
  _pdat->tabnum[ tbar->addTab( tr( "Volumes" ) )        ] = 4;
  _pdat->tabnum[ tbar->addTab( tr( "OpenGL" ) )         ] = 5;

  GlobalConfiguration	*cfg = theAnatomist->config();

  //	Application tab

  QGroupBox *app = new QGroupBox( this );
  QVBoxLayout *vlay = new QVBoxLayout( app );
  _pdat->tabs.push_back( app );
  QWidget *applang = new QWidget( app );
  vlay->addWidget( applang );
  QHBoxLayout *hlay = new QHBoxLayout( applang );
  hlay->setSpacing( 5 );
  hlay->setMargin( 0 );
  QLabel	*l = new QLabel( tr( "Language :" ), applang );
  hlay->addWidget( l );
  l->setToolTip( tr( "Language options will apply the next time you start " 
                     "Anatomist (save preferences !)" ) );
  QComboBox	*langbox = new QComboBox( applang );
  hlay->addWidget( langbox );
  string	lang;
  cfg->getProperty( "language", lang );
  langbox->addItem( tr( "default" ) );

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
      langbox->addItem( (*id).c_str() );
      if( lang == *id )
	langind = i+1;
    }
  langbox->setCurrentIndex( langind );

  QWidget *appbrow = new QWidget( app );
  vlay->addWidget( appbrow );
  hlay = new QHBoxLayout( appbrow );
  hlay->setSpacing( 5 );
  hlay->setMargin( 0 );
  hlay->addWidget( new QLabel( tr( "HTML browser command line :" ), 
                               appbrow ) );
  QComboBox	*htmlbox = new QComboBox( appbrow );
  hlay->addWidget( htmlbox );
  htmlbox->setEditable( true );
  htmlbox->setDuplicatesEnabled( false );
  htmlbox->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, 
                                       QSizePolicy::Fixed ) );
  // htmlbox->setMaximumWidth( 400 );
  string	htmltxt;
  if( cfg->getProperty( "html_browser", htmltxt ) )
    htmlbox->addItem( htmltxt.c_str() );
#ifdef __APPLE__
  htmlbox->addItem( "/Applications/Safari.app/Contents/MacOS/Safari %1" );
  htmlbox->addItem( "/Applications/Internet\\ Explorer.app/Contents/" 
                    "MacOS/Internet\\ Explorer %1" );
#else
#ifdef _WIN32
  htmlbox->addItem( "explorer.exe %1" );
#endif
#endif
  htmlbox->addItem( "konqueror %1 &" );
  htmlbox->addItem( "firefox %1 &" );
  htmlbox->addItem( "mozilla %1 &" );
  htmlbox->addItem( "netscape -noraise -remote openBrowser\\(file:%1\\) "
      "2> /dev/null || netscape -no-about-splash file:%1 &" );
  htmlbox->addItem( "kfmclient openURL=%1" );
  htmlbox->setCurrentIndex( 0 );

  QWidget *ulev = new QWidget( app );
  vlay->addWidget( ulev );
  QHBoxLayout *ulevl = new QHBoxLayout( ulev );
  ulevl->setSpacing( 5 );
  ulevl->setContentsMargins( 0, 0, 0, 0 );
  ulev->setLayout( ulevl );
  QLabel *ull = new QLabel( tr( "User level :" ), ulev );
  ull->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
  ulevl->addWidget( ull );
  QComboBox *au = new QComboBox( ulev );
  ulevl->addWidget( au );
  au->setEditable( true );
  _pdat->userLevel = au;
  au->addItem( "Basic" );
  au->addItem( "Advanced" );
  au->addItem( "Expert" );
  au->setEditable( true );
  au->setInsertPolicy( QComboBox::NoInsert );
  au->setValidator( new QRegExpValidator( QRegExp(
    "\\d*|Basic|Advanced|Expert|Debugger", Qt::CaseInsensitive ), au ) );
  resetUserLevel();
  connect( au, SIGNAL( activated( const QString & ) ), this,
    SLOT( setUserLevel( const QString & ) ) );

  QWidget *confirm = new QWidget( app );
  vlay->addWidget( confirm );
  QHBoxLayout *confl = new QHBoxLayout( confirm );
  confl->setSpacing( 5 );
  confl->setContentsMargins( 0, 0, 0, 0 );
  confirm->setLayout( confl );
  QCheckBox *confla = new QCheckBox( tr( "Confirm before quitting" ),
                                     confirm );
  confl->addWidget( confla );
  bool confbq = true;
  try
  {
    Object oask = theAnatomist->config()->getProperty( "confirmBeforeQuit" );
    if( !oask.isNull() )
      confbq = (bool) oask->getScalar();
  }
  catch( ... )
  {
  }
  confla->setChecked( confbq );
  connect( confla, SIGNAL( stateChanged( int ) ), this,
           SLOT( confirmBeforeQuitChanged( int ) ) );

  QGroupBox	*refs = new QGroupBox( tr( "Default referentials" ), app );
  vlay->addWidget( refs );
  vlay->addStretch( 1 );
  vlay = new QVBoxLayout( refs );
  QWidget *refg = new QWidget( refs );
  vlay->addWidget( refg );
  QGridLayout *glay = new QGridLayout( refg );
  glay->setMargin( 0 );
  glay->addWidget(
    new QLabel( tr( "Default referential for loaded objects" ), refg ), 0, 0 );
  _pdat->defobjref = new QPushButton( refg );
  glay->addWidget( _pdat->defobjref, 0, 1 );
  setRefColor( theAnatomist->getControlWindow()->defaultObjectsReferential(), 
               _pdat->defobjref );
  _pdat->defobjref->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, 
                                                QSizePolicy::Fixed ) );
  glay->addWidget(
    new QLabel( tr( "Default referential for new windows" ), refg ), 1, 0 );
  _pdat->defwinref = new QPushButton( refg );
  glay->addWidget( _pdat->defwinref, 1, 1 );
  setRefColor( theAnatomist->getControlWindow()->defaultWindowsReferential(), 
               _pdat->defwinref );
  _pdat->defwinref->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, 
                                                QSizePolicy::Fixed ) );
  connect( _pdat->defobjref, SIGNAL( clicked() ), this, 
           SLOT( changeDefObjectsRef() ) );
  connect( _pdat->defwinref, SIGNAL( clicked() ), this, 
           SLOT( changeDefWindowsRef() ) );

  //	Linked cursor tab

  QWidget *lkcur = new QWidget( this );
  vlay = new QVBoxLayout( lkcur );
  vlay->setMargin( 0 );
  vlay->setSpacing( 5 );
  lkcur->hide();
  _pdat->tabs.push_back( lkcur );
  QCheckBox	*cursEnable 
    = new QCheckBox( tr( "Display linked cursor" ), lkcur );
  vlay->addWidget( cursEnable );
  cursEnable->setChecked( AWindow::hasGlobalCursor() );
  QGroupBox	*cursShape = 
    new QGroupBox( tr( "Cursor shape :" ), lkcur );
  vlay->addWidget( cursShape );
  hlay = new QHBoxLayout( cursShape );
  cursShape->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, 
                                         QSizePolicy::Fixed ) );
  _pdat->cursShape = new QComboBox( cursShape );
  hlay->addWidget( _pdat->cursShape );
  QPushButton	*cursShapeBtn = new QPushButton( tr( "Load..." ), cursShape );
  hlay->addWidget( cursShapeBtn );
  cursShapeBtn->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, 
                                            QSizePolicy::Fixed ) );
  updateCursorsCombo( _pdat->cursShape );

  QWidget *cursSzBox = new QWidget( lkcur );
  hlay = new QHBoxLayout( cursSzBox );
  vlay->addWidget( cursSzBox );
  hlay->setSpacing( 10 );
  hlay->setMargin( 0 );
  hlay->addWidget( new QLabel( tr( "Size :" ), cursSzBox ) );
  _pdat->cursEdit 
    = new QLineEdit( QString::number( AWindow::cursorSize() ), cursSzBox );
  hlay->addWidget( _pdat->cursEdit );
  _pdat->cursEdit->setSizePolicy( QSizePolicy( QSizePolicy::Preferred, 
                                               QSizePolicy::Fixed ) );
  _pdat->cursSlider = new QSlider( Qt::Horizontal, lkcur );
  vlay->addWidget( _pdat->cursSlider );
  _pdat->cursSlider->setMinimum( 0 );
  _pdat->cursSlider->setMaximumWidth( 256 );
  _pdat->cursSlider->setPageStep( 1 );
  _pdat->cursSlider->setValue( AWindow::cursorSize() );
  QGroupBox *cursCol = new QGroupBox( tr( "Cursor color :" ), lkcur );
  vlay->addWidget( cursCol );
  QVBoxLayout *vlay2 = new QVBoxLayout( cursCol );
  QButtonGroup *bgp = new QButtonGroup( cursCol );
  bgp->setExclusive( true );
  QRadioButton *rb = new QRadioButton( tr( "Use default color" ), cursCol );
  vlay2->addWidget( rb );
  rb->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, 
                                  QSizePolicy::Fixed ) );
  bgp->addButton( rb, 0 );
  rb = new QRadioButton( tr( "Custom color :" ), cursCol );
  vlay2->addWidget( rb );
  rb->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, 
                                  QSizePolicy::Fixed ) );
  bgp->addButton( rb, 1 );
  bgp->button( 1 - AWindow::useDefaultCursorColor() )->setChecked( true );
  _pdat->cursColBtn = new QPushButton( cursCol );
  vlay2->addWidget( _pdat->cursColBtn );
  AimsRGB	col = AWindow::cursorColor();
  _pdat->cursColBtn->setPalette( QPalette( QColor( col.red(), col.green(), 
                                                   col.blue() ) ) );
  _pdat->cursColBtn->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, 
                                                 QSizePolicy::Fixed ) );
  vlay->addStretch( 1 );

  //	windows tab

  QWidget *winbox = new QWidget( this );
  _pdat->tabs.push_back( winbox );
  vlay = new QVBoxLayout( winbox );
  vlay->setMargin( 0 );
  winbox->hide();
  QGroupBox	*flip 
    = new QGroupBox( tr( "3D views" ), winbox );
  vlay->addWidget( flip );
  vlay2 = new QVBoxLayout( flip );
  QGroupBox  *flipbx = new QGroupBox( tr( "Axial/coronal slices orientation" ),
                                      flip );
  vlay2->addWidget( flipbx );
  QVBoxLayout *vlay3 = new QVBoxLayout( flipbx );
  QButtonGroup *flipg = new QButtonGroup( flipbx );
  flipg->setExclusive( true );
  rb = new QRadioButton( tr( "Radioligical convention (seen from bottom, " 
                             "L/R flipped)" ), flipbx );
  vlay3->addWidget( rb );
  flipg->addButton( rb, 0 );
  rb = new QRadioButton( tr( "Neurological convention (seen from top)" ), 
                         flipbx );
  vlay3->addWidget( rb );
  flipg->addButton( rb, 1 );
  int     btn = 0;
  string  axconv;
  cfg->getProperty( "axialConvention", axconv );
  if( axconv == "neuro" )
    btn = 1;
  flipg->button( btn )->setChecked( true );

  QHBoxLayout *hlay10 = new QHBoxLayout();
  vlay2->addLayout(hlay10);

  QCheckBox	*flipDisplay 
    = new QCheckBox( tr( "Display L/R in corners" ), flip );
  hlay10->addWidget( flipDisplay );
  flipDisplay->setChecked( AWindow::leftRightDisplay() );

  hlay10->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Minimum));
  const vector<string> defaultAnnotations = AWindow::displayedAnnotations();
  string annotation_array[] = {"Anterior", "Inferior", "Left", "Posterior", "Right", "Superior"};
  const vector<string> annotations (annotation_array, annotation_array + sizeof(annotation_array)/sizeof(string));
  QMenu* menu = new QMenu();
  for (vector<string>::const_iterator it = annotations.begin(); it != annotations.end(); it++)
  {
	  QCheckBox* checkBox = new QCheckBox(QString::fromStdString(*it), menu);
	  bool checked = (std::find(defaultAnnotations.begin(), defaultAnnotations.end(), *it) != defaultAnnotations.end());
	  checkBox->setChecked(checked);
	  QWidgetAction* checkableAction = new QWidgetAction(menu);
	  checkableAction->setDefaultWidget(checkBox);
	  menu->addAction(checkableAction);
	  connect(checkBox, SIGNAL(stateChanged(int)), this,
			  	  	    SLOT(displayedAnnotationChanged(int)));
  }
  QPushButton* pushBt = new QPushButton("Displayed annotations");
  pushBt->setMenu(menu);
  hlay10->addWidget(pushBt);

  hlay10->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Minimum));
  hlay10->addWidget(new QLabel("Annotation size: "));
  QSpinBox* leftRightDisplaySizeSb = new QSpinBox(flip);
  hlay10->addWidget(leftRightDisplaySizeSb);
  leftRightDisplaySizeSb->setValue(AWindow::leftRightDisplaySize());
  hlay10->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Minimum));

  _pdat->disppos = new QCheckBox( tr( "Display cursor position by default" ),
                                  flip );
  vlay2->addWidget( _pdat->disppos );
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
  _pdat->graphicsview = new QCheckBox(
    tr( "Use graphics overlay on OpenGL rendering" ), flip );
  vlay2->addWidget( _pdat->graphicsview );
#if defined( __APPLE__ )
  int use_graphicsview = 0;
#else
  int use_graphicsview = 1;
#endif
  try
  {
    Object  x = cfg->getProperty( "windowsUseGraphicsView" );
    if( !x.isNull() )
      use_graphicsview = (int) x->getScalar();
  }
  catch( ... )
  {
  }
  _pdat->graphicsview->setChecked( use_graphicsview );

  hlay = new QHBoxLayout( winbox );
  vlay2->addLayout( hlay );
  _pdat->winbackgroundbt = new QPushButton( "...", winbox );
  col = AimsRGB( 255, 255, 255 );
  try
  {
    Object oc = theAnatomist->config()->getProperty( "windowBackground" );
    if( !oc.isNull() )
    {
      Object oi = oc->objectIterator();
      if( oi->isValid() )
      {
        col[0] = uint8_t( oi->currentValue()->getScalar() * 255.99 );
        oi->next();
        if( oi->isValid() )
        {
          col[1] = uint8_t( oi->currentValue()->getScalar() * 255.99 );
          oi->next();
          if( oi->isValid() )
            col[2] = uint8_t( oi->currentValue()->getScalar() * 255.99 );
        }
      }
    }
  }
  catch( ... )
  {
  }
  _pdat->winbackgroundbt->setPalette( QPalette( QColor( col.red(), col.green(),
                                                        col.blue() ) ) );
  _pdat->winbackgroundbt->setSizePolicy( QSizePolicy( QSizePolicy::Fixed,
                                                      QSizePolicy::Fixed ) );
  hlay->addWidget( _pdat->winbackgroundbt );
  hlay->addWidget( new QLabel( tr( "Default background color" ) ) );

  QGroupBox	*winsz 
    = new QGroupBox( tr( "Default windows size" ), winbox );
  vlay->addWidget( winsz );
  hlay = new QHBoxLayout( winsz );
  _pdat->winszed = new QLineEdit( winsz );
  hlay->addWidget( _pdat->winszed );
  _pdat->winszed->setSizePolicy( QSizePolicy( QSizePolicy::Preferred, 
                                              QSizePolicy::Fixed ) );
  hlay->addWidget( new QLabel( tr( "pixel / mm" ), winsz ) );
  float	wf = 1.5;
  theAnatomist->config()->getProperty( "windowSizeFactor", wf );
  _pdat->winszed->setText( QString::number( wf ) );
  _pdat->winszed->setValidator( new QDoubleValidator( 0.001, 1e6, 3, 
                                                      _pdat->winszed ) );
  QGroupBox *brows = new QGroupBox( tr( "Browsers" ), winbox );
  vlay->addWidget( brows );
  vlay2 = new QVBoxLayout( brows );
  QWidget *brattlen = new QWidget( brows );
  vlay2->addWidget( brattlen );
  hlay = new QHBoxLayout( brattlen );
  hlay->setMargin( 0 );
  hlay->addWidget( new QLabel( tr( "limit browsers attribute values to: " ), 
                               brattlen ) );
  _pdat->browsattlen = new QLineEdit( brattlen );
  hlay->addWidget( _pdat->browsattlen );
  hlay->addWidget( new QLabel( tr( " characters" ), brattlen ) );
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

  vlay->addStretch( 1 );

  //	control window tab

  QGroupBox	*cwin = new QGroupBox( this );
  vlay2 = new QVBoxLayout( cwin );
  cwin->hide();
  _pdat->tabs.push_back( cwin );
  QCheckBox	*cwlogo = new QCheckBox( tr( "Display nice logo" ), cwin );
  vlay2->addWidget( cwlogo );
  ControlWindow	*cw = theAnatomist->getControlWindow();
  cwlogo->setChecked( cw->logoEnabled() );
  vlay2->addStretch( 1 );

  //	Volumes tab

  QGroupBox	*tvol = new QGroupBox( this );
  vlay = new QVBoxLayout( tvol );
  tvol->hide();
  _pdat->tabs.push_back( tvol );
  QCheckBox	*tvint = new QCheckBox( tr( "Interpolation on volumes when "
                                            "changing referential" ), tvol );
  vlay->addWidget( tvint );
  int	tvich = 1;
  cfg->getProperty( "volumeInterpolation", tvich );
  tvint->setChecked( tvich );
  QCheckBox	*tvspm 
    = new QCheckBox( tr( "Use referential / transformations "
      "information found in objects headers (SPM, NIFTI...)" ), tvol );
  vlay->addWidget( tvspm );
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
  vlay->addWidget( refscan );
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

  vlay->addStretch( 1 );

  //    OpenGL tab

  QGroupBox    *tgl = new QGroupBox( this );
  vlay = new QVBoxLayout( tgl );
  tgl->hide();
  _pdat->tabs.push_back( tgl );

  QWidget *btexmax = new QWidget( tgl );
  vlay->addWidget( btexmax );
  hlay = new QHBoxLayout( btexmax );
  hlay->setMargin( 0 );
  hlay->addWidget( new QLabel( tr( "limit number of textures: " ), btexmax ) );
  QComboBox *texmax = new QComboBox( btexmax );
  hlay->addWidget( texmax );
  _pdat->texmax = texmax;
  texmax->setDuplicatesEnabled( false );
  texmax->setSizePolicy( QSizePolicy( QSizePolicy::Expanding,
                                      QSizePolicy::Fixed ) );
  texmax->addItem( tr( "Unlimited" ) );
  texmax->addItem( "0" );
  texmax->addItem( "1" );
  texmax->addItem( "2" );
  texmax->addItem( "3" );
  texmax->addItem( "4" );
  texmax->addItem( "5" );
  texmax->addItem( "6" );
  texmax->addItem( "7" );
  texmax->addItem( "8" );
  texmax->setEditable( true );
  texmax->setValidator( new QRegExpValidator( QRegExp(
    "\\d*|" + tr( "Unlimited" ) + "|-1", Qt::CaseInsensitive ), texmax ) );
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
  texmax->setCurrentIndex( ntexmax + 1 );
  btexmax->setToolTip( tr( "Try this option if you encounter OpenGL rendering "
  "problems.\n"
  "Such problems have been seen on Windows machines, where rendering was not "
  "performed at all\n"
  "if more than 3 texture units were enabled (even on non-tetured objects).\n"
  "Use 'Unlimited' if rendering is OK."
  ) );

  QCheckBox *glselect = new QCheckBox( tr( "Use OpenGL selection" ), tgl );
  vlay->addWidget( glselect );
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

  // shaders
  QCheckBox *glshader = new QCheckBox( 
    tr( "Enable OpenGL shaders (GLSL 1.2 support needed)" ), tgl );
  vlay->addWidget( glshader );
  bool	support_glshader = Shader::isSupported();
  bool	useglshader;

  if (not support_glshader)
  {
    glshader->setEnabled(false);
    useglshader = false;
  }
  else useglshader = Shader::isActivated();

  glshader->setChecked( useglshader);
  glshader->setToolTip( tr( "Enable OpenGL shaders." ) );

  QCheckBox *glshaderbydefault = new QCheckBox( 
    tr( "Use shader-based OpenGL pipeline (lighting/shading model) by default" 
      ), tgl );
  vlay->addWidget( glshaderbydefault );
  if (not support_glshader)
    glshaderbydefault->setEnabled(false);
  glshaderbydefault->setChecked(Shader::isUsedByDefault());

  QHBoxLayout *polylay = new QHBoxLayout;
  vlay->addLayout( polylay );
  QLabel *lpl = new QLabel( tr( "Limit polygons / object:" ) );
  polylay->addWidget( lpl );
  lpl->setToolTip( tr( "Setting such a limit allows to work around "
  "performance problems with very large meshes and limited 3D hardware. "
  "Rendering will be incomplete in such a case, but this may avoid complete "
  "computer hangups.\nIt is especially useful when displaying large fiber "
  "tracts sets, where cutting off a part of fibers will, in most cases, not "
  "really affect the golbal sight of fibers.\n0 here means no limitation." ) );
  _pdat->limitpoly = new QLineEdit( tgl );
  _pdat->limitpoly->setValidator(
    new QIntValidator( 0, 2000000000, _pdat->limitpoly ) );
  polylay->addWidget( _pdat->limitpoly );
  unsigned long mpoly = 0;
  try
  {
    Object x = cfg->getProperty( "maxPolygonsPerObject" );
    if( !x.isNull() )
      mpoly = (unsigned long) x->getScalar();
  }
  catch( ... )
  {
  }
  _pdat->limitpoly->setText( QString::number( mpoly ) );

  vlay->addStretch( 1 );

  //	top-level widget setting

  mainlay->addWidget( tbar );
  mainlay->addWidget( app );
  mainlay->addWidget( lkcur );
  mainlay->addWidget( winbox );
  mainlay->addWidget( cwin );
  mainlay->addWidget( tvol );
  mainlay->addWidget( tgl );

  connect( tbar, SIGNAL( currentChanged( int ) ), this, SLOT( enableTab( int ) ) );
  connect( htmlbox, SIGNAL( activated( const QString & ) ), this, 
	   SLOT( htmlBrowserChanged( const QString & ) ) );
  connect( langbox, SIGNAL( activated( const QString & ) ), this, 
	   SLOT( languageChanged( const QString & ) ) );
  connect( cursEnable, SIGNAL( toggled( bool ) ), this, 
	   SLOT( enableCursor( bool ) ) );
  connect( _pdat->cursShape, SIGNAL( activated( const QString & ) ), this, 
	   SLOT( setCursorShape( const QString & ) ) );
  connect( cursShapeBtn, SIGNAL( clicked() ), this, SLOT( loadCursor() ) );
  connect( bgp, SIGNAL( buttonClicked( int ) ), this, 
	   SLOT( setCursorColorMode( int ) ) );
  connect( _pdat->cursEdit, SIGNAL( returnPressed() ), this, 
	   SLOT( cursorEditChanged() ) );
  connect( _pdat->cursSlider, SIGNAL( valueChanged( int ) ), this, 
	   SLOT( cursorSliderChanged( int ) ) );
  connect( _pdat->cursColBtn, SIGNAL( clicked() ), this, 
	   SLOT( choseCursorColor() ) );

  connect( flipDisplay, SIGNAL( toggled( bool ) ), this, 
	   SLOT( enableLRDisplay( bool ) ) );
  connect(leftRightDisplaySizeSb, SIGNAL(valueChanged(int)), this,
  	      	  	  	  	  	  	  SLOT(leftRightDisplaySizeChanged(int)));
  connect( flipg, SIGNAL( buttonClicked( int ) ), this, 
	   SLOT( setAxialConvention( int ) ) );
  connect( _pdat->winszed, SIGNAL( returnPressed() ), this, 
           SLOT( defaultWinSizeChanged() ) );
  connect( _pdat->disppos, SIGNAL( toggled( bool ) ), this,
           SLOT( enableDisplayCursorPosition( bool ) ) );
  connect( _pdat->graphicsview, SIGNAL( toggled( bool ) ), this,
           SLOT( enableGraphicsView( bool ) ) );
  connect( _pdat->winbackgroundbt, SIGNAL( clicked() ),
           this, SLOT( changeWindowBackground() ) );
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
  connect( glshader, SIGNAL( toggled( bool ) ), this,
           SLOT( enableOpenGLShader( bool ) ) );
  connect( glshaderbydefault, SIGNAL( toggled( bool ) ), this,
           SLOT( shadersByDefault( bool ) ) );
  connect( _pdat->limitpoly, SIGNAL( editingFinished() ),
           this, SLOT( maxPolygonsPerObjectChanged() ) );
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
  theAnatomist->config()->update();
}


void PreferencesWindow::loadCursor()
{
  QString filt = theAnatomist->objectsFileFilter().c_str();
  QString capt = tr( "Load Anatomist objects" );

  QFileDialog	& fd = fileDialog();
  fd.setNameFilter( filt );
  fd.setWindowTitle( capt );
  fd.setFileMode( QFileDialog::ExistingFiles );
  fd.setAcceptMode( QFileDialog::AcceptOpen );
  if( !fd.exec() )
    return;

  QStringList		filenames = fd.selectedFiles();
  list<QString>		scenars;
  set<AObject *>	loaded;

  for ( QStringList::Iterator it = filenames.begin(); it != filenames.end();
        ++it )
  {
    LoadObjectCommand *command = new LoadObjectCommand( (*it).toStdString(),
                                                        -1, "", true );
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
    theAnatomist->config()->update();
  }
}


void PreferencesWindow::cursorSliderChanged( int val )
{
  _pdat->cursEdit->setText( QString::number( val ) );
  AWindow::setCursorSize( val );
  updateWindows();
  theAnatomist->config()->update();
}


void PreferencesWindow::cursorEditChanged()
{
  unsigned	val = _pdat->cursEdit->text().toUInt();
  _pdat->cursSlider->setValue( val );
  AWindow::setCursorSize( val );
  updateWindows();
  theAnatomist->config()->update();
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
    theAnatomist->config()->update();
  }
}


void PreferencesWindow::enableLRDisplay( bool state )
{
  AWindow::setLeftRightDisplay( state );

  updateWindows();
  theAnatomist->config()->update();
}

void PreferencesWindow::leftRightDisplaySizeChanged(int size)
{
	AWindow::setLeftRightDisplaySize(size);
	updateWindows();
	theAnatomist->config()->update();
}

void PreferencesWindow::displayedAnnotationChanged(int)
{
	QCheckBox* checkbox = dynamic_cast<QCheckBox*>(sender());
	if (!checkbox)
	{
		return;
	}
	vector<string> annotations = AWindow::displayedAnnotations();
	const string text = checkbox->text().toStdString();
	if (checkbox->isChecked())
	{
		annotations.push_back(text);
	}
	else
	{
		annotations.erase(std::remove(annotations.begin(), annotations.end(),
									  text), annotations.end());
	}
	std::sort(annotations.begin(), annotations.end());
	AWindow::setDisplayedAnnotations(annotations);
	updateWindows();
	theAnatomist->config()->update();
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
    theAnatomist->config()->setProperty( "language", string( lang.toStdString() ) );
  else
    theAnatomist->config()->removeProperty( "language" );
}


void PreferencesWindow::htmlBrowserChanged( const QString & brows )
{
  theAnatomist->config()->setProperty( "html_browser", 
					string( brows.toStdString() ) );
}


void PreferencesWindow::setUserLevel( const QString & x )
{
  bool ok = true;
  unsigned y = x.toUInt( &ok );
  if( !ok )
  {
    QString s = x.toLower();
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
    au->setCurrentIndex( y );
  }
  else if( y == 3 )
  {
    theAnatomist->setUserLevel( 3 );
    if( au->count() <= 3 || au->itemText( 3 ) != "Debugger" )
      au->insertItem( 3, "Debugger" );
    au->setCurrentIndex( 3 );
  }
  else
  {
    theAnatomist->setUserLevel( y );
  }
  theAnatomist->config()->update();
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
  ChooseReferentialWindow
    crw( glub,
         tr( "Default objects referential" ).toStdString().c_str() );
  crw.setModal( true );
  if( crw.exec() )
  {
    cout << "OK\n";
    cout << crw.selectedReferential() << ", " << _pdat->defobjref << endl;
    setRefColor( crw.selectedReferential(), _pdat->defobjref );
    theAnatomist->getControlWindow()
      ->setDefaultObjectsReferential( crw.selectedReferential() );
  }
}


void PreferencesWindow::changeDefWindowsRef()
{
  set<AObject *>		glub;
  ChooseReferentialWindow
    crw( glub,
         tr( "Default windows referential" ).toStdString().c_str() );
  crw.setModal( true );
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


void PreferencesWindow::enableGraphicsView( bool x )
{
  theAnatomist->config()->setProperty( "windowsUseGraphicsView", int(x) );
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
  if( mt.toLower() == tr( "Unlimited" ).toLower() )
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
    _pdat->texmax->setCurrentIndex( imt + 1 );
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


void PreferencesWindow::enableOpenGLShader( bool x )
{
  if( x )
  {
    if( theAnatomist->config()->hasProperty( "disableOpenGLShader" ) )
      theAnatomist->config()->removeProperty( "disableOpenGLShader" );
    Shader::enable_all();
  }
  else
  {
    theAnatomist->config()->setProperty( "disableOpenGLShader", int(1) );
    Shader::disable_all();
  }
}


void PreferencesWindow::shadersByDefault( bool x )
{
  if( x )
  {
    theAnatomist->config()->setProperty( "shadersByDefault", int(1) );
  }
  else
  {
    if( theAnatomist->config()->hasProperty( "shadersByDefault" ) )
      theAnatomist->config()->removeProperty( "shadersByDefault" );
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


void PreferencesWindow::confirmBeforeQuitChanged( int x )
{
  cout << "confirmBeforeQuitChanged: " << x << endl;
  if( x )
  {
    if( theAnatomist->config()->hasProperty( "confirmBeforeQuit" ) )
      theAnatomist->config()->removeProperty( "confirmBeforeQuit" );
  }
  else
    theAnatomist->config()->setProperty( "confirmBeforeQuit", int(0) );
}


void PreferencesWindow::changeWindowBackground()
{
  QColor        col = QColor( 255, 255, 255 );
  try
  {
    Object oc = theAnatomist->config()->getProperty( "windowBackground" );
    if( !oc.isNull() )
    {
      Object oi = oc->objectIterator();
      if( oi->isValid() )
      {
        col.setRed( uint8_t( oi->currentValue()->getScalar() * 255.99 ) );
        oi->next();
        if( oi->isValid() )
        {
          col.setGreen( uint8_t( oi->currentValue()->getScalar() * 255.99 ) );
          oi->next();
          if( oi->isValid() )
            col.setBlue( uint8_t( oi->currentValue()->getScalar() * 255.99 ) );
        }
      }
    }
  }
  catch( ... )
  {
  }
  col = QColorDialog::getColor( col );
  if( col.isValid() )
  {
    vector<float> bgcol( 4, 1. );
    bgcol[0] = col.red() / 255.;
    bgcol[1] = col.green() / 255.;
    bgcol[2] = col.blue() / 255.;
    _pdat->winbackgroundbt->setPalette( QPalette( col ) );
    theAnatomist->config()->setProperty( "windowBackground", bgcol );
  }
}


void PreferencesWindow::maxPolygonsPerObjectChanged()
{
  bool ok = false;
  int npoly = _pdat->limitpoly->text().toInt( &ok );
  if( ok )
  {
    theAnatomist->config()->setProperty( "maxPolygonsPerObject", npoly );
    GLComponent::glSetGlobalMaxNumDisplayedPolygons( npoly );
  }
}

