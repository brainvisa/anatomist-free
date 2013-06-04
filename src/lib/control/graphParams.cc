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


#include <anatomist/control/graphParams.h>

#include <anatomist/control/toolTips-qt.h>
#include <anatomist/graph/GraphObject.h>
#include <anatomist/graph/Graph.h>
#include <anatomist/hierarchy/hierarchy.h>
#include <graph/tree/tree.h>
#include <anatomist/selection/selectFactory.h>
#include <anatomist/dialogs/colorDialog.h>

#include <qlayout.h>
#include <qbuttongroup.h>
#include <qgroupbox.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qpixmap.h>


using namespace anatomist;
using namespace std;

GraphParams::GraphParams()
  : colorsActive( true ), attribute( "name" ), toolTips( true ),
    saveMode( 1 ), saveOnlyModified( true ), autoSaveDir( true ),
    loadRelations( false ), selectRenderMode( 0 ),
    rescanhierarchies( true )
{
  delete _graphParams();
  _graphParams() = this;
  selectRenderModes.push_back( QT_TRANSLATE_NOOP( QGraphParam,
                                                  "ColoredSelection"  ) );
  selectRenderModes.push_back( QT_TRANSLATE_NOOP( QGraphParam,
                                                  "OutlinedSelection" ) );
}


GraphParams::~GraphParams()
{
  _graphParams() = 0;
}


GraphParams* & GraphParams::_graphParams()
{
  static GraphParams	*gp = 0;
  return gp;
}


GraphParams* GraphParams::graphParams()
{
  GraphParams	*gp = _graphParams();
  if( !gp )
    gp = new GraphParams;
  return gp;
}


void GraphParams::updateGraphs() const
{
  if( graphParams()->colorsActive )
    AGraph::specialColorFunc = &recolorLabelledGraph;
  else
    AGraph::specialColorFunc = 0;

  set<AObject *>			obj = theAnatomist->getObjects();
  set<AObject *>::const_iterator	io, fo=obj.end();
  AGraph				*ag;

  for( io=obj.begin(); io!=fo; ++io )
    {
      ag = dynamic_cast<AGraph *>( *io );
      if( ag )
	{
	  ag->SetMaterial( ag->GetMaterial() );
	  ag->notifyObservers( (void *) this );
	}
    }
  QAViewToolTip::setEnabled( toolTips );

  if( QGraphParam::theGP() )
    QGraphParam::theGP()->update();
}


Tree* GraphParams::findTreeWith( const Tree* tr, const string & att, 
				 const string & val, vector<Tree *> & parents )
{
  Tree			*t, *t2;
  Tree::const_iterator	it, ft=tr->end();
  string		aval;

  for( it=tr->begin(); it!=ft; ++it )
    {
      t = (Tree *) *it;
      t2 = findTreeWith( t, att, val, parents );
      if( t2 )
	{
	  parents.push_back( t );
	  return( t2 );
	}
      if( t->getProperty( att, aval ) && aval == val )
	return( t );
    }

  return( 0 );	// not found
}


void GraphParams::allowRescanHierarchies( bool x )
{
  rescanhierarchies = x;
}


bool GraphParams::nomenclatureColorForLabel( const string & label,
                                             const Hierarchy *hie,
                                             Material & mat )
{
  //	find matching element
  const list<Tree *>          *parents;
  Tree	*t = 0;
  string attval = label;

  //	check for compound names
  string		name;
  string::size_type	pos;

  while( !t && !attval.empty() )
  {
    pos = attval.find( '+' );
    if( pos == string::npos )
      pos = attval.size();
    name = attval.substr( 0, pos );
    attval.erase( 0, pos+1 );
    t = hie->findNamedNode( name, &parents );
  }
  if( !t )
    return false;

  vector<int>	col;
  list<Tree *>::const_iterator        it = parents->begin();
  ++it; // 1st is t

  while( !t->getProperty( "color", col ) && it != parents->end() )
  {
    t = *it;
    ++it;
  }

  if( col.size() >= 3 )
  {
    mat.SetDiffuse( ((float) col[0])/255, ((float) col[1])/255,
                      ((float) col[2])/255, mat.Diffuse( 3 ) );
    if( col.size() >= 4 )
      mat.SetDiffuseA( (float) col[3] / 255 );
    return true;
  }

  return false;
}


bool GraphParams::recolorLabelledGraph( AGraph*ag, AGraphObject* go,
                                        Material & mat )
{
  static Hierarchy	*hie = 0;
  string		synt, attval, prop;

  if( ag )
    prop = ag->labelProperty();
  else
    prop = graphParams()->attribute;
  if( prop.empty() || !go->attributed()->getProperty( prop, attval ) )
    return false;

  //	find matching hierarchy
  hie = Hierarchy::findMatchingNomenclature( go );
  if( !hie )
    return false;

  return nomenclatureColorForLabel( attval, hie, mat );
}


int GraphParams::selectRenderModeFromString( const string & mode )
{
  unsigned i, n = selectRenderModes.size();
  for( i=0; i<n; ++i )
    if( selectRenderModes[i] == mode )
      return i;
  return -1;
}


namespace
{
  QPushButton	*_selcol = 0;
}


struct QGraphParam::Private
{
  QCheckBox	*savemodif;
  QCheckBox	*autodir;
  QButtonGroup	*dispmode;
  QCheckBox	*usenomen;
  QComboBox	*attribs;
  QCheckBox	*ttips;
  QCheckBox	*invcol;
  QButtonGroup	*iobox;
  QCheckBox     *loadrelations;
  QComboBox     *selectmode;
};


QGraphParam* & QGraphParam::_qGraphParam()
{
  static QGraphParam	*gp = 0;
  return gp;
}


QGraphParam* QGraphParam::theGP()
{
  return _qGraphParam();
}


QGraphParam::QGraphParam( QWidget* parent, const char* name )
  :
    QWidget( parent, Qt::Window ),
    d( new Private )
{
  delete _qGraphParam();
  _qGraphParam() = this;
  GraphParams	*gp = GraphParams::graphParams();

  setWindowTitle( tr( "Graph parameters" ) );
  setObjectName(name);
  setAttribute(Qt::WA_DeleteOnClose);

  QVBoxLayout	*lay1 = new QVBoxLayout( this );
  lay1->setMargin( 5 );
  lay1->setSpacing( 5 );
  QGroupBox	*bgr 
    = new QGroupBox( tr( "Graph 3D display mode" ), this );
  QVBoxLayout *vlay = new QVBoxLayout( bgr );
  QButtonGroup *grb = new QButtonGroup( bgr );
  d->dispmode = grb;
  grb->setExclusive( true );
  QRadioButton *rb = new QRadioButton( tr( "Display trangulations" ), bgr );
  vlay->addWidget( rb );
  grb->addButton( rb, 0 );
  rb = new QRadioButton( tr( "Display facets" ), bgr );
  vlay->addWidget( rb );
  grb->addButton( rb, 1 );
  rb = new QRadioButton( tr( "Display all" ), bgr );
  vlay->addWidget( rb );
  grb->addButton( rb, 2 );
  rb = new QRadioButton( tr( "Display first object" ), bgr );
  vlay->addWidget( rb );
  grb->addButton( rb, 3 );
  vlay->addStretch( 1 );

  grb->button( (int) AGraphObject::showType() )->setChecked( true );

  QGroupBox	*cgr = new QGroupBox( tr( "Colors (2D / 3D)" ), this );
  vlay = new QVBoxLayout( cgr );
  QCheckBox	*col = 
    new QCheckBox( tr( "Use Nomenclature/color bindings (if any)" ), cgr );
  vlay->addWidget( col );
  d->usenomen = col;
  QWidget *fr1 = new QWidget( cgr );
  vlay->addWidget( fr1 );
  QHBoxLayout *hlay = new QHBoxLayout( fr1 );
  hlay->setMargin( 0 );
  hlay->setSpacing( 10 );
  QLabel	*lab1 = new QLabel( tr( "Use attribute :" ), fr1 );
  hlay->addWidget( lab1 );
  QComboBox	*attribs = new QComboBox( fr1 );
  hlay->addWidget( attribs );
  d->attribs = attribs;

  col->setChecked( gp->colorsActive );
  lab1->setFixedSize( lab1->sizeHint() );
  attribs->addItem( "name" );
  attribs->addItem( "label" );
  attribs->setFixedHeight( attribs->sizeHint().height() );
  if( gp->attribute == "label" )
    attribs->setCurrentIndex( 1 );

  QCheckBox	*tip = new QCheckBox( tr( "Display ToolTips" ), cgr );
  vlay->addWidget( tip );
  d->ttips = tip;
  tip->setChecked( gp->toolTips );
  vlay->addStretch( 1 );

  QGroupBox	*sel = new QGroupBox( tr( "Selection color" ), this );
  vlay = new QVBoxLayout( sel );
  QWidget *selhbox = new QWidget( sel );
  vlay->addWidget( selhbox );
  hlay = new QHBoxLayout( selhbox );
  hlay->setMargin( 0 );
  lab1 = new QLabel( tr( "Selection highlight type :" ), selhbox );
  hlay->addWidget( lab1 );
  lab1->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
  QComboBox *seltype = new QComboBox( selhbox );
  hlay->addWidget( seltype );
  int i, n = GraphParams::graphParams()->selectRenderModes.size();
  for( i=0; i<n; ++i )
    seltype->addItem(
      tr( GraphParams::graphParams()->selectRenderModes[i].c_str() ) );
  d->selectmode = seltype;
  d->selectmode->setCurrentIndex( GraphParams::graphParams()->selectRenderMode );
  QCheckBox	*inv = new QCheckBox( tr( "Use inverse color" ), sel );
  vlay->addWidget( inv );
  d->invcol = inv;

  QWidget *fr2 = new QWidget( sel );
  vlay->addWidget( fr2 );
  hlay = new QHBoxLayout( fr2 );
  hlay->setMargin( 0 );
  hlay->setSpacing( 10 );
  hlay->addWidget( new QLabel( tr( "Constant color :" ), fr2 ) );
  _selcol = new QPushButton( fr2 );
  hlay->addWidget( _selcol );
  hlay->addWidget( new QLabel( fr2 ) );
  vlay->addStretch( 1 );

  inv->setChecked( SelectFactory::selectColorInverse() );
  _selcol->setFixedSize( 32, 16 );
  QPixmap	pix( 32, 16 );
  pix.fill( QColor( (int) ( SelectFactory::selectColor().r * 255 ), 
                    (int) ( SelectFactory::selectColor().g * 255 ), 
                    (int) ( SelectFactory::selectColor().b * 255 ) ) );
  _selcol->setIcon( pix );

  QGroupBox *iobox = new QGroupBox( tr( "IO settings" ), this );
  vlay = new QVBoxLayout( iobox );
  QButtonGroup *iog = new QButtonGroup( iobox );
  d->iobox = iog;
  d->loadrelations = new QCheckBox( tr( "Load sub-objects in relations" ),
                                    iobox );
  vlay->addWidget( d->loadrelations );
  d->loadrelations->setChecked( gp->loadRelations );
  rb = new QRadioButton( tr( "Default (as loaded)" ), iobox );
  vlay->addWidget( rb );
  iog->addButton( rb, 0 );
  rb = new QRadioButton( tr( "Use one file per sub-object type" ), iobox );
  vlay->addWidget( rb );
  iog->addButton( rb, 1 );
  rb = new QRadioButton( tr( "Use one file per individual sub-object" ), 
                         iobox );
  vlay->addWidget( rb );
  iog->addButton( rb, 1 );
  iog->button( gp->saveMode )->setChecked( true );
  d->savemodif = new QCheckBox( tr( "Save only modified sub-objects" ), 
                                iobox );
  vlay->addWidget( d->savemodif );
  d->savemodif->setChecked( gp->saveOnlyModified );
  d->autodir = new QCheckBox( tr( "Set sub-objects dir. from graph filename" ),
                              iobox );
  vlay->addWidget( d->autodir );
  d->autodir->setChecked( gp->autoSaveDir );
  vlay->addStretch( 1 );

  lay1->addWidget( bgr );
  lay1->addWidget( cgr );
  lay1->addWidget( sel );
  lay1->addWidget( iobox );
  setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum ) );

  connect( grb, SIGNAL( buttonClicked( int ) ), 
           this, SLOT( btnClicked( int ) ) );
  connect( col, SIGNAL( toggled( bool ) ), this, 
	   SLOT( colorClicked( bool ) ) );
  connect( attribs, SIGNAL( activated( const QString & ) ), this, 
	   SLOT( attribActivated( const QString & ) ) );
  connect( tip, SIGNAL( toggled( bool ) ), this, 
	   SLOT( installToolTips( bool ) ) );

  connect( inv, SIGNAL( toggled( bool ) ), this, 
	   SLOT( invSelColorClicked( bool ) ) );
  connect( _selcol, SIGNAL( clicked() ), this, 
	   SLOT( selColorClicked() ) );
  connect( d->loadrelations, SIGNAL( toggled( bool ) ), this,
           SLOT( loadRelationsChanged( bool ) ) );
  connect( d->savemodif, SIGNAL( toggled( bool ) ), this,
	   SLOT( saveModifChanged( bool ) ) );
  connect( d->autodir, SIGNAL( toggled( bool ) ), this, 
	   SLOT( autoDirChanged( bool ) ) );
  connect( iog, SIGNAL( buttonClicked( int ) ), this, 
	   SLOT( saveModeSelected( int ) ) );
  connect( seltype, SIGNAL( activated(int) ), this,
           SLOT( setSelectionRenderingMode( int ) ) );
}


QGraphParam::~QGraphParam()
{
  delete d;
  _qGraphParam() = 0;
}


void QGraphParam::btnClicked( int btn )
{
  if( (int) AGraphObject::showType() != btn )
    {
      AGraphObject::setShowType( (AGraphObject::ShowType) btn );
      refreshGraphs();
    }
}


void QGraphParam::colorClicked( bool onoff )
{
  GraphParams::graphParams()->colorsActive = onoff;
  refreshGraphs();
}


void QGraphParam::attribActivated( const QString & str )
{
  GraphParams::graphParams()->attribute = str.toStdString();
  refreshGraphs();
}


void QGraphParam::refreshGraphs() const
{
  GraphParams::graphParams()->updateGraphs();
}


void QGraphParam::installToolTips( bool onoff )
{
  GraphParams::graphParams()->toolTips = onoff;
  QAViewToolTip::setEnabled( onoff );
}


void QGraphParam::invSelColorClicked( bool onoff )
{
  SelectFactory::selectColorInverse() = onoff;
}


void QGraphParam::selColorClicked()
{
  int	alpha = (int) ( SelectFactory::selectColor().a * 255 );
  bool	nalpha = SelectFactory::selectColor().na;
  QColor	col 
    = QAColorDialog::getColor
    ( QColor( (int) ( SelectFactory::selectColor().r * 255 ),
              (int) ( SelectFactory::selectColor().g * 255 ),
              (int) ( SelectFactory::selectColor().b * 255 ) ),
      theAnatomist->getQWidgetAncestor(),
      tr( "Selection color" ).toStdString().c_str(), &alpha, &nalpha );
  if( col.isValid() )
    {
      SelectFactory::HColor	hcol( ((float) col.red()) / 255, 
                                      ((float) col.green()) / 255, 
                                      ((float) col.blue()) / 255, 
                                      ((float) alpha) / 255, nalpha );

      QPixmap pix( 32, 16 );
      pix.fill( col );
      _selcol->setIcon( pix );
      SelectFactory::setSelectColor( hcol );
    }
}


void QGraphParam::loadRelationsChanged( bool x )
{
  GraphParams::graphParams()->loadRelations = x;
}


void QGraphParam::saveModifChanged( bool x )
{
  GraphParams::graphParams()->saveOnlyModified = x;
}


void QGraphParam::autoDirChanged( bool x )
{
  GraphParams::graphParams()->autoSaveDir = x;
}


void QGraphParam::saveModeSelected( int x )
{
  if( x < 3 )
    GraphParams::graphParams()->saveMode = x;
}


const string & QGraphParam::nomenclatureAttrib() const
{
  return GraphParams::graphParams()->attribute;
}


void QGraphParam::setLabelColorsActivated( bool state )
{
  GraphParams::graphParams()->colorsActive = state;
}


bool QGraphParam::labelColorsActivated()
{
  return GraphParams::graphParams()->colorsActive;
}


void QGraphParam::setColorsAttribute( const string & attr )
{
  GraphParams::graphParams()->attribute = attr;
}


string QGraphParam::colorsAttribute()
{
  return GraphParams::graphParams()->attribute;
}


bool QGraphParam::toolTipsInstalled()
{
  return GraphParams::graphParams()->toolTips;
}


int QGraphParam::savingMode()
{
  return GraphParams::graphParams()->saveMode;
}


bool QGraphParam::saveOnlyModified()
{
  return GraphParams::graphParams()->saveOnlyModified;
}


bool QGraphParam::autoSaveDirectory()
{
  return GraphParams::graphParams()->autoSaveDir;
}


void QGraphParam::update()
{
  blockSignals( true );

  GraphParams	*gp = GraphParams::graphParams();

  d->dispmode->button( (int) AGraphObject::showType() )->setChecked( true );
  d->usenomen->setChecked( gp->colorsActive );
  int		i, n = d->attribs->count();
  for( i=0; i<n; ++i )
    if( d->attribs->itemText( i ) == gp->attribute.c_str() )
    {
      d->attribs->setCurrentIndex( i );
      break;
    }
  if( i == n )
  {
    d->attribs->addItem( gp->attribute.c_str() );
    d->attribs->setCurrentIndex( i );
  }
  d->ttips->setChecked( gp->toolTips );
  d->invcol->setChecked( SelectFactory::selectColorInverse() );

  QPixmap	pix( 32, 16 );
  pix.fill( QColor( (int) ( SelectFactory::selectColor().r * 255 ), 
                    (int) ( SelectFactory::selectColor().g * 255 ), 
                    (int) ( SelectFactory::selectColor().b * 255 ) ) );
  _selcol->setIcon( pix );

  d->selectmode->setCurrentIndex( 
    GraphParams::graphParams()->selectRenderMode );

  d->iobox->button( gp->saveMode )->setChecked( true );
  d->savemodif->setChecked( gp->saveOnlyModified );
  d->autodir->setChecked( gp->autoSaveDir );

  blockSignals( false );
}


void QGraphParam::setSelectionRenderingMode( int i )
{
  cout << "setSelectionRenderingMode " << i << endl;
  GraphParams::graphParams()->selectRenderMode = i;
  SelectFactory::factory()->refreshSelectionRendering();
}

