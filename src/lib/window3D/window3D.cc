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

#include <cstdlib>
#include <anatomist/surface/texsurface.h>
#include <anatomist/surface/texture.h>
#include <anatomist/surface/surface.h>
#include <anatomist/surface/triangulated.h>
#include <aims/vector/vector.h>
#include <anatomist/surface/glcomponent.h>
#include <aims/utility/converter_texture.h>

#include <anatomist/window3D/window3D.h>
#include <anatomist/window3D/window3D_private.h>
#include <anatomist/window/glwidgetmanager.h>
#include <anatomist/window3D/glwidget3D.h>
#include <anatomist/control/toolTips-qt.h>
#include <anatomist/reference/Geometry.h>
#include <anatomist/reference/Referential.h>
#include <anatomist/reference/Transformation.h>
#include <anatomist/reference/transfSet.h>
#include <anatomist/controler/controlswitch.h>
#include <anatomist/controler/controldictionary.h>
#include <anatomist/controler/controlmanager.h>
#include <anatomist/reference/wChooseReferential.h>
#include <anatomist/controler/icondictionary.h>
#include <anatomist/misc/error.h>
#include <anatomist/selection/selectFactory.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/color/Light.h>
#include <anatomist/window3D/wLightModel.h>
#include <anatomist/window3D/wFixedPointOfView.h>
#include <anatomist/window3D/wTools3D.h>
#include <anatomist/window3D/cursor.h>
#include <anatomist/window3D/zoomDialog.h>
#include <anatomist/window/colorstyle.h>
#include <anatomist/window/glcaps.h>
#include <anatomist/window/viewstate.h>
#include <anatomist/application/globalConfig.h>
#include <anatomist/mobject/MObject.h>
#include <anatomist/commands/cLinkedCursor.h>
#include <anatomist/commands/cCreateWindow.h>
#include <anatomist/commands/cAddObject.h>
#include <anatomist/commands/cWindowConfig.h>
#include <anatomist/processor/Processor.h>
#include <anatomist/control/graphParams.h>
#include <anatomist/reference/transformobserver.h>
#include <anatomist/application/settings.h>
#include <anatomist/window3D/orientationAnnotation.h>
#include <anatomist/window3D/agraphicsview_p.h>
#include <anatomist/object/objectConverter.h>
#include <anatomist/mobject/Fusion2D.h>
#include <anatomist/fusion/defFusionMethods.h>
#include <aims/mesh/surfaceOperation.h>
#include <anatomist/object/objectparamselect.h>
#include <aims/resampling/quaternion.h>
#include <qslider.h>
#include <qglobal.h>
#include <qmessagebox.h>
#ifdef ANA_USE_QOPENGLWIDGET
#include <QOpenGLWidget>
#else
#include <QGLWidget>
#endif
#include <qlabel.h>
#include <qmenubar.h>
#include <qmenu.h>
#include <qdialog.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <qtoolbar.h>
#include <qpushbutton.h>
#include <qapplication.h>
#include <qlist.h>
#include <qtimer.h>
#include <QScreen>
#include <qstatusbar.h>
#include <qpainter.h>
#include <iostream>
#include <float.h>
#ifdef _WIN32
#define rint(x) floor(x+0.5)
#endif

#include <anatomist/graph/attribAObject.h>
#include <anatomist/graph/GraphObject.h>
#include <anatomist/graph/Graph.h>

#ifdef __APPLE__
#include <AvailabilityMacros.h>
#endif

/* whith ANA_USE_QGRAPHICSVIEW defined, 3D windows will contain a 
   QGraphicsView, in which the OpenGL widget is the background (viewport).
*/
#define ANA_USE_QGRAPHICSVIEW

// uncomment this to enable debug output for update pattern
//#define ANA_DEBUG_UPDATE

using namespace anatomist;
using namespace anatomist::internal;
using namespace aims;
using namespace carto;
using namespace std;

// static

string AWindow3D::_baseTitle = "3D";
set<unsigned> AWindow3D::_count3d;

static string AWindow3D_axialTitle = "A";
static string AWindow3D_coronalTitle = "C";
static string AWindow3D_sagittalTitle = "S";
static string AWindow3D_3DTitle = "3D";

struct AWindow3D::Private
{
    enum RefreshType
    {
      LightRefresh, TempRefresh, FullRefresh,
    };
    enum RenderConstraint
    {
      RenderAfter, RenderBefore,
    };
    typedef pair<RenderConstraint, AObject *> ConstrainedObject;

    Private();
    ~Private();
    void deleteLists();

    GLWidgetManager *draw;
    vector<QSlider *> sliders;
    QSlider *slidt;
    QWidget *refbox;
    QPushButton *reflabel;
    QLabel *refdirmark;
    QLabel *timelabel;
    QWidget *timepanel;
    vector<QWidget *> sliderpanels;
    QSlider *slids;
    QLabel *slicelabel;
    vector<QLabel *> slicelabels;
    QWidget *slicepanel;
    QAViewToolTip *tooltip;
    AWindow3D::ViewType viewtype;
    QToolBar *mute;
    QToolBar *save_tb;
    QAction *axialbt;
    QAction *coronalbt;
    QAction *sagittalbt;
    QAction *obliquebt;
    QAction *threedbt;
    QAction *savebt;
    PrimList primitives;
    bool orientationCube;
    bool boundingFrame;
    AWindow3D::RenderingMode renderingMode;
    Tools3DWindow *tools;
    FixedPointOfViewWindow *poview;
    LightModelWindow *lightview;
    Light *light;
    Quaternion slicequat;
    QSize askedsize;
    AWindow3D::ClipMode clipmode;
    float clipdist;
    bool transpz;
    bool culling;
    bool flatshading;
    bool smooth;
    bool fog;
    map<AObject*, pair<unsigned, unsigned> > tmpprims;
    RefreshType refreshneeded;
    list<ObjectModifier *> objmodifiers;
    bool linkonslider;
    AWindow3D *lefteye;
    AWindow3D *righteye;
    QLabel *objvallabel;
    bool statusbarvisible;
    bool needsextrema;
    map<AObject *, ConstrainedObject> renderconstraints;

    int mouseX;
    int mouseY;
    bool surfpaintState;
    bool constraintEditorState;

    std::vector<string> constraintList;
    int constraintType;
    AObject *texConstraint;
    OrientationAnnotation * orientAnnot;
    bool sortPolygons;
    bool sortPolygonsDirection;
    // auto-fusion volumes or Sliceable objects
    rc_ptr<AObject> autoFusion;
};

namespace
{
  using anatomist::Referential;

  Quaternion axialQuaternion(Referential* ref)
  {
    string ax;
    theAnatomist->config()->getProperty("axialConvention", ax);
    Quaternion quat(1, 0, 0, 0);
    if (ax == "neuro") quat = Quaternion(0, 0, 1, 0);
    if (ref && ref != Referential::acPcReferential())
    {
      const anatomist::Transformation *t =
          ATransformSet::instance()->transformation(
              Referential::acPcReferential(), ref);
      if (t)
      {
        Point3df p = t->transform( Point3df( 1, 0, 0 ) )
          - t->transform( Point3df( 0, 0, 0 ) );
        Quaternion q;
        if (p[0] < 0)
        {
          q.fromAxis(Point3df(0, 0, 1), M_PI);
          quat *= q;
          q.fromAxis(Point3df(1, 0, 0), M_PI);
          quat *= q;
        }
        p = t->transform( Point3df( 0, 1, 0 ) )
          - t->transform( Point3df( 0, 0, 0 ) );
        if (p[1] < 0)
        {
          q.fromAxis(Point3df(1, 0, 0), M_PI);
          quat *= q;
        }
      }
    }
    return quat;
  }

  Quaternion coronalQuaternion(Referential* ref)
  {
    static const float c = 1. / sqrt(2.);
    string ax;
    theAnatomist->config()->getProperty("axialConvention", ax);
    Quaternion quat(c, 0, 0, c);
    if (ax == "neuro") quat = Quaternion(0, c, c, 0);
    if (ref && ref != Referential::acPcReferential())
    {
      const anatomist::Transformation *t =
          ATransformSet::instance()->transformation(
              Referential::acPcReferential(), ref);
      if (t)
      {
        Point3df p = t->transform( Point3df( 1, 0, 0 ) )
          - t->transform( Point3df( 0, 0, 0 ) );
        Quaternion q;
        if (p[0] < 0)
        {
          q.fromAxis(Point3df(0, 1, 0), M_PI);
          quat *= q;
          q.fromAxis(Point3df(1, 0, 0), M_PI);
          quat *= q;
        }
        p = t->transform( Point3df( 0, 0, 1 ) )
          - t->transform( Point3df( 0, 0, 0 ) );
        if (p[2] < 0)
        {
          q.fromAxis(Point3df(1, 0, 0), M_PI);
          quat *= q;
        }
      }
      if (ref->isDirect())
      {
        // remove the bottom-up inversion
        Quaternion q(1, 0, 0, 0);
        quat *= q;
      }
    }
    return quat;
  }

  Quaternion sagittalQuaternion(Referential* ref)
  {
    Quaternion quat(0.5, 0.5, 0.5, 0.5);
    if (ref && ref != Referential::acPcReferential())
    {
      const anatomist::Transformation *t =
          ATransformSet::instance()->transformation(
              Referential::acPcReferential(), ref);
      if (t)
      {
        Point3df p = t->transform( Point3df( 0, 1, 0 ) )
          - t->transform( Point3df( 0, 0, 0 ) );
        Quaternion q;
        if (p[1] < 0)
        {
          q.fromAxis(Point3df(1, 0, 0), M_PI);
          quat *= q;
          q.fromAxis(Point3df(0, 1, 0), M_PI);
          quat *= q;
        }
        p = t->transform( Point3df( 0, 0, 1 ) )
          - t->transform( Point3df( 0, 0, 0 ) );
        if (p[2] < 0)
        {
          q.fromAxis(Point3df(0, 1, 0), M_PI);
          quat *= q;
        }
      }
      if (ref->isDirect())
      {
        // remove the bottom-up inversion
        Quaternion q(0, 1, 0, 0);
        quat *= q;
      }
    }
    return quat;
  }

  AObject* objectWithGLID(int id)
  {
    if (id < 0) return 0;
    static map<int, AObject *> cache;
    map<int, AObject *>::iterator ic, ec = cache.end();
    ic = cache.find(id);
    if (ic != ec)
    {
      if (theAnatomist->hasObject(ic->second))
      {
        GLComponent* glc = ic->second->glAPI();
        if (glc && glc->glObjectID() == id)
        {
          return ic->second;
        }
      }
      cache.erase(ic); // invalid
    }
    const set<AObject *> objs = theAnatomist->getObjects();
    set<AObject *>::const_iterator io, eo = objs.end();
    AObject *obj;
    int oid;
    for (io = objs.begin(); io != eo; ++io)
    {
      obj = *io;
      GLComponent* glc = obj->glAPI();
      if (glc)
      {
        oid = glc->glObjectID();
        cache[oid] = obj;
        if (oid == id)
        {
          // TODO: check parents/children
          return obj;
        }
      }
    }
    return 0;
  }
}

AWindow3D::Private::Private() :
  draw(0), slidt(0), refbox(0), reflabel(0), refdirmark(0), timelabel(0),
      timepanel(0), slids(0), slicelabel(0), slicepanel(0), tooltip(0),
      viewtype(AWindow3D::Oblique), mute(0), save_tb( 0 ), axialbt(0),
      coronalbt(0), sagittalbt(0), obliquebt(0), threedbt( 0 ), savebt( 0 ),
      orientationCube(false),
      boundingFrame(false), renderingMode(AWindow3D::Normal), tools(0),
      poview(0), lightview(0), light(new Light), slicequat(0, 0,
          0, 1), askedsize(0, 0), clipmode(AWindow3D::NoClip), clipdist(1),
      transpz(true), culling(true), flatshading(false), smooth(false), fog(
          false), refreshneeded(FullRefresh), linkonslider(false), lefteye(0),
      righteye(0), objvallabel(0), statusbarvisible(false),
      needsextrema(false),
      mouseX(0), mouseY(0), surfpaintState(false), constraintEditorState(false),
      constraintList(), constraintType(0), texConstraint(0), orientAnnot( 0 ), sortPolygons( false ), sortPolygonsDirection( false ), autoFusion( 0 )
{
  try
  {
    Object oc = theAnatomist->config()->getProperty( "windowBackground" );
    if( !oc.isNull() )
    {
      Object oi = oc->objectIterator();
      if( oi->isValid() )
      {
        light->Background()[0] = oi->currentValue()->getScalar();
        oi->next();
        if( oi->isValid() )
        {
          light->Background()[1] = oi->currentValue()->getScalar();
          oi->next();
          if( oi->isValid() )
          {
            light->Background()[2] = oi->currentValue()->getScalar();
            oi->next();
            if( oi->isValid() )
              light->Background()[3] = oi->currentValue()->getScalar();
          }
        }
      }
    }
  }
  catch( ... )
  {
  }
}

AWindow3D::Private::~Private()
{
  if( autoFusion )
    autoFusion.reset( 0 );
  while( !objmodifiers.empty() )
    delete objmodifiers.front();
  delete tools;
  delete poview;
  delete lightview;
  delete light;
  delete orientAnnot;
}

void AWindow3D::Private::deleteLists()
{
}

//	Objects modifier


AWindow3D::ObjectModifier::ObjectModifier(AWindow3D* w) :
  _window(w)
{
  w->registerObjectModifier(this);
}

AWindow3D::ObjectModifier::~ObjectModifier()
{
  _window->unregisterObjectModifier(this);
}

namespace
{

  using anatomist::Referential;

  void paintRefLabel(QPushButton* reflabel, 
                     QLabel* refdirmark, const Referential* ref)
  {
    if (ref && ref->isDirect())
    {
      AimsRGB col = ref->Color();
      QPixmap pix(32, 7);
      pix.fill(QColor(col.red(), col.green(), col.blue()));
      QPainter p;
      int darken = 25;
      p.begin(&pix);
      p.setPen(QPen(QColor(col.red() > darken ? col.red() - darken : col.red()
          + darken, col.green() > darken ? col.green() - darken : col.green()
          + darken, col.blue() > darken ? col.blue() - darken : col.blue()
          + darken), 5));
      p.drawLine(3, 10, 25, -3);
      p.end();
      QPalette pal( QColor(col.red(), col.green(), col.blue()) );
      pal.setBrush( QPalette::Window, pix );
      reflabel->setPalette( pal );
      if( refdirmark )
        refdirmark->show();
    }
    else
    {
//       reflabel->unsetPalette();
//       reflabel->setBackgroundMode(Qt::PaletteButton);
      if (refdirmark) refdirmark->hide();
      if (ref)
      {
        AimsRGB col = ref->Color();
        reflabel->setPalette(QPalette(
            QColor(col.red(), col.green(), col.blue())));
      }
      else
        reflabel->setPalette(QPalette(QColor(192, 192, 192)));
    }
 }

  AWindow3D::GLWidgetCreator & glWidgetCreator()
  {
    static AWindow3D::GLWidgetCreator c = 0;
    return c;
  }

}


//	AWindow3D

AWindow3D::AWindow3D(ViewType t, QWidget* parent, Object options, Qt::WindowFlags f) :
    ControlledWindow(parent, "window3D", options, f),
    Observable(),
    d(new AWindow3D::Private)
{
  bool nodeco = !toolBarsVisible();
  setAttribute( Qt::WA_DeleteOnClose );

  QWidget *vb = new QWidget(this);
  QVBoxLayout *vlay = new QVBoxLayout( vb );
  vb->setLayout( vlay );
  vlay->setContentsMargins( 2, 2, 2, 2 );
  vlay->setSpacing(5);
  d->refbox = new QWidget(vb);
  QHBoxLayout *hlay = new QHBoxLayout( d->refbox );
  d->refbox->setLayout( hlay );
  hlay->setContentsMargins( 0, 0, 0, 0 );
  hlay->setSpacing( 5 );
  vlay->addWidget( d->refbox );
  d->reflabel = new QPushButton(d->refbox);
  hlay->addWidget( d->reflabel );
  const QPixmap *directpix;
  IconDictionary *icons = IconDictionary::instance();
  directpix = icons->getIconInstance("direct_ref_mark");
  d->refdirmark = new QLabel(d->refbox);
  hlay->addWidget( d->refdirmark );
  if (directpix) d->refdirmark->setPixmap(*directpix);
  d->refdirmark->setFixedSize(QSize(21, 7));
  d->refdirmark->hide();
  setQtColorStyle(d->reflabel);
  d->reflabel->setFixedHeight(7);
  d->refbox->setFixedHeight(d->refbox->sizeHint().height());
  if (nodeco) d->refbox->hide();

  QWidget *hb = new QWidget(vb);
  hb->setObjectName( "horiz_widget" );
  vlay->addWidget( hb );
  QHBoxLayout *hbl = new QHBoxLayout( hb );
  hbl->setSpacing(0);
  hbl->setContentsMargins( 0, 0, 0, 0 );

  paintRefLabel(d->reflabel, d->refdirmark, getReferential());

  QWidget *daparent = hb;
  AGraphicsView *gv = 0;
#ifdef ANA_USE_QGRAPHICSVIEW
#if ( defined( __APPLE__ ) && !defined( MAC_OS_X_VERSION_10_11 ) ) || defined( _WIN32 ) // does not work well on Mac with Qt 4.6
  bool use_graphicsview = false;
#else
  bool use_graphicsview = true;
#endif
  try
  {
    Object o = theAnatomist->config()->getProperty(
      "windowsUseGraphicsView" );
    use_graphicsview = (bool) o->getScalar();
  }
  catch( ... )
  {
  }
  if( use_graphicsview )
    gv = new AGraphicsView( hb );
#endif
  if( gv )
  {
    daparent = gv;
    hbl->addWidget( daparent );
  }
  if( glWidgetCreator() )
    d->draw = glWidgetCreator()( this, daparent, "GL drawing area",
        GLWidgetManager::sharedWidget(), Qt::WindowFlags() );
  else
  {
    d->draw = new QAGLWidget3D( this, daparent, "GL drawing area",
                                GLWidgetManager::sharedWidget() );
  }
  if( gv )
  {
    gv->setViewport( d->draw->qglWidget() );
    gv->setScene( new AGraphicsScene( gv ) );
    gv->setFrameStyle( QFrame::NoFrame );
    gv->setViewportUpdateMode( QGraphicsView::FullViewportUpdate );
    gv->setAcceptDrops( true );
  }
  else
    hbl->addWidget( d->draw->qglWidget() );

  float wf = 1.5;
  theAnatomist->config()->getProperty("windowSizeFactor", wf);

//   if( parentWidget() == 0 )
  if( gv )
    switch (t)
    {
      case Axial:
        gv->setSizeHint( QSize( int(256 * wf), int(256 * wf) ) );
        break;
      case Coronal:
        gv->setSizeHint( QSize( int(256 * wf), int(124 * wf) ) );
        break;
      case Sagittal:
        gv->setSizeHint( QSize( int(256 * wf), int(124 * wf) ) );
        break;
      default:
        gv->setSizeHint( QSize( int(256 * wf), int(256 * wf) ) );
        break;
    }
  else
    switch (t)
    {
      case Axial:
        d->draw->setPreferredSize(int(256 * wf), int(256 * wf));
        break;
      case Coronal:
        d->draw->setPreferredSize(int(256 * wf), int(124 * wf));
        break;
      case Sagittal:
        d->draw->setPreferredSize(int(256 * wf), int(124 * wf));
        break;
      default:
        break;
    }

  d->slicepanel = new QWidget(hb);
  d->sliderpanels.push_back( d->slicepanel );
  vlay = new QVBoxLayout( d->slicepanel );
  vlay->setSpacing(0);
  vlay->setContentsMargins( 0, 0, 0, 0 );
  hbl->addWidget( d->slicepanel );
  d->slicelabel = new QLabel( "0", d->slicepanel );
  d->slicelabels.push_back( d->slicelabel );
  d->slicelabel->setFixedWidth(30);
  vlay->addWidget( d->slicelabel );
  d->slids = new NoDragSlider( 0, 0, 0, 1, 0, Qt::Vertical, d->slicepanel,
                               "sliderS" );
  d->sliders.push_back( d->slids );
  d->slids->setFixedWidth(d->slids->sizeHint().width());
  vlay->addWidget( d->slids );
  d->slicepanel->setSizePolicy(QSizePolicy(QSizePolicy::Fixed,
      QSizePolicy::Expanding));
  d->slicepanel->hide();

  d->timepanel = new QWidget(hb);
  d->sliderpanels.push_back( d->timepanel );
  vlay = new QVBoxLayout( d->timepanel );
  vlay->setSpacing(0);
  vlay->setContentsMargins( 0, 0, 0, 0 );
  hbl->addWidget( d->timepanel );
  d->timelabel = new QLabel( "0", d->timepanel );
  d->slicelabels.push_back( d->timelabel );
  d->timelabel->setFixedWidth(30);
  vlay->addWidget( d->timelabel );
  d->slidt = new NoDragSlider( 1, 0, 0, 1, 0, Qt::Vertical, d->timepanel,
                               "sliderT" );
  d->sliders.push_back( d->slidt );
  d->slidt->setFixedWidth(d->slidt->sizeHint().width());
  vlay->addWidget( d->slidt );
  d->timepanel->setSizePolicy(QSizePolicy(QSizePolicy::Fixed,
      QSizePolicy::Expanding));
  d->timepanel->hide();
  d->slids->setInvertedAppearance( true );
  d->slids->setInvertedControls( true );
  d->slidt->setInvertedAppearance( true );
  d->slids->setInvertedControls( true );

  d->objvallabel = new QLabel(statusBar());
  statusBar()->addPermanentWidget( d->objvallabel, 0 );
  d->objvallabel->setTextInteractionFlags(
    d->objvallabel->textInteractionFlags() | Qt::TextSelectableByMouse
    | Qt::TextSelectableByKeyboard );
//   // find label in statusbar
//   QWidgetItem *wi = dynamic_cast<QWidgetItem *>(
//     static_cast<QBoxLayout *>(
//       static_cast<QBoxLayout *>( statusBar()->layout()->itemAt(0) )
//       ->itemAt(1) )->itemAt(2) );
//   if( wi )
//   {
//     QLabel *l = dynamic_cast<QLabel *>( wi->widget() );
//     if( l )
//       l->setTextInteractionFlags(
//         l->textInteractionFlags() | Qt::TextSelectableByMouse
//         | Qt::TextSelectableByKeyboard );
//     cout << "label: " << l << ", " << d->objvallabel << endl;
//   }

  bool showsbar = !nodeco;
  if( showsbar )
    try
    {
      Object o = theAnatomist->config()->getProperty( "displayCursorPosition"
        );
      if( !o.isNull() )
        showsbar = (bool) o->getScalar();
    }
    catch( ... )
    {
    }

  if( showsbar )
    d->statusbarvisible = true;
  else
    statusBar()->hide();

  setCentralWidget(vb);

  //	controls

  d->draw->controlSwitch()->attach(this);
  d->draw->controlSwitch()->notifyAvailableControlChange();
  d->draw->controlSwitch()->notifyActivableControlChange();
  // d->draw->controlSwitch()->setActiveControl("Default 3D control");
  // // should be done in ControlSwitch
  // d->draw->controlSwitch()->notifyActiveControlChange();
  // d->draw->controlSwitch()->notifyActionChange();
  setFocusProxy(d->draw->qglWidget());

  if (!nodeco)
  {
    const QObjectList & ch = children();
    QObjectList::const_iterator ic, ec = ch.end();
    QToolBar *ntb = 0;
    for( ic=ch.begin(); ic!=ec && !(ntb=qobject_cast<QToolBar *>( *ic ) );
        ++ic );
    if( !ntb )
    ntb = addToolBar( Qt::LeftToolBarArea, tr( "controls" ), "controls" );

    //	Menus

    QMenu *win = new QMenu( tr("Window"), this );
    menuBar()->addMenu(win);
    win->addAction( tr("Save..."), this, SLOT(saveSnapshot()) );
    win->addAction( tr("Save high-res..."),
                    this, SLOT(saveSnapshotWithCustomSize()) );
    win->addAction( tr("Start recording..."),
                    d->draw->qobject(), SLOT( recordStart() ) );
    win->addAction( tr("Start recording high-res..."),
                    d->draw->qobject(), SLOT( recordStartWithCustomSize() ) );
    win->addAction( tr("Stop recording"), d->draw->qobject(),
        SLOT(recordStop()) );
    win->addSeparator();
    win->addAction( tr("Resize..."), this, SLOT(resizeView()) );
    win->addAction( tr("Zoom..."), this, SLOT(askZoom()) );
    win->addSeparator();
    win->addAction( tr("Show/hide toolbox (ROI etc)"), this, SLOT(
        switchToolbox()), Qt::Key_F1 );
    win->addAction( tr("Show/hide menus and toolbars"), this, SLOT(
        toggleToolBars()) );
    win->addAction( tr("Show/hide cursor position"), this, SLOT(
        toggleStatusBarVisibility()) );
    win->addSeparator();
    setDetachMenuAction( win->addAction( tr( "Detach view" ), this,
            SLOT( detach() ) ) );
    if( !parent )
      enableDetachMenu( false );
    if( theAnatomist->userLevel() >= 2 ) 
      win->addAction( tr( "Open stereoscopic right eye view"), this, 
                      SLOT(openStereoView()) );
    win->addSeparator();
    win->addAction( tr("Close"), this, SLOT(close()), Qt::CTRL | Qt::Key_W );

    QMenu *scene = new QMenu( tr("Scene"), this );
    menuBar()->addMenu( scene );
    scene->addAction( tr("Lighting"), this, SLOT(lightView()) );
    scene->addSeparator();
    scene->addAction( tr("Fixed points of view"), this, SLOT(pointsOfView()) );
    scene->addAction( tr("Tools"), this, SLOT(tools()) );
    scene->addAction( tr("Sync 3D views in same group"), this,
        SLOT(syncViews()) );
    scene->addAction( tr("Focus view on objects"), this, SLOT(focusView()),
        Qt::Key_Home );
    scene->addAction( tr("Auto-set rotation center in middle of scene"), this,
        SLOT(setAutoRotationCenter()) );
    scene->addAction( tr("Manually specify linked cursor position"), this,
        SLOT(setLinkedCursorPos()), Qt::CTRL | Qt::Key_P );
    scene->addAction( tr("Find vertex / polygon / texture"), this,
        SLOT(findPrimitive()), Qt::CTRL | Qt::Key_F );

    //	Mutation toolbar

    d->mute = insertToolBar( 0, Qt::TopToolBarArea, tr( "mutations" ),
                             "mutations" );
    d->mute->setIconSize( QSize( 20, 20 ) );
    const QPixmap *p;

    p = icons->getIconInstance("axial");
    if (p)
      d->axialbt = d->mute->addAction( *p, tr("Axial"),
        this, SLOT( muteAxial() ) );
    else
      d->axialbt = d->mute->addAction( tr("Axial"),
        this, SLOT( muteAxial() ) );
    d->axialbt->setToolTip( tr("Mute into axial view") );

    p = icons->getIconInstance("coronal");
    if (p)
      d->coronalbt = d->mute->addAction( *p, tr("Coronal"), 
        this, SLOT( muteCoronal() ) );
    else
      d->coronalbt = d->mute->addAction( tr("Coronal"), 
        this, SLOT( muteCoronal() ) );
    d->coronalbt->setToolTip( tr( "Mute into coronal view") );

    p = icons->getIconInstance("sagittal");
    if (p)
      d->sagittalbt = d->mute->addAction( *p, tr("Sagittal"), 
        this, SLOT( muteSagittal() ) );
    else
      d->sagittalbt = d->mute->addAction( tr("Sagittal"), 
        this, SLOT( muteSagittal() ) );
    d->sagittalbt->setToolTip( tr( "Mute into sagittal view" ) );

    p = icons->getIconInstance("oblique");
    if (p)
      d->obliquebt = d->mute->addAction( *p, tr("Oblique"), 
        this, SLOT( muteOblique() ) );
    else
      d->obliquebt = d->mute->addAction( tr("Oblique"), 
        this, SLOT( muteOblique() ) );
    d->obliquebt->setToolTip( tr( "Free orientation view" ) );

    p = icons->getIconInstance("3D");
    if (p)
      d->threedbt = d->mute->addAction( *p, tr("3D"), this, SLOT( mute3D() ) );
    else
      d->threedbt = d->mute->addAction( tr("3D"), this, SLOT( mute3D() ) );
    d->threedbt->setToolTip( tr("Mute into 3D view") );

    d->save_tb = insertToolBar( 1, Qt::TopToolBarArea, tr( "save" ),
                                "save" );
    d->save_tb->setIconSize( QSize( 20, 20 ) );

    p = icons->getIconInstance("save");
    if (p)
      d->savebt = d->save_tb->addAction( *p, tr("Save"),
        this, SLOT( saveObject() ) );
    else
      d->savebt = d->save_tb->addAction( tr("Save"),
        this, SLOT( saveObject() ) );
    d->savebt->setToolTip( tr(
      "Save modified objects, in place (overwrite older files)") );
    d->savebt->setEnabled( false );

    d->axialbt->setCheckable(true);
    d->coronalbt->setCheckable(true);
    d->sagittalbt->setCheckable(true);
    d->obliquebt->setCheckable(true);
    d->threedbt->setCheckable(true);

    d->mute->addSeparator();
    p = icons->getIconInstance("RoiControl");
    QAction *roibt;
    if (p)
      roibt = d->mute->addAction( *p, tr("ROI toolbox"),
        this, SLOT( switchToolbox() ) );
    else
      roibt = d->mute->addAction( tr("ROI toolbox"),
        this, SLOT( switchToolbox() ) );
    roibt->setToolTip( tr( "Open the ROI toolbox" ) );
    roibt->setCheckable( true );

    // in some versions of Qt5, the menubar does not show automatically
    menuBar()->show();

  }

  //	Signals & slots
  connect( d->slids, SIGNAL( valueChanged( int ) ), this,
      SLOT( changeSlice( int ) ) );
  connect( d->slidt, SIGNAL( valueChanged( int ) ), this,
      SLOT( changeTime( int ) ) );
  connect(d->reflabel, SIGNAL(clicked()), this, SLOT(changeReferential()));

  setViewType(t);

  d->tooltip = new QAViewToolTip(this, d->draw->qglWidget());

  d->orientAnnot = new OrientationAnnotation( this );
  setChanged();
  Refresh();
}

AWindow3D::~AWindow3D()
{
  if (d->lefteye)
  {
    AWindow3D *w = d->lefteye;
    d->lefteye = 0;
    w->setRightEyeWindow(0);
  }
  if (d->righteye)
  {
    AWindow3D *w = d->righteye;
    d->righteye = 0;
    delete w;
  }
  if (_instNumber != -1) _count3d.erase(_count3d.find(_instNumber));

  setChanged();
  notifyUnregisterObservers();

  AObject *curs = Cursor::currentCursor();
  if (curs)
  {
    GLComponent *gc = curs->glAPI();
    if (gc)
      gc->clearLists();
  }

  delete d;
}

void AWindow3D::setGLWidgetCreator(GLWidgetCreator c)
{
  glWidgetCreator() = c;
}

AWindow::Type AWindow3D::type() const
{
  if (viewType() == ThreeD)
    return (WINDOW_3D);
  else
    return (WINDOW_2D);
}

AWindow::SubType AWindow3D::subtype() const
{
  switch (viewType())
  {
    case Axial:
      return (AXIAL_WINDOW);
      break;
    case Coronal:
      return (CORONAL_WINDOW);
      break;
    case Sagittal:
      return (SAGITTAL_WINDOW);
      break;
    case ThreeD:
      return ((SubType) 0);
      break;
    default:
      return (OBLIQUE_WINDOW);
  }
}

void AWindow3D::polish()
{
  QAWindow::ensurePolished();
  createTitle();
}

const set<unsigned> & AWindow3D::typeCount() const
{
  return (_count3d);
}

set<unsigned> & AWindow3D::typeCount()
{
  return (_count3d);
}

const string & AWindow3D::baseTitle() const
{
  switch (d->viewtype)
  {
    case Axial:
      return (AWindow3D_axialTitle);
      break;
    case Coronal:
      return (AWindow3D_coronalTitle);
      break;
    case Sagittal:
      return (AWindow3D_sagittalTitle);
      break;
    case ThreeD:
      return (AWindow3D_3DTitle);
      break;
    default:
      return (_baseTitle);
  }
}

namespace
{

  using anatomist::Referential;

  void printPositionAndValue(AObject* obj, const Referential* wref,
      const Point3df & wpos, const vector<float> & tpos, unsigned indent)
  {
    if (obj->isMultiObject())
    {
      unsigned i;
      for (i = 0; i < indent; ++i)
        cout << ' ';
      cout << obj->name() << " :\n";
      const MObject *mo = (const MObject *) obj;
      MObject::const_iterator im, em = mo->end();
      for (im = mo->begin(); im != em; ++im)
        printPositionAndValue(*im, wref, wpos, tpos, indent + 2);
    }
    else if (obj->hasTexture())
    {
      Point3df pos;
      vector<float> vals;
      unsigned i, n;

      vector<float> fpos( 3 );
      fpos[0] = wpos[0];
      fpos[1] = wpos[1];
      fpos[2] = wpos[2];
      fpos.insert( fpos.end(), tpos.begin(), tpos.end() );
      vals = obj->texValues( fpos, wref );
      for (i = 0; i < indent; ++i)
        cout << ' ';
      cout << obj->name() << " : ";
      pos = anatomist::Transformation::transform( wpos, wref,
          obj->getReferential(), Point3df(1, 1, 1),
          Point3df( obj->voxelSize() ) );
      fpos[0] = pos[0];
      fpos[1] = pos[1];
      fpos[2] = pos[2];
      if( obj->Is2DObject() )
      {
        cout << "(" << (short) rint( fpos[0] );
        for( i=1, n=fpos.size(); i<n; ++i )
          cout << ", " << (short) rint( fpos[i] );
        cout << ")";
      }
      else
      {
        cout << "(" << fpos[0];
        for( i=1, n=fpos.size(); i<n; ++i )
          cout << ", " << fpos[i];
        cout << ")";
      }
      cout << " -> (";
      for (i = 0, n = vals.size(); i < n; ++i)
        cout << " " << vals[i];
      cout << " )" << endl;
    }
  }

}

void AWindow3D::printPositionAndValue()
{
  using carto::shared_ptr;

  list<shared_ptr<AObject> >::iterator obj;

  for (obj = _objects.begin(); obj != _objects.end(); ++obj)
    ::printPositionAndValue(obj->get(), getReferential(), _position, _timepos,
                            0);
}

void AWindow3D::updateObject2D(AObject* obj, PrimList* pl,
    ViewState::glSelectRenderMode selectmode)
{
  if (!pl) pl = &d->primitives;
  SliceViewState st(_timepos, true, _position, &d->slicequat,
                    getReferential(),
      windowGeometry(), &d->draw->quaternion(), this, selectmode);
  obj->render(*pl, st);
}

void AWindow3D::updateObject3D(AObject* obj, PrimList* pl,
    ViewState::glSelectRenderMode selectmode)
{
  if (!pl) pl = &d->primitives;
  obj->render(*pl, ViewState(_timepos, this, selectmode));
}

void AWindow3D::updateObject(AObject* obj, PrimList* pl,
    ViewState::glSelectRenderMode selectmode)
{
  unsigned l1 = 0, l2;

  if (pl)
    l1 = pl->size();
  else
    l1 = d->primitives.size();

  GLPrimitives gp;
  if (!obj->Is2DObject() || (d->viewtype == ThreeD && obj->Is3DObject()))
    updateObject3D(obj, &gp, selectmode);
  else
    updateObject2D(obj, &gp, selectmode);

  // perform lists modifications
  list<ObjectModifier *>::iterator im, em = d->objmodifiers.end();
  for (im = d->objmodifiers.begin(); im != em; ++im)
    (*im)->modify(obj, gp);

  if (pl)
    pl->insert(pl->end(), gp.begin(), gp.end());
  else
    d->primitives.insert(d->primitives.end(), gp.begin(), gp.end());

  if (pl)
    l2 = pl->size();
  else
    l2 = d->primitives.size();
  if (l2 > l1) d->tmpprims[obj] = pair<unsigned, unsigned> (l1, l2);
}

void AWindow3D::freeResize()
{
  /*cout << "freeResize - DA size : " << d->draw->width() << " x "
   << d->draw->height() << endl;*/
  QGraphicsView *gv = dynamic_cast<QGraphicsView *>(
    d->draw->qglWidget()->parent() );
  if( gv )
    gv->setMinimumSize( QSize(0, 0) );
  d->draw->qglWidget()->setMinimumSize(QSize(0, 0));
}

void AWindow3D::refreshNow()
{
  using carto::shared_ptr;

  sortPolygons();

  switch (d->refreshneeded)
  {
    case Private::LightRefresh:
      refreshLightViewNow();
      return;
    case Private::TempRefresh:
      // partial refresh: update only temp objects
      refreshTempNow();
      return;
    default:
      break;
  }
  // cout << "AWindow3D::refreshNow\n";
  //ControlledWindow::refreshNow();	// common parts

  //	delete tmp objects primitives
  d->tmpprims.clear();
  updateWindowGeometry(); // do this only in special cases ?

  list<shared_ptr<AObject> >::iterator i;

  Point3df vs, bmin, bmax;
  vector<float> bbmin, bbmax;

  boundingBox( bbmin, bbmax );
  bmin = Point3df( bbmin[0], bbmin[1], bbmin[2] );
  bmax = Point3df( bbmax[0], bbmax[1], bbmax[2] );
  if (d->needsextrema)
  {
    d->draw->setExtrema(bmin, bmax);
    d->needsextrema = false;
  }

  Geometry *geom = windowGeometry();

  Point4dl dmin;
  Point4dl dmax;

  if (geom)
  {
    dmin = geom->DimMin();
    dmax = geom->DimMax();
  }
  else
  {
    dmin = Point4dl(0, 0, 0, 0);
    dmax = Point4dl(1, 1, 1, 0);
  }

  setupSliceSlider();
  setupTimeSlider( bbmin, bbmax );
  if (d->askedsize.width() >= 0 && d->askedsize.height() > 0)
  {
    resizeView(d->askedsize.width(), d->askedsize.height());
    d->askedsize = QSize(0, 0);
  }

  d->draw->qglWidget()->makeCurrent();
  d->deleteLists();

  updateLeftRightAnnotations();

  // Denis: selection
  struct TmpCol
  {
      SelectFactory::HColor diffuse;
      SelectFactory::HColor unlit;
      int mode;
  };

  TmpCol *tmpcol = new TmpCol[_objects.size()];
  unsigned u = 0;

  // handle selection
  for (i = _objects.begin(); i != _objects.end(); ++i)
    if (SelectFactory::factory()->isSelected(Group(), i->get()))
    {
      Material & mat = (*i)->GetMaterial();
      SelectFactory::HColor col = SelectFactory::factory()->highlightColor(
          i->get());
      GLfloat *dif = mat.Diffuse();
      GLfloat *unl = mat.unlitColor();

      TmpCol & tcol = tmpcol[u];
      tcol.diffuse.r = dif[0]; // sauver les vraies couleurs
      tcol.diffuse.g = dif[1];
      tcol.diffuse.b = dif[2];
      tcol.diffuse.a = dif[3];
      tcol.unlit.r = unl[0];
      tcol.unlit.g = unl[1];
      tcol.unlit.b = unl[2];
      tcol.unlit.a = unl[3];
      tcol.mode = mat.renderProperty(Material::RenderMode);
      switch (GraphParams::graphParams()->selectRenderMode)
      {
        case 0:
          dif[0] = col.r;
          dif[1] = col.g;
          dif[2] = col.b;
          if (!col.na) dif[3] = col.a;
          break;
        case 1:
          unl[0] = col.r;
          unl[1] = col.g;
          unl[2] = col.b;
          if (!col.na) unl[3] = col.a;
          mat.setRenderProperty(Material::RenderMode, Material::ExtOutlined);
      }
      (*i)->SetMaterial(mat);
      GLComponent *g = (*i)->glAPI();
      if (g) g->glSetChanged(GLComponent::glMATERIAL);
      ++u;
    }

  list<AObject *> renderobj;
  list<AObject *>::iterator transparent = processRenderingOrder(renderobj);
  list<AObject*>::iterator al, el = renderobj.end();

  //	Rendering mode primitive (must be first)
  GLList *renderpr = new GLList;
  renderpr->generate();
  GLuint renderGLL = renderpr->item();
  if (!renderGLL) AWarning("AWindow3D::Refresh: OpenGL error.");

  glNewList(renderGLL, GL_COMPILE);

  glLineWidth(1);
  if (flatShading())
    glShadeModel( GL_FLAT);
  else
    glShadeModel( GL_SMOOTH);
  if (cullingEnabled())
    glEnable( GL_CULL_FACE);
  else
    glDisable(GL_CULL_FACE);
  if (smoothing())
  {
    glEnable( GL_LINE_SMOOTH);
    glEnable( GL_POLYGON_SMOOTH);
  }
  else
  {
    glDisable( GL_LINE_SMOOTH);
    glDisable( GL_POLYGON_SMOOTH);
  }
  glEnable( GL_LIGHTING);
  glPolygonOffset(0, 0);
  glDisable( GL_POLYGON_OFFSET_FILL);
  if (fog())
  {
    glEnable( GL_FOG);
    glFogi(GL_FOG_MODE, GL_EXP);
    glFogf(GL_FOG_DENSITY, 0.01);
    glFogfv(GL_FOG_COLOR, light()->Background());
  }
  else
    glDisable( GL_FOG);

  switch (renderingMode())
  {
    case Wireframe:
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
      break;
    case HiddenWireframe:
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
      glPolygonOffset(1.05, 1);
      glEnable(GL_POLYGON_OFFSET_FILL);
      break;
    case Material::Outlined:
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
      glPolygonOffset(1.05, 1);
      glEnable(GL_POLYGON_OFFSET_FILL);
    default:
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  }

  glEndList();
  d->primitives.push_back(RefGLItem(renderpr));

  //	Things to do before rendering objects
  Primitive *pr = new Primitive;

  GLuint localGLL = glGenLists(2);
  if (!localGLL) AWarning("AWindow3D::Refresh: OpenGL error.");

  glNewList(localGLL, GL_COMPILE);

  glDisable( GL_BLEND);

  GLdouble plane[4];
  Point3df dir = d->slicequat.transformInverse( Point3df(0, 0, -1) );
  plane[0] = dir[0];
  plane[1] = dir[1];
  plane[2] = dir[2];
  plane[3] = -dir.dot(_position) + d->clipdist;

  switch (clipMode())
  {
    case Single:
      glEnable( GL_CLIP_PLANE0);
      glDisable( GL_CLIP_PLANE1);
      glClipPlane(GL_CLIP_PLANE0, plane);
      // glLightModeli( GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE );
      break;
    case Double:
      glEnable(GL_CLIP_PLANE0);
      glEnable(GL_CLIP_PLANE1);
      glClipPlane(GL_CLIP_PLANE0, plane);
      plane[0] *= -1;
      plane[1] *= -1;
      plane[2] *= -1;
      plane[3] = dir.dot(_position) + d->clipdist;
      glClipPlane(GL_CLIP_PLANE1, plane);
      // glLightModeli( GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE );
      break;
    default:
      glDisable(GL_CLIP_PLANE0);
      glDisable(GL_CLIP_PLANE1);
      // glLightModeli( GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE );
      break;
  }

  glEndList();

  pr->insertList(localGLL);
  d->primitives.push_back(RefGLItem(pr));

  //	Draw opaque objects
  for (al = renderobj.begin(); al != transparent; ++al)
    updateObject(*al);

  //	Settings between opaque and transparent objects
  GLPrimitives curspl = cursorGLL();

  Primitive *pr2 = 0;

  /*if( !blended.empty() )
   {*/
  d->draw->setTransparentObjects(true);
  //if( !pr2 )
  pr2 = new Primitive;
  pr2->insertList(localGLL + 1);
  glNewList(localGLL + 1, GL_COMPILE);
  glEnable(GL_BLEND);
  if (!transparentZEnabled()) glDepthMask( GL_FALSE); // don't write in z-buffer
  glEndList();
  /*}
   else
   {
   glDeleteLists( localGLL+1, 1 );
   d->draw->setTransparentObjects( false );
   }*/

  if (pr2) d->primitives.push_back(RefGLItem(pr2));

  //	Draw transparent objects
  for (al = transparent; al != el; ++al)
    updateObject(*al);

  //	Settings after transparent objects
  Primitive *pr3 = 0;

  if (!transparentZEnabled())
  {
    pr3 = new Primitive;

    GLuint zGLL = glGenLists(1);
    if (!zGLL)
    {
      cerr << "AWindow3D: OpenGL error.\n";
    }
    else
    {
      glNewList(zGLL, GL_COMPILE);
      glDepthMask( GL_TRUE); // write again in z-buffer
      glEndList();
      pr3->insertList(zGLL);
    }
  }

  if (clipMode() != NoClip)
  {
    if (!pr3) pr3 = new Primitive;

    GLuint clipGLL = glGenLists(1);
    if (!clipGLL)
    {
      cerr << "AWindow3D: OpenGL error.\n";
    }
    else
    {
      glNewList(clipGLL, GL_COMPILE);
      glDisable( GL_CLIP_PLANE0);
      glDisable( GL_CLIP_PLANE1);
      glEndList();

      pr3->insertList(clipGLL);
    }
  }

  if (pr3) d->primitives.push_back(RefGLItem(pr3));
  //	draw the cursor a second time after transparent objects
  //	(since it's not written in Z-buffer, it always appears behind)
  /*  if( !curspl.empty() )
   d->primitives.insert( d->primitives.end(), curspl.begin(), curspl.end() );
   */

  /*	Finish rendering mode operations: restore initial modes
   and eventually performs a second rendering of all objects */
  Primitive *renderoffpr = 0;
  bool rendertwice = false;
  switch (renderingMode())
  {
    case HiddenWireframe:
    {
      renderoffpr = new Primitive;
      GLuint hwfGLL = glGenLists(1);
      if (!hwfGLL)
      {
        cerr << "AWindow3D: not enough OGL memory.\n";
        delete renderoffpr;
        renderoffpr = 0;
      }
      else
      {
        glNewList(hwfGLL, GL_COMPILE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        // glLineWidth( 2 );
        glEndList();

        renderoffpr->insertList(hwfGLL);
      }
      rendertwice = true;
    }
      break;
    case Material::Outlined:
    {
      renderoffpr = new Primitive;
      GLuint hwfGLL = glGenLists(1);
      if (!hwfGLL)
      {
        cerr << "AWindow3D: not enough OGL memory.\n";
        delete renderoffpr;
        renderoffpr = 0;
      }
      else
      {
        glNewList(hwfGLL, GL_COMPILE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        //glLineWidth( 2 );
        //glShadeModel( GL_FLAT );
        glDisable(GL_LIGHTING);
        // glColor3f( 0, 0, 0 );
        glEndList();

        renderoffpr->insertList(hwfGLL);
      }
      rendertwice = true;
    }
      break;
    default:
      break;
  }
  if (renderoffpr) d->primitives.push_back(RefGLItem(renderoffpr));

  if (rendertwice)
  {
    // copy whole primitives list except first and last (render modes)
    unsigned i, n = d->primitives.size() - 2;
    PrimList::iterator ip = d->primitives.begin();
    for (++ip, i = 0; i < n; ++i, ++ip)
      d->primitives.push_back(*ip);
  }

  d->draw->setPrimitives(d->primitives);
  d->primitives.clear(); // delete local references

  if (d->light)
  {
    d->draw->setLightGLList(d->light->getGLList());
    d->draw->setBackgroundAlpha(d->light->Background(3));
  }
  else
    d->draw->setBackgroundAlpha(1.);

  ResetRefreshFlag();

  d->draw->updateGL();

  // Denis : selection (suite)

  u = 0;
  for (i = _objects.begin(); i != _objects.end(); ++i)
    if (SelectFactory::factory()->isSelected(Group(), i->get()))
    {
      Material & mat = (*i)->GetMaterial();
      TmpCol & tcol = tmpcol[u];
      GLfloat *dif = mat.Diffuse();
      GLfloat *unl = mat.unlitColor();

      dif[0] = tcol.diffuse.r; // remettre les vraies couleurs
      dif[1] = tcol.diffuse.g;
      dif[2] = tcol.diffuse.b;
      dif[3] = tcol.diffuse.a;
      unl[0] = tcol.unlit.r;
      unl[1] = tcol.unlit.g;
      unl[2] = tcol.unlit.b;
      unl[3] = tcol.unlit.a;

      mat.setRenderProperty(Material::RenderMode, tcol.mode);
      (*i)->SetMaterial(mat);
      GLComponent *g = (*i)->glAPI();
      if (g) g->glSetChanged(GLComponent::glMATERIAL);
      ++u;
    }
  delete[] tmpcol;
  // fin de modif

  showReferential();

  emit refreshed();
}

void AWindow3D::showReferential()
{
  paintRefLabel(d->reflabel, d->refdirmark, getReferential());
}

void AWindow3D::updateLeftRightAnnotations()
{
    if ( !d->orientAnnot )
    {
        return;
    }

    d->orientAnnot->update();
}

void AWindow3D::displayClickPoint()
{
  cout << "displayClickPoint\n";
}

int AWindow3D::computeNearestVertexFromPolygonPoint(const ViewState & vs, int poly,
      const GLComponent* glc, const Point3df & position,
      Point3df & positionNearestVertex)
{
  int index_nearest_vertex = -1;

  const GLfloat *avert = glc->glVertexArray(vs);
  const Point3df *vert = reinterpret_cast<const Point3df *> (avert);
  unsigned npoly = glc->glNumPolygon(vs);
  unsigned polsize = glc->glPolygonSize(vs);
  unsigned v, i;

  //compute the nearest polygon vertex
  float min, dist_min = FLT_MAX;

  if (poly >= 0 && (unsigned) poly < npoly)
  {
    const GLuint *apoly = glc->glPolygonArray(vs);
    for (i = 0; i < polsize; ++i)
    {
      v = apoly[poly * polsize + i];
      positionNearestVertex = vert[v];
      min = (position - positionNearestVertex).norm2();

      if (min < dist_min)
      {
        dist_min = min;
        index_nearest_vertex = v;
      }
    }
    positionNearestVertex = vert[index_nearest_vertex];

    if (theAnatomist->userLevel() >= 3)
    {
      cout << "3D point picked= " << position << "\n";
      cout << "ID polygon selected = " << poly << "\n";
      cout << "index nearest vertex = " << index_nearest_vertex << "\n";
      cout << "3D coord vertex value = " << positionNearestVertex << "\n";
    }
  }
  return index_nearest_vertex;
}

void AWindow3D::getInfos3DFromClickPoint(int x, int y, Point3df & position,
    int *poly, AObject *objselect, string & objtype, vector<float> & texvalue,
    string & textype,
    Point3df & positionNearestVertex, int* indexNearestVertex,
    vector<string> & texlabels )
{
  d->draw->positionFromCursor(x, y, position);

  *poly = polygonAtCursorPosition(x, y, objselect);

  vector<float> fpos = getFullPosition();
  fpos[0] = position[0];
  fpos[1] = position[1];
  fpos[2] = position[2];

  getInfos3DFromPosition( fpos, *poly, objselect, objtype, texvalue,
                          textype, positionNearestVertex, indexNearestVertex,
                          texlabels );
}


void AWindow3D::getInfos3DFromPosition( const vector<float> & fpos,
    int poly, AObject *objselect, string & objtype, vector<float> & texvalue,
    string & textype,
    Point3df & positionNearestVertex, int* indexNearestVertex,
    vector<string> & texlabels )
{
  // cout << "getInfos3DFromPosition, obj: " << ( objselect ? objselect->name() : "<no object>" ) << ", poly: " << poly << ", pos: " << fpos[0] << ", " << fpos[1] << ", " << fpos[2] << ", " << fpos[3] << endl;

  *indexNearestVertex = -1;

  if( !objselect )
    return;

  objtype = objselect->objectTypeName( objselect->type() );

  if( !hasObject(objselect) )
    return;
  GLComponent *glc = objselect->glAPI();
  if( !glc )
    return;

  float dist = -1;
  objselect->nearestVertex( fpos, indexNearestVertex, &dist, -1, 0, false,
                            poly );
  texvalue = objselect->texValues( fpos, getReferential(), poly );
  objselect->getTextureLabels( texvalue, texlabels, textype );
}

bool AWindow3D::positionFromCursor(int x, int y, Point3df & position)
{
  bool res = d->draw->positionFromCursor(x, y, position);
  d->mouseX = x;
  d->mouseY = y;
  return (res);
}


string AWindow3D::displayInfoAtClickPositionAsText( int x, int y, bool html )
{
  string text;

  AObject* obj = objectAtCursorPosition( x, y );
  if( !obj )
    return text;

  int poly = -1, vert = -1;
  vector<float> texval;
  vector<string> texlabels;
  Point3df pos, posvert;
  string objtype, textype;
  getInfos3DFromClickPoint( x, y, pos, &poly, obj, objtype, texval, textype,
                            posvert, &vert, texlabels );
  stringstream txt;
  string lf, coma = ", ";
  if( html )
  {
    lf = "<br/>";
    coma = "<br/>";
  }
  if( vert >= 0 )
    txt << "poly: " << poly << coma << "vert: " << vert << coma << "tex: ";

  bool first = true;
  vector<float>::const_iterator iv, ev = texval.end();
  vector<string>::const_iterator il, el = texlabels.end();
  for( iv=texval.begin(), il=texlabels.begin(); iv!=ev; ++iv )
  {
    if( first )
      first = false;
    else
      txt << ", ";
    txt << *iv;
    if( il != el )
    {
      txt << ": " << *il;
      ++il;
    }
  }

  text = txt.str();
  return text;
}


void AWindow3D::displayInfoAtClickPosition( int x, int y )
{
  if( !d->objvallabel || d->objvallabel->text() != "" )
    return;
  string text = displayInfoAtClickPositionAsText( x, y );
  d->objvallabel->setText( text.c_str() );
}


View* AWindow3D::view()
{
  return (d->draw);
}

const View* AWindow3D::view() const
{
  return (d->draw);
}

void AWindow3D::resizeView()
{
  //	piggy way quick-designed dialog...

  QDialog rv( this );
  rv.setObjectName( "resize dialog" );
  rv.setModal( true );
  rv.setWindowTitle( tr( "Resize window" ) );
  QVBoxLayout *l = new QVBoxLayout(&rv);
  l->setContentsMargins( 5, 5, 5, 5 );
  l->setSpacing( 5 );
  l->addWidget( new QLabel( tr("New window size :"), &rv ) );
  QWidget *hb = new QWidget( &rv );
  l->addWidget( hb );
  QHBoxLayout *hlay = new QHBoxLayout( hb );
  hlay->setSpacing( 10 );
  hlay->setContentsMargins( 0, 0, 0, 0 );
  QSize sz = d->draw->qglWidget()->size();
  QLineEdit *xed = new QLineEdit(QString::number(sz.width()), hb);
  hlay->addWidget( xed );
  hlay->addWidget( new QLabel("x", hb) );
  QLineEdit *yed = new QLineEdit(QString::number(sz.height()), hb);
  hlay->addWidget( yed );
  QWidget *hb2 = new QWidget( &rv );
  l->addWidget( hb2 );
  hlay = new QHBoxLayout( hb2 );
  hlay->setSpacing( 10 );
  hlay->setContentsMargins( 0, 0, 0, 0 );
  QPushButton *ok = new QPushButton(tr("OK"), hb2 );
  hlay->addWidget( ok );
  QPushButton *cc = new QPushButton(tr("Cancel"), hb2 );
  hlay->addWidget( cc );
  ok->setDefault(true);
  connect( ok, SIGNAL( clicked() ), &rv, SLOT( accept() ) );
  connect( cc, SIGNAL( clicked() ), &rv, SLOT( reject() ) );

  if( rv.exec() )
    resizeView( xed->text().toInt(), yed->text().toInt() );
}

void AWindow3D::askZoom()
{
  float z = d->draw->zoom();
  QSize sz = d->draw->qglWidget()->size();
  Geometry *g = windowGeometry();
  Point3df gs = Point3df(1, 1, 1);
  Point4dl dim = Point4dl(sz.width(), sz.height(), 1, 1);

  if (g && viewType() != ThreeD)
  {
    gs = g->Size();
    dim = g->DimMax() - g->DimMin();
    float zx = z * sz.width() / (gs[0] * dim[0]), zy = z * sz.height() / (gs[1]
        * dim[1]);
    z = zx;
    if (zy > z) z = zy;
  }

  ZoomDialog zd( z, true, theAnatomist->getQWidgetAncestor(), "zoom dialog", 
                 true);

  if (zd.exec())
  {
    float z2 = zd.zoomText().toFloat();
    if (zd.mustResize())
    {
      if( g && viewType() != ThreeD )
      {
        float w = gs[0] * dim[0] * z2, h = gs[1] * dim[1] * z2;
        int scrw = 0, scrh = 0, sxmin = -1, symin = -1;
        QList<QScreen *> screens = qApp->screens();
        QList<QScreen *>::iterator is, es = screens.end();
        QScreen *screen = 0;
        for( is=screens.begin(); is!=es; ++is )
        {
          QRect geom = (*is)->geometry();
          if( sxmin < 0 || geom.left() < sxmin )
            sxmin = geom.left();
          if( symin < 0 || geom.top() < symin )
            symin = geom.top();
          if( scrw < geom.right() )
            scrw = geom.right();
          if( scrh < geom.bottom() )
            scrh = geom.bottom();
        }
        if( scrw != 0 )
        {
          scrw -= sxmin;
          scrh -= symin;
        }
        else
        {
          scrw = 2000;
          scrh = 1600;
        }

        z2 = 1;
        if (w > scrw)
        {
          z2 = w / scrw;
          w = scrw;
        }
        if (h > scrh)
        {
          z2 = h / scrh;
          h = scrh;
        }

        d->draw->setZoom(z2);
        resizeView((int) w, (int) h);
      }
      else
      {
        resizeView((int) (z2 / z * sz.width()), (int) (z2 / z * sz.height()));
      }
    }
    else
    {
      d->draw->setZoom(z2);
      refreshLightViewNow();
    }
  }
}

float AWindow3D::getZoom() const
{
  return d->draw->zoom();
}

void AWindow3D::resizeEvent( QResizeEvent * event )
{
  refreshLightViewNow();

  ControlledWindow::resizeEvent( event );
}

void AWindow3D::resizeView(int w, int h)
{
  /*
   if( parent() != 0 )	// don't try this if we are already in a layout
   return;
   */

    int scrw = 0, scrh = 0, sxmin = -1, symin = -1;
    QList<QScreen *> screens = qApp->screens();
    QList<QScreen *>::iterator is, es = screens.end();
    QScreen *screen = 0;
    for( is=screens.begin(); is!=es; ++is )
    {
      QRect geom = (*is)->geometry();
      if( sxmin < 0 || geom.left() < sxmin )
        sxmin = geom.left();
      if( symin < 0 || geom.top() < symin )
        symin = geom.top();
      if( scrw < geom.right() )
        scrw = geom.right();
      if( scrh < geom.bottom() )
        scrh = geom.bottom();
    }
    if( scrw != 0 )
    {
      scrw -= sxmin;
      scrh -= symin;
    }
    else
    {
      scrw = 2000;
      scrh = 1600;
    }

  if( w > scrw )
    w = scrw;
  if( h > scrh )
    h = scrh;

  QSize s = QSize(w, h);
  if (d->draw->qglWidget()->size() != s)
  {
    QGraphicsView *gv = dynamic_cast<QGraphicsView *>(
      d->draw->qglWidget()->parent() );
    if( gv )
    {
      AGraphicsView *agv = dynamic_cast<AGraphicsView *>( gv );
      if( agv )
        agv->setSizeHint( s );
      gv->setMinimumSize( s );
      resize( s );
      gv->resize( s );
    }
    d->draw->setPreferredSize( s.width(), s.height() );
    d->draw->qglWidget()->setMinimumSize( s );
    d->draw->setMinimumSizeHint( s );
    d->draw->qglWidget()->updateGeometry();
    QTimer::singleShot( 500, this, SLOT(freeResize()) );
  }
}

void AWindow3D::setupTimeSlider( const vector<float> & bmin,
                                 const vector<float> & bmax )
{
  float timestep = windowGeometry()->stepSize()[3];
  d->slidt->setMinimum( (int) rint( bmin[3] / timestep ) );
  d->slidt->setMaximum( (int) rint( bmax[3] / timestep ) );
  int t = (int) rint( getTime() / timestep );
  if (d->slidt->value() != t)
  {
    d->slidt->setValue(t);
    d->timelabel->setText(QString::number(t));
  }
  if( bmin[3] >= bmax[3] )
    d->timepanel->hide();
  else
    d->timepanel->show();
  //cout << "show time\n";

  unsigned i, n = bmax.size();
  if( n - 3 > _timepos.size() )
  {
    unsigned m = _timepos.size();
    _timepos.resize( n - 3 );
    for( i=m; i<n - 3; ++i )
      _timepos[i] = 0;
  }
  else if( n - 3 < _timepos.size() )
    _timepos.resize( n - 3 );

  QSlider *slider;
  for( i=4; i<n; ++i )
  {
    if( d->sliderpanels.size() < i - 1 )
    {
      // create a new slider
      QWidget *hb = findChild<QWidget *>( "horiz_widget" );
      QWidget *panel = new QWidget( hb );
      d->sliderpanels.push_back( panel );
      hb->layout()->addWidget( panel );
      QVBoxLayout *vlay = new QVBoxLayout( panel );
      vlay->setSpacing(0);
      vlay->setContentsMargins( 0, 0, 0, 0 );
      QLabel *label = new QLabel( "0", panel );
      d->slicelabels.push_back( label );
      label->setFixedWidth(30);
      vlay->addWidget( label );
      stringstream name;
      name << "sliderX" << i;
      slider
        = new NoDragSlider( i-2, 0, 0, 1, 0, Qt::Vertical, d->timepanel,
                            name.str().c_str() );
      d->sliders.push_back( slider );
      slider->setFixedWidth( slider->sizeHint().width() );
      vlay->addWidget( slider );
      panel->setSizePolicy( QSizePolicy( QSizePolicy::Fixed,
                                         QSizePolicy::Expanding ) );
      slider->setInvertedAppearance( true );
      slider->setInvertedControls( true );
      connect( slider, SIGNAL( myValueChanged( int, int ) ),
               this, SLOT( changeTimeSliders( int, int ) ) );
    }
    slider = d->sliders[ i - 2 ];
    timestep = windowGeometry()->stepSize()[i];
    slider->setMinimum( (int) rint( bmin[i] / timestep ) );
    slider->setMaximum( (int) rint( bmax[i] / timestep ) );
    d->sliderpanels[ i - 2 ]->show();
  }

  // delete additional sliders which are not useful any longer
  for( n=d->sliderpanels.size() + 2; i<n; ++i )
  {
    delete d->sliderpanels[ d->sliderpanels.size() - 1 ];
    d->sliders.erase( d->sliders.begin() + ( d->sliders.size() - 1 ) );
    d->sliderpanels.erase( d->sliderpanels.begin()
                           + ( d->sliderpanels.size() - 1 ) );
    d->slicelabels.erase( d->slicelabels.begin()
                          + ( d->slicelabels.size() - 1 ) );
  }
}

void AWindow3D::setupSliceSlider(float mins, float maxs)
{
  // cout << "setupSliceSlider : " << mins << " - " << maxs << endl;
  if( d->slids->minimum() != (int) mins || d->slids->maximum() != (int) maxs )
  {
    d->slids->blockSignals(true);
    d->slids->setMinimum((int) mins);
    d->slids->setMaximum((int) maxs);
    d->slids->blockSignals(false);
  }

  updateSliceSlider();

  if ( /*d->viewtype == ThreeD ||*/mins >= maxs)
  {
    d->slicepanel->hide();
    updateGeometry();
  }
  else
  {
    d->slicepanel->show();
    updateGeometry();
  }
  // d->needssliceslider = false;
}

void AWindow3D::setupSliceSlider()
{
  float mins, maxs;
  Geometry *geom = windowGeometry();

  if( geom )
  {
    mins = 0;
    maxs = geom->DimMax()[2] - geom->DimMin()[2] - 1;
  }
  else
  {
    mins = 0;
    maxs = 0;
  }

  setupSliceSlider(mins, maxs);
}


void AWindow3D::changeTimeSliders( int slider_num, int value )
{
  int sl = slider_num - 1;
  float timestep = windowGeometry()->stepSize()[sl + 3];
  if( timestep == 0.f )
    timestep = 1.f;
  if( value != (int) (_timepos[ sl ] / timestep ) )
  {
    float tvalue = value * timestep;
    d->slicelabels[slider_num]->setText( QString::number( value ) );

    _timepos[sl] = tvalue;
    SetRefreshFlag();

    if( d->linkonslider )
    {
      vector<float> p(3);
      p.reserve( 3 + _timepos.size() );
      Point3df pos = getPosition();
      p[0] = pos[0];
      p[1] = pos[1];
      p[2] = pos[2];
      p.insert( p.end(), _timepos.begin(), _timepos.end() );
      LinkedCursorCommand *c = new LinkedCursorCommand( this, p );
      theProcessor->execute(c);
    }
    else
    {
      Refresh();
    }

    emit sliderChanged( sl + 3, value );
  }
}


void AWindow3D::changeTime(int t)
{
  changeTimeSliders( 1, t );
}


int AWindow3D::updateSliceSlider()
{
  Point3df pos = getPosition();
  int sl;
  Geometry *geom = windowGeometry();
  Point3df vs;
  float minsl = 0;

  if (geom)
  {
    vs = geom->Size();
    minsl = geom->DimMin()[2];
  }
  else
    vs = Point3df(1, 1, 1);

  // get slice plane
  Point3df norm = d->slicequat.transformInverse(Point3df(0, 0, 1));
  float ds = norm.dot(pos);

  sl = (int) rint(ds / vs[2] - minsl);
  //cout << "slice : " << sl << endl;

  /* if( sl < d->slids->minValue() )
   {
   cerr << "slider value : " << sl << " below minimum\n";
   //sl = d->slids->minValue();
   }
   if( sl > d->slids->maxValue() )
   {
   cerr << "slider value : " << sl << " above maximum\n";
   //sl = d->slids->maxValue();
   }*/

  if (d->slids->value() != sl)
  {
    d->slids->setValue(sl);
    /*cout << "S slider change : " << d->slids->value() << " -> "
     << sl << endl;*/
  }
  QString ns = QString::number(sl);
  if (d->slicelabel->text() != ns) d->slicelabel->setText(ns);

  return (sl);
}

void AWindow3D::changeSlice(int s)
{
  // cout << "changeSlice " << this << ": " << s << endl;
  Point3df pos = getPosition();
  Geometry *geom = windowGeometry();
  Point3df vs;
  float minsl = 0;

  if (geom)
  {
    vs = geom->Size();
    minsl = vs[2] * geom->DimMin()[2];
  }
  else
    vs = Point3df(1, 1, 1);

  // get slice plane
  Point3df norm = d->slicequat.transformInverse( Point3df(0, 0, 1) );
  // project
  float ds = -norm.dot(pos);
  pos = pos + (vs[2] * s + minsl + ds) * norm;

  // cout << "new pos : " << pos << endl;

  if (pos != _position)
  {
    // cout << "rechange pos\n";
    //d->slicelabel->setText( QString::number( updateSliceSlider() ) );
    int olds = int( rint( ( norm.dot( _position ) - minsl ) / vs[2] ) );

    if( d->linkonslider && olds != s )
    {
      vector<float> p(4);
      p[0] = pos[0];
      p[1] = pos[1];
      p[2] = pos[2];
      p[3] = getTime();
      LinkedCursorCommand *c = new LinkedCursorCommand(this, p);
      theProcessor->execute(c);
    }
    else
    {
      _position = pos;
      updateSliceSlider();
      Refresh();
    }
  }

  int dim = 2;
  if( norm[1] > norm[2] && norm[1] > norm[0] )
    dim = 1;
  else if( norm[0] > norm[1] && norm[0] > norm[2] )
    dim = 0;
  emit( sliderChanged( dim, s ) );
}

void AWindow3D::changeReferential()
{
  set<AWindow *> sw;
  sw.insert(this);
  ChooseReferentialWindow *w 
    = new ChooseReferentialWindow(sw, "Choose Referential Window");
  w->setAttribute( Qt::WA_DeleteOnClose );
  w->show();
}

void AWindow3D::unregisterObject(AObject* o)
{
  d->tmpprims.erase(o);
  d->renderconstraints.erase(o);
  map<AObject *, Private::ConstrainedObject>::iterator i =
      d->renderconstraints.begin(), e = d->renderconstraints.end(), j;
  while (i != e)
  {
    if (i->second.second == o)
    {
      j = i;
      ++i;
      d->renderconstraints.erase(j);
    }
    else
      ++i;
  }

  removeFromAutoFusion2D( o );

  anatomist::Referential *r1 = getReferential();
  if (r1)
  {
    anatomist::Referential *r2 = o->getReferential();
    if (r2) ATransformSet::instance()->unregisterObserver(r1, r2, this);
  }

  ControlledWindow::unregisterObject(o);
}

void AWindow3D::registerObject(AObject* o, bool temporaryObject, int pos)
{
  if( hasObject( o ) )
    return;

  bool reorient = ( viewType() != AWindow3D::ThreeD
                    && !isViewOblique() );
  bool fst = _objects.empty();
  ControlledWindow::registerObject(o, temporaryObject, pos);

  if( autoFusion2D( o ) )
  {
    Fusion2D *autoFusion = static_cast<Fusion2D *>( d->autoFusion.get() );
    if( autoFusion->size() + 1 == _objects.size() )
    {
      // the only "visible" object is the auto-fusion: use it to setup
      // bounding box and window size
      fst = true;
      o = autoFusion;
    }
  }

  vector<float> bmin, bmax;

  anatomist::Referential *r1 = getReferential();
  if (r1)
  {
    anatomist::Referential *r2 = o->getReferential();
    if (r2) ATransformSet::instance()->registerObserver(r1, r2, this);
  }

  if (!temporaryObject)
  {
    d->needsextrema = true;

    if( fst )
    {
      boundingBox( bmin, bmax );
      d->draw->setExtrema( Point3df( bmin[0], bmin[1], bmin[2] ),
                           Point3df( bmax[0], bmax[1], bmax[2] ) );
      d->needsextrema = false;
      updateWindowGeometry();
      setupSliceSlider();

      set<AWindow*> wg = theAnatomist->getWindowsInGroup(Group());
      bool setpos = true;
      if (wg.size() > 1)
      {
        set<AWindow*>::const_iterator iw, ew = wg.end();
        for (iw = wg.begin(); iw != ew; ++iw)
          if (*iw != this && dynamic_cast<const AWindow3D *> (*iw))
          {
            set<AObject *> objs = (*iw)->Objects();
            if( !objs.empty() )
            {
              setPosition((*iw)->getPosition(), (*iw)->getReferential());
              setpos = false;
              break;
            }
          }
      }
      if (setpos)
      {
        // set cursor in middle of object
        bmin[0] = ( bmin[0] + bmax[0] ) * 0.5f;
        bmin[1] = ( bmin[1] + bmax[1] ) * 0.5f;
        bmin[2] = ( bmin[2] + bmax[2] ) * 0.5f;
        setPosition( bmin, getReferential() );
      }
      setTime(0);

      // resize window if in 2D mode
      if( o->Is2DObject() && isWindow() )
      {
        float wf = 1.5;
        theAnatomist->config()->getProperty("windowSizeFactor", wf);

        vector<float> mo, Mo;
        switch (viewType())
        {
          case Axial:
            o->boundingBox2D( mo, Mo );
            d->askedsize = QSize( (int) ( wf * ( Mo[0] - mo[0] ) ),
                (int) ( wf * (Mo[1] - mo[1] ) ) );
            break;
          case Coronal:
            o->boundingBox2D( mo, Mo );
            d->askedsize = QSize( (int) ( wf * ( Mo[0] - mo[0] ) ),
                (int) ( wf * ( Mo[2] - mo[2] ) ) );
            break;
          case Sagittal:
            o->boundingBox2D( mo, Mo );
            d->askedsize = QSize( (int) ( wf * (Mo[1] - mo[1] ) ),
                (int) ( wf * (Mo[2] - mo[2] ) ) );
            break;
          default:
            break;
        }
      }
      if( reorient )
        setViewType( viewType() ); // reorient according to transforms
    }
    Refresh();
  }
  else
    refreshTemp();
}


bool AWindow3D::inAutoFusion( const anatomist::AObject *obj ) const
{
  if( !d->autoFusion )
    return false;
  const Fusion2D *autoFusion
    = static_cast<const Fusion2D *>( d->autoFusion.get() );
  MObject::const_iterator io, eo = autoFusion->end();
  for( io=autoFusion->begin(); io!=eo; ++io )
    if( *io == obj )
      return true;
  return false;
}


bool AWindow3D::autoFusion2D( AObject *obj )
{
  if( obj == d->autoFusion.get() || obj->Is3DObject() )
    return false;
  GLComponent *gl = obj->glAPI();
  if( !gl || !gl->sliceableAPI() )
    return false;
  vector<AObject *> vobj;
  if( d->autoFusion )
  {
    // if there is a autoFusion, take its children
    Fusion2D * autoFusion = static_cast<Fusion2D *>( d->autoFusion.get() );
    MObject::iterator io, eo = autoFusion->end();
    for( io=autoFusion->begin(); io!=eo; ++io )
      vobj.push_back( *io );
  }
  else
  {
    /* if there was no autoFusion yet, look in already displayed objects
       if there was a Sliceable */
    set<AObject *>::iterator iso, eso = _sobjects.end();
    for( iso=_sobjects.begin(); iso!=eso; ++iso )
      if( *iso != obj && !(*iso)->Is3DObject() && !isTemporary( *iso )
          && (*iso)->glAPI() && (*iso)->glAPI()->sliceableAPI() )
      {
        vobj.push_back( *iso );
        // there should be only 1, otherwise the autoFusion would already
        // exist
        break;
      }
  }
  if( vobj.empty() )
    return false;
  // of course we add obj to the list
  vobj.push_back( obj );

  set<AObject *> sobj;
  sobj.insert( vobj.begin(), vobj.end() );
  Fusion2dMethod m;
  if( !m.canFusion( sobj ) )
    return false;
  string fusionName;
  if( d->autoFusion )
  {
    fusionName = d->autoFusion->name();
    unregisterObject( d->autoFusion.get() );
    d->autoFusion.reset( 0 );

    // some objects may have been deleted and unregistered during the above
    // operation. Re-check all
    vector<AObject *>::iterator iv, iv2, ev = vobj.end();
    bool changed = false;
    for( iv=vobj.begin(); iv!=ev; )
    {
      if( theAnatomist->hasObject( *iv ) || *iv == obj )
        ++iv;
      else
      {
        iv2 = iv;
        ++iv;
        vobj.erase( iv2 );
        changed = true;
      }
    }
    if( changed )
    {
      sobj.clear();
      sobj.insert( vobj.begin(), vobj.end() );
      if( !m.canFusion( sobj ) )
        return false;
    }
  }
  d->autoFusion.reset( m.fusion( vobj ) );
  static_cast<Fusion2D *>( d->autoFusion.get() )->setDynamic( true );
  if( fusionName.empty() )
    fusionName = theAnatomist->makeObjectName( "auto_fusion" );
  theAnatomist->registerObjectName( fusionName, d->autoFusion.get() );
  d->autoFusion->setName( fusionName );
  theAnatomist->registerObject( d->autoFusion.get(), false );
  theAnatomist->releaseObject( d->autoFusion.get() );
  ControlledWindow::registerObject( d->autoFusion.get() );

  return true;
}


void AWindow3D::removeFromAutoFusion2D( AObject *obj )
{
  // cout << "removeFromAutoFusion2D: " << obj->name() << endl;
  if( !d->autoFusion || obj == d->autoFusion.get() )
    return;
  GLComponent *gl = obj->glAPI();
  if( !gl || !gl->sliceableAPI() )
    return;
  vector<AObject *> vobj;
  bool found = false;
  Fusion2D *autoFusion = static_cast<Fusion2D *>( d->autoFusion.get() );
  MObject::iterator io, eo = autoFusion->end();
  for( io=autoFusion->begin(); io!=eo; ++io )
    if( *io == obj )
      found = true;
    else
      vobj.push_back( *io );
  if( !found )
    return;
  unregisterObject( autoFusion );
  d->autoFusion.reset( 0 );
  if( vobj.size() >= 2 )
  {
    Fusion2dMethod m;
    d->autoFusion.reset( m.fusion( vobj ) );
    static_cast<Fusion2D *>( d->autoFusion.get() )->setDynamic( true );
    theAnatomist->registerObject( d->autoFusion.get(), false );
    registerObject( d->autoFusion.get() );
  }
}


void AWindow3D::setViewType(ViewType t)
{
  static const float c = 1. / sqrt(2.);
  bool has_changed = false;

  if( d->viewtype != t )
  {
    d->viewtype = t;
    updateViewTypeToolBar();
    has_changed = true;
  }
  // handle point of view
  switch (t)
  {
    case Axial:
      d->draw->setQuaternion(axialQuaternion(getReferential()));
      d->slicequat = Point4df(0, 0, 0, 1);
      focusView();
      break;
    case Coronal:
      d->draw->setQuaternion(coronalQuaternion(getReferential()));
      d->slicequat = Point4df(c, 0, 0, c);
      focusView();
      break;
    case Sagittal:
      d->draw->setQuaternion(sagittalQuaternion(getReferential()));
      d->slicequat = Point4df(-0.5, -0.5, -0.5, 0.5);
      focusView();
      break;
    default:
      break;
  }

  if( has_changed )
    emit orientationChanged();
}


AWindow3D::ViewType AWindow3D::viewType() const
{
  return (d->viewtype);
}

bool AWindow3D::isViewOblique() const
{
    const Point4df quat_vector = d->draw->quaternion().vector();
    if (quat_vector != axialQuaternion(getReferential()).vector() &&
        quat_vector != coronalQuaternion(getReferential()).vector() &&
        quat_vector != sagittalQuaternion(getReferential()).vector())
    {
        return true;
    }
    static const float c = 1. / sqrt(2.);
    const Point4df & sl_vec = d->slicequat.vector();
    if( sl_vec != Point4df(0, 0, 0, 1) && sl_vec != Point4df(c, 0, 0, c)
        && sl_vec != Point4df(-0.5, -0.5, -0.5, 0.5) )
      return true;
    return false;
}

void AWindow3D::muteAxial()
{
  setViewType( Axial);
  updateControls();
  theAnatomist->NotifyWindowChange(this);
  Refresh();
}

void AWindow3D::muteCoronal()
{
  setViewType( Coronal);
  updateControls();
  theAnatomist->NotifyWindowChange(this);
  Refresh();
}

void AWindow3D::muteSagittal()
{
  setViewType( Sagittal);
  updateControls();
  theAnatomist->NotifyWindowChange(this);
  Refresh();
}

void AWindow3D::muteOblique()
{
  setViewType( Oblique);
  updateControls();
  theAnatomist->NotifyWindowChange(this);
  Refresh();
}

void AWindow3D::mute3D()
{
  setViewType( ThreeD);
  updateControls();
  theAnatomist->NotifyWindowChange(this);
  Refresh();
}

void AWindow3D::updateViewTypeToolBar()
{
  if (toolBarsVisible())
  {
    d->axialbt->setChecked(d->viewtype == Axial);
    d->coronalbt->setChecked(d->viewtype == Coronal);
    d->sagittalbt->setChecked(d->viewtype == Sagittal);
    d->obliquebt->setChecked(d->viewtype == Oblique);
    d->threedbt->setChecked(d->viewtype == ThreeD);
  }
}

GLPrimitives AWindow3D::cursorGLL() const
{
  GLPrimitives curspl;
  if (hasCursor())
  {
    AObject *curs = Cursor::currentCursor();
    if (curs)
    {
      GLComponent *gc = curs->glAPI();
      if (gc)
      {
        ViewState vs(0);
        // cursor color
        Material mat = curs->GetMaterial();
        if (!useDefaultCursorColor())
        {
          AimsRGB rgb = cursorColor();
          mat.SetDiffuse(((float) rgb[0]) / 255, ((float) rgb[1]) / 255,
              ((float) rgb[2]) / 255, mat.Diffuse(3));
        }
        else
          mat.SetDiffuse(224. / 255, 0, 0, mat.Diffuse(3));
        curs->SetMaterial(mat);

        curspl = gc->glMainGLL(vs);
        if (!curspl.empty())
        {
          bool dr = getReferential() && getReferential()->isDirect();
          Point3df pos = getPosition();
          GLList *posl = new GLList;
          posl->generate();
          glNewList(posl->item(), GL_COMPILE);
          GLfloat mat[16];
          GLfloat scl = ((GLfloat) cursorSize()) / 20;

          // write 4x4 matrix in column
          mat[0] = scl;
          mat[1] = 0;
          mat[2] = 0;
          mat[3] = 0;
          mat[4] = 0;
          mat[5] = scl;
          mat[6] = 0;
          mat[7] = 0;
          mat[8] = 0;
          mat[9] = 0;
          mat[10] = scl;
          mat[11] = 0;
          mat[12] = pos[0];
          mat[13] = pos[1];
          mat[14] = pos[2];
          mat[15] = 1;

          if (dr) glFrontFace( GL_CCW);
          glMatrixMode( GL_MODELVIEW);
          glPushMatrix();
          glMultMatrixf(mat);
          glPushAttrib(GL_LIGHTING_BIT | GL_LINE_BIT | GL_CURRENT_BIT
              | GL_DEPTH_BUFFER_BIT);
          //glDepthMask( GL_FALSE ); // don't write cursor in z-buffer
          glEndList();

          curspl.insert(curspl.begin(), RefGLItem(posl));

          posl = new GLList;
          posl->generate();
          glNewList(posl->item(), GL_COMPILE);
          glPopAttrib();
          glMatrixMode(GL_MODELVIEW);
          glPopMatrix();
          if (dr) glFrontFace( GL_CW);
          glEndList();
          curspl.push_back(RefGLItem(posl));

          GLPrimitives::iterator i, e = curspl.end();
          for (i = curspl.begin(); i != e; ++i)
            (*i)->setGhost(true);

          d->primitives.insert(d->primitives.end(), curspl.begin(),
              curspl.end());
        }
      }
    }
  }
  return curspl;
}

void AWindow3D::updateWindowGeometry()
{
  Geometry *g = new Geometry(setupWindowGeometry(_objects, d->slicequat,
      getReferential(), d->draw->qglWidget(), clipMode() != NoClip ) );
  setWindowGeometry(g);
}

Geometry AWindow3D::setupWindowGeometry(
    const list<carto::shared_ptr<AObject> > & objects, const Quaternion & slicequat,
#ifdef ANA_USE_QOPENGLWIDGET
    const anatomist::Referential *wref, QOpenGLWidget* glw, bool with3d )
#else
    const anatomist::Referential *wref, QGLWidget* glw, bool with3d )
#endif
{
  // cout << "setupWindowGeometry, objects: " << objects.size() << endl;
  using carto::shared_ptr;

  list<shared_ptr<AObject> >::const_iterator obj;
  bool first = true, firsttex = true;
  Point3df p, dmin, dmax;
  vector<float> pmin, pmax, size, s2, vst, tmin, tmax;
  vector<int> dimMin(3, 0), dimMax(3, 1);
  anatomist::Referential *oref;
  AObject *o;
  anatomist::Transformation *tr;
  Point3df u, v, w;

  size = vector<float>( 3, 1.f );
  vst = vector<float>( 3, 1.f );

  // local macro
#define check_extremum( p )				\
	  if( tr )					\
	    p = tr->transform( p );			\
	  p = slicequat.transform( p );  		\
	  if( p[0] < dmin[0] )				\
	    dmin[0] = p[0];				\
	  if( p[1] < dmin[1] )				\
	    dmin[1] = p[1];				\
	  if( p[2] < dmin[2] )				\
	    dmin[2] = p[2];				\
	  if( p[0] > dmax[0] )				\
	    dmax[0] = p[0];				\
	  if( p[1] > dmax[1] )				\
	    dmax[1] = p[1];				\
	  if( p[2] > dmax[2] )				\
	    dmax[2] = p[2];
  // end macro

  for( obj = objects.begin(); obj != objects.end(); ++obj )
  {
    o = obj->get();

    if( with3d || o->Is2DObject() )
    {
      s2 = o->voxelSize();
      // cout << "    Object " << o->name() << endl;
      while( s2.size() < 3 )
        s2.push_back( 1. );
      vector<float> ovs = s2;

      tr = 0;
      if (wref)
      {
        oref = o->getReferential();
        if (oref) tr = theAnatomist->getTransformation(oref, wref);
      }

      u = Point3df(1, 0, 0);
      v = Point3df(0, 1, 0);
      w = Point3df(0, 0, 1);
      if (tr)
      {
        u = tr->transform(u) - tr->transform(Point3df(0, 0, 0));
        v = tr->transform(v) - tr->transform(Point3df(0, 0, 0));
        w = tr->transform(w) - tr->transform(Point3df(0, 0, 0));
      }
      u = slicequat.transformInverse(u);
      v = slicequat.transformInverse(v);
      w = slicequat.transformInverse(w);
      //cout << "base : " << u << endl << v << endl << w << endl;

      s2[0] = 1. / max( max( fabs(u[0] / s2[0] ), fabs( u[1] / s2[1] ) ),
                        fabs( u[2] / s2[2] ) );
      s2[1] = 1. / max( max( fabs(v[0] / ovs[0]), fabs( v[1] / ovs[1] ) ),
                        fabs(v[2] / ovs[2] ) );
      s2[2] = 1. / max( max( fabs(w[0] / ovs[0]), fabs( w[1] / ovs[1] ) ),
                        fabs( w[2] / ovs[2] ) );

      // check extrema

      if ((*obj)->boundingBox(pmin, pmax))
      {
        // cout << "    boundingbox : " << pmin.size() << " / " << pmax.size() << endl;
        p = Point3df( pmin[0], pmin[1], pmin[2] );
        if (tr) p = tr->transform(p);
        p = slicequat.transform(p);

        if (o->textured2D())
        { // keep voxel resolution of only textured objects
          if (firsttex)
          {
            vst = s2;
            firsttex = false;
          }
          else
          {
            if (s2[0] < vst[0]) vst[0] = s2[0];
            if (s2[1] < vst[1]) vst[1] = s2[1];
            if (s2[2] < vst[2]) vst[2] = s2[2];
          }
        }

        if (first)
        {
          size = s2;
          dmin = dmax = p;
          tmin.insert( tmin.begin(), pmin.begin() + 3, pmin.end() );
          tmax.insert( tmax.begin(), pmax.begin() + 3, pmax.end() );
          first = false;
        }
        else
        {
          if (s2[0] < size[0]) size[0] = s2[0];
          if (s2[1] < size[1]) size[1] = s2[1];
          if (s2[2] < size[2]) size[2] = s2[2];

          if (p[0] < dmin[0]) dmin[0] = p[0];
          if (p[1] < dmin[1]) dmin[1] = p[1];
          if (p[2] < dmin[2]) dmin[2] = p[2];
          for( size_t i=3; i<pmin.size(); ++i )
          {
            if( tmin.size() <= i )
            {
              tmin.push_back( pmin[i] );
              tmax.push_back( pmax[i] );
            }
            else
            {
              if( tmin[i - 3] > pmin[i] )
                tmin[i - 3] = pmin[i];
              if( tmax[i - 3] < pmax[i] )
                tmax[i - 3] = pmax[i];
            }
          }
        }
        if (p[0] > dmax[0]) dmax[0] = p[0];
        if (p[1] > dmax[1]) dmax[1] = p[1];
        if (p[2] > dmax[2]) dmax[2] = p[2];

        p = Point3df(pmax[0], pmin[1], pmin[2]);
        check_extremum( p );
        p = Point3df(pmax[0], pmax[1], pmin[2]);
        check_extremum( p );
        p = Point3df(pmin[0], pmax[1], pmin[2]);
        check_extremum( p );
        p = Point3df(pmin[0], pmin[1], pmax[2]);
        check_extremum( p );
        p = Point3df(pmax[0], pmin[1], pmax[2]);
        check_extremum( p );
        p = Point3df(pmax[0], pmax[1], pmax[2]);
        check_extremum( p );
        p = Point3df(pmin[0], pmax[1], pmax[2]);
        check_extremum( p );
      }
    }
  }

#undef check_extremum

  if (first) // no object
  {
    //size = Point3df( 1, 1, 1 );
    dimMin = vector<int>( 4, 0 );
    dimMax = vector<int>( 4, 1 );
    dimMax[3] = 0;
  }
  else
  {
    /* keep resolution of textured objects in x & y directions,
     and of all 2D objects in z direction */
    size[0] = vst[0];
    size[1] = vst[1];

    dimMin[0] = (int) ::ceil(dmin[0] / size[0]);
    dimMin[1] = (int) ::ceil(dmin[1] / size[1]);
    dimMin[2] = (int) ::ceil(dmin[2] / size[2]);
    dimMax[0] = (int) rint((dmax[0] - dmin[0]) / size[0]) + dimMin[0];
    dimMax[1] = (int) rint((dmax[1] - dmin[1]) / size[1]) + dimMin[1];
    dimMax[2] = (int) rint((dmax[2] - dmin[2]) / size[2]) + dimMin[2];
    int i, n = tmin.size();
    for( i=0; i<n; ++i )
    {
      dimMin.push_back( int( ::ceil( tmin[i] / size[i+3] ) ) );
      dimMax.push_back( int( rint( ( tmax[i] - tmin[i] ) / size[i+3] ) )
                        + dimMin[i] );
    }
  }

  /*cout << "new win geometry : vs : " << size << endl;
   cout << "dimmin : " << dimMin << endl;
   cout << "dimmax : " << dimMax << " ( " << dimMax - dimMin << " )" << endl;*/

  // check if dimensions can be handled by GL textures
  static GLint dimmax = 0;
  if( dimmax == 0 )
  {
    if (!glw) glw = GLWidgetManager::sharedWidget();
    glw->makeCurrent();
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &dimmax);
  }

  if (dimMax[0] - dimMin[0] >= dimmax)
  {
    cerr << "warning: window geometry too small (dimx : " << dimMax[0]
        - dimMin[0] << ", max: " << dimmax << ")\n";
    float scl = ((float) dimMax[0] - dimMin[0] + 1) / dimmax;
    size[0] *= scl;
    dimMin[0] = (int) (dimMin[0] / scl);
    dimMax[0] = dimMin[0] + dimmax;
  }
  if (dimMax[1] - dimMin[1] > dimmax)
  {
    cerr << "warning: window geometry too small (dimy : " << dimMax[1]
        - dimMin[1] << ", max: " << dimmax << ")\n";
    float scl = ((float) dimMax[1] - dimMin[1] + 1) / dimmax;
    size[1] *= scl;
    dimMin[1] = (int) (dimMin[1] / scl);
    dimMax[1] = dimMin[1] + dimmax;
  }
  if (dimMax[2] - dimMin[2] > dimmax)
  {
    cerr << "warning: window geometry too small (dimz : " << dimMax[2]
        - dimMin[2] << ", max: " << dimmax << ")\n";
    float scl = ((float) dimMax[1] - dimMin[1] + 1) / dimmax;
    size[2] *= scl;
    dimMin[2] = (int) (dimMin[2] / scl);
    dimMax[2] = dimMin[2] + dimmax;
  }

  return Geometry( size, dimMin, dimMax );
}

namespace
{

  AObject* objectToShow(const AWindow3D* w, const set<AObject *> & objs)
  {
    AObject *obj = 0;
    SelectFactory *sf = SelectFactory::factory();
    unsigned nobj = 0, nsel = 0;

    set<AObject *> found;
    set<AObject *>::const_iterator io, eo = objs.end(), jo, ejo = found.end(),
      tjo;
    for (io = objs.begin(); io != eo; ++io)
      if( (*io)->hasTexture() )
      {
        if( obj )
        {
          // check if one is parent of the other
          MObject *mobj = dynamic_cast<MObject *>( *io );
          if( obj->parents().find( mobj ) != obj->parents().end() )
          {
            // io is parent of obj: take it preferably
            found.erase( found.find( obj ) );
            // if io is parent to other already found objects, then remove them
            obj = *io;
            for( jo=found.begin(); jo!=ejo; )
            {
              if( (*jo)->parents().find( mobj ) != (*jo)->parents().end() )
              {
                tjo = jo;
                ++jo;
                found.erase( tjo );
                --nobj;
              }
              else
                ++jo;
            }
            found.insert( obj );
          }
          else if( (*io)->parents().find( dynamic_cast<MObject *>( obj ) )
                    == (*io)->parents().end() )
          {
            // no parental link: probably too many objects
            ++ nobj;
            found.insert( *io );
          }
          // else obj is parent: keep it as is
        }
        else
        {
          ++nobj;
          found.insert( *io );
        }
        if( !obj )
          obj = *io;
        if( sf->isSelected( w->Group(), *io ) )
        {
          if( nsel > 0 )
            return 0; // ambiguity
          ++nsel;
          obj = *io;
        }
      }
    if( nsel == 1 || ( nsel == 0 && nobj == 1 ) )
      return obj;
    return 0;
  }

}


void AWindow3D::setPosition( const Point3df& position,
                             const anatomist::Referential* orgref )
{
  vector<float> pos( 3 );
  pos[0] = position[0];
  pos[1] = position[1];
  pos[2] = position[2];
  setPosition( pos, orgref );
}


void AWindow3D::setPosition( const vector<float> & position,
                             const anatomist::Referential* orgref )
{
  anatomist::Transformation *tra = 0;
  if( orgref )
    tra = theAnatomist->getTransformation( orgref, getReferential() );
  Point3df pos( position[0], position[1], position[2] );

  if (tra)
    pos = tra->transform( pos );

  Geometry *geom = windowGeometry();

  if( geom && viewType() != ThreeD )
  { // snap to nearest slice
    Point3df dir = d->slicequat.transformInverse(Point3df(0, 0, 1));
    float z = dir.dot(pos) / geom->Size()[2];

    /* compare to current position, and avoid changing if it is just half way
       between slices */
    Point3df curpos = _position;
    // _position is already in our referential
//     if (tra)
//       curpos = tra->transform( curpos );
    float cz = dir.dot( curpos ) / geom->Size()[2];
    cz = rint( cz );
    if( abs( z - cz ) <= 0.6 ) // snap to current slice
      pos += float( cz - z ) * geom->Size()[2] * dir;
    else // further than that: snap to other slice
      pos += float( rint(z) - z ) * geom->Size()[2] * dir;
  }

  bool haschanged = false;

  if( pos != _position )
  {
    _position = pos;
    SetRefreshFlag();
    haschanged = true;
    updateSliceSlider();
  }

  if( position.size() > 3 )
  {
    vector<float> tpos( position.begin() + 3, position.end() );
    if( tpos != _timepos )
    {
      _timepos = tpos;
      SetRefreshFlag();
      haschanged = true;
      updateTimeSliders();
    }
  }

  if( haschanged )
  {
    // status bar
    QStatusBar *sb = statusBar();
    QString spos = "( "
      + QString::number( _position[0] )
      + ", " + QString::number( _position[1] ) + ", "
      + QString::number( _position[2] );
    unsigned i, n = _timepos.size();
    for( i=0; i<n; ++i )
      spos += ", " + QString::number( _timepos[i] );
    sb->showMessage( tr("cursor position: ") + spos + " )" );
    // object value
    AObject *obj = objectToShow(this, _sobjects);
    if (obj)
    {
      string objtype;
      vector<float> vals;
      // = obj->texValues( getFullPosition(), getReferential() );
      string textype;
      Point3df pvert;
      int vindex;
      vector<string> texlabels;
      getInfos3DFromPosition( getFullPosition(), -1, obj, objtype, vals,
                              textype, pvert, &vindex, texlabels );

      QString txt;
      unsigned i, n = vals.size();
      for (i = 0; i < n; ++i)
      {
        if (i != 0) txt += ", ";
        txt += QString::number(vals[i]);
        if( texlabels.size() > i )
          txt += QString( ": " ) + QString( texlabels[i].c_str() );
      }
      if( d->objvallabel )
        d->objvallabel->setText(txt);
    }
    else if( d->objvallabel )
      d->objvallabel->setText("");
  }
}

void AWindow3D::setViewPoint(float *quaternion, const float zoom)
{
  d->draw->setZoom(zoom);
  d->draw->setQuaternion( Point4df( quaternion ) );
  refreshLightViewNow();
}

void AWindow3D::setLight(const Light & l)
{
  if (!d->light)
    d->light = new Light(l);
  else
    *d->light = l;
}

Light* AWindow3D::light()
{
  return (d->light);
}

void AWindow3D::setOrientationCube(bool state)
{
  d->orientationCube = state;
}

bool AWindow3D::hasOrientationCube() const
{
  return (d->orientationCube);
}

void AWindow3D::setBoundingFrame(bool state)
{
  d->boundingFrame = state;
}

bool AWindow3D::hasBoundingFrame() const
{
  return (d->boundingFrame);
}

void AWindow3D::setRenderingMode(RenderingMode mode)
{
  d->renderingMode = mode;
}

AWindow3D::RenderingMode AWindow3D::renderingMode() const
{
  return (d->renderingMode);
}

void AWindow3D::lightView()
{
  if (!d->lightview) d->lightview = new LightModelWindow(this);
  d->lightview->show();
  d->lightview->raise();
}

void AWindow3D::pointsOfView()
{
  if (!d->poview) d->poview = new FixedPointOfViewWindow(this, 0,
      "Fixed points of view");
  d->poview->show();
  d->poview->raise();
}

void AWindow3D::tools()
{
  if (!d->tools) d->tools = new Tools3DWindow(this);
  d->tools->show();
  d->tools->raise();
}

bool AWindow3D::surfpaintIsVisible()
{
  return d->surfpaintState;
}

void AWindow3D::setVisibleSurfpaint(bool b)
{
  d->surfpaintState = b;
}

bool AWindow3D::constraintEditorIsActive()
{
  return d->constraintEditorState;
}

void AWindow3D::setActiveConstraintEditor(bool b)
{
  d->constraintEditorState = b;
}

std::vector<string> AWindow3D::getConstraintList()
{
  return d->constraintList;
}

int AWindow3D::getConstraintType() const
{
  return d->constraintType;
}

AObject* AWindow3D::getConstraintTexture()
{
  return d->texConstraint;
}

void AWindow3D::loadConstraintData( const std::vector<string> & constraintList,
                                    int constraintType, AObject *texConstraint )
{
  d->constraintList = constraintList;
  d->constraintType = constraintType;
  d->texConstraint = texConstraint;
}

void AWindow3D::syncViews(bool keepextrema)
{
  AWindow3D *w2;
  set<AWindow *> win = theAnatomist->getWindows();
  set<AWindow *>::const_iterator iw, fw = win.end();
  GLWidgetManager *da = d->draw, *da2;
  Point2df tr;
  anatomist::Referential *ref = getReferential();

  for (iw = win.begin(); iw != fw; ++iw)
  {
    if (*iw != this && (*iw)->Group() == Group())
    {
      w2 = dynamic_cast<AWindow3D *> (*iw);
      if (w2 && w2->viewType() == ThreeD)
      {
        anatomist::Transformation *tr = 0;
        da2 = w2->d->draw;
        if (!keepextrema)
        {
          tr = theAnatomist->getTransformation( ref, w2->getReferential() );
          da2->setAutoCentering(false);
          Point3df bmin = da->windowBoundingMin(),
            bmax = da->windowBoundingMax();
          if( tr )
            tr->transformBoundingBox( da->windowBoundingMin(),
                                      da->windowBoundingMax(), bmin, bmax );
          //da2->setExtrema( da->boundingMin(), da->boundingMax() );
          da2->setWindowExtrema( bmin, bmax );
        }
        da2->setZoom(da->zoom());
        da2->setQuaternion(da->quaternion());
        if (!keepextrema)
        {
          Point3df rcent = da->rotationCenter();
          if( tr )
            rcent = tr->transform( rcent );
          da2->setRotationCenter( rcent );
        }
        w2->refreshLightViewNow();
      }
    }
  }
}


bool AWindow3D::boundingBox( vector<float> & bmin,
                             vector<float> & bmax ) const
{
  using carto::shared_ptr;

  bool valid = false;
  bool timevalid = false;

  bmin.resize( 4 );
  bmax.resize( 4 );

  //	determine objects extrema
  if( _objects.empty() )
  {
    bmin[0] = 0.f;
    bmin[1] = 0.f;
    bmin[2] = 0.f;
    bmin[3] = 0.f;
    bmax[0] = 1.f;
    bmax[1] = 1.f;
    bmax[2] = 1.f;
    bmax[3] = 0.f;
    return false;
  }
  else
  {
    list<shared_ptr<AObject> >::const_iterator i, e = _objects.end();
    AObject *obj;
    vector<float> pmin, pmax;
    Point3df ppmino, ppmaxo, ppmin, ppmax;
    anatomist::Referential *wref = getReferential(), *oref;
    anatomist::Transformation *tr;
    unsigned j, n, m = 4;

    bmin[0] = FLT_MAX;
    bmin[1] = FLT_MAX;
    bmin[2] = FLT_MAX;
    bmin[3] = FLT_MAX;
    bmax[0] = -FLT_MAX;
    bmax[1] = -FLT_MAX;
    bmax[2] = -FLT_MAX;
    bmax[3] = -FLT_MAX;

    for( i=_objects.begin(); i != e; ++i )
    {
      obj = i->get();
      if( isTemporary( obj ) )
        continue;

      if( obj->boundingBox( pmin, pmax ) )
      {
        if( wref && (oref = obj->getReferential() )
          && ( tr = theAnatomist->getTransformation( oref, wref ) ) )
        {
          ppmino = Point3df( pmin );
          ppmaxo = Point3df( pmax );
          tr->transformBoundingBox( ppmino, ppmaxo, ppmin, ppmax );
          pmin[0] = ppmin[0];
          pmin[1] = ppmin[1];
          pmin[2] = ppmin[2];
          pmax[0] = ppmax[0];
          pmax[1] = ppmax[1];
          pmax[2] = ppmax[2];
        }

        if (valid)
        {
          for( j=0, n=std::min( unsigned( pmin.size() ), m ); j<n; ++j )
          {
            if( pmin[j] < bmin[j] )
              bmin[j] = pmin[j];
            if( pmax[j] > bmax[j] )
              bmax[j] = pmax[j];
          }
          while( pmin.size() > m )
          {
            bmin.push_back( pmin[m] );
            bmax.push_back( pmax[m] );
            ++m;
          }
        }
        else
        {
          if( !timevalid )
          {
            bmin = pmin;
            bmax = pmax;
          }
          else
          {
            bmin[0] = pmin[0];
            bmin[1] = pmin[1];
            bmin[2] = pmin[2];
            bmax[0] = pmax[0];
            bmax[1] = pmax[1];
            bmax[2] = pmax[2];
          }
        }
        valid = true;
        timevalid = true;
      }
      else if( pmax.size() >= 3 )
      {
        cout << "obj bbox false but time bounds\n";
        /* some objects (Textures) return false for boundingbox() because they
           have no geometric (3D) bounds, but still have time bounds */
        timevalid = true;
        for( j=3, n=std::min( unsigned( pmin.size() ), m ); j<n; ++j )
        {
          if( pmin[j] < bmin[j] )
            bmin[j] = pmin[j];
          if( pmax[j] > bmax[j] )
            bmax[j] = pmax[j];
        }
        while( pmin.size() > m )
        {
          bmin.push_back( pmin[m] );
          bmax.push_back( pmax[m] );
          ++m;
        }
      }
    }
  }

  // avoid flat bboxes
  for( unsigned j=0; j<3; ++j )
    if( bmin[j] == bmax[j] )
    {
      unsigned k = ( j + 1 ) % 3, l = ( j + 2 ) % 3;
      float d = std::min( bmax[k] - bmin[k], bmax[l] - bmin[l] );
      if( d == 0. )
        d = std::max( bmax[k] - bmin[k], bmax[l] - bmin[l] );
      bmin[j] -= d * 0.01;
      bmax[j] += d * 0.01;
    }

  // d->needsboundingbox = false;
  return valid;
}

void AWindow3D::focusView()
{
  vector<float> bmin, bmax;

  if( boundingBox( bmin, bmax ) )
  {
    d->draw->setExtrema( Point3df( bmin[0], bmin[1], bmin[2] ),
                         Point3df( bmax[0], bmax[1], bmax[2] ) );
    d->needsextrema = false;
    d->draw->setZoom(1.);
    d->draw->setAutoCentering(true);
    Refresh();
  }
}

void AWindow3D::toolsWinDestroyed()
{
  d->tools = 0;
}

void AWindow3D::povWinDestroyed()
{
  d->poview = 0;
}

void AWindow3D::lightWinDestroyed()
{
  d->lightview = 0;
  //cout << "AWindow3D::lightWinDestroyed\n";
}

bool AWindow3D::perspectiveEnabled() const
{
  return (d->draw->perspectiveEnabled());
}

void AWindow3D::enablePerspective(bool state)
{
  d->draw->enablePerspective(state);
}

void AWindow3D::setAutoRotationCenter()
{
  d->draw->setAutoCentering(true);
  refreshLightViewNow();
}

const Quaternion & AWindow3D::sliceQuaternion() const
{
  return (d->slicequat);
}

void AWindow3D::setSliceQuaternion(const Quaternion & q)
{
  d->slicequat = q;
  d->refreshneeded = Private::FullRefresh;
}

void AWindow3D::setSliceOrientation( const Point3df & normal )
{
  Point3df pos, v1, v2, v3, r;
  float    x, y, angle;

  v3 = normal;
  v3.normalize();
  r = Point3df( 0, 0, 1 );
  v1 = crossed( r, v3 );
  if( v1.norm2() < 0.0001 )
  {
    v1 = Point3df( 1, 0, 0 );
    v2 = crossed( v1, r );
  }
  else
  {
    v1.normalize();
    v2 = crossed( v1, r );
  }
  r = crossed( v1, v2 );
  x = -r.dot( v3 );
  y = -v2.dot( v3 );
  angle = acos( x );
  if( y < 0 )
    angle *= -1;
  d->slicequat.fromAxis( v1, angle );
}

AWindow3D::ClipMode AWindow3D::clipMode() const
{
  return (d->clipmode);
}

void AWindow3D::setClipMode(ClipMode m)
{
  d->clipmode = m;
  setChanged();
}

float AWindow3D::clipDistance() const
{
  return (d->clipdist);
}

void AWindow3D::setClipDistance(float dis)
{
  d->clipdist = dis;
}

bool AWindow3D::transparentZEnabled() const
{
  return (d->transpz);
}

void AWindow3D::enableTransparentZ(bool x)
{
  d->transpz = x;
}

bool AWindow3D::cullingEnabled() const
{
  return (d->culling);
}

void AWindow3D::setCulling(bool x)
{
  d->culling = x;
}

bool AWindow3D::flatShading() const
{
  return (d->flatshading);
}

void AWindow3D::setFlatShading(bool x)
{
  d->flatshading = x;
}

bool AWindow3D::smoothing() const
{
  return (d->smooth);
}

void AWindow3D::setSmoothing(bool x)
{
  d->smooth = x;
}

bool AWindow3D::fog() const
{
  return (d->fog);
}

void AWindow3D::setFog(bool x)
{
  d->fog = x;
}


void AWindow3D::Refresh()
{
  d->refreshneeded = Private::FullRefresh;
  ControlledWindow::Refresh();
}

void AWindow3D::refreshLightView()
{
  if (!needsRedraw())
  {
    d->refreshneeded = Private::LightRefresh;
    ControlledWindow::Refresh();
  }
}

void AWindow3D::refreshLightViewNow()
{
  using carto::shared_ptr;

  d->refreshneeded = Private::FullRefresh;
  list<shared_ptr<AObject> >::iterator i, e = _objects.end();
  GLWidgetManager *wm = dynamic_cast<GLWidgetManager *>( view() );
  bool dontsortpoly = true;
  if( wm && wm->hasCameraChanged() && d->sortPolygons )
    dontsortpoly = false;

  for( i = _objects.begin();
      i != e && !(*i)->renderingIsObserverDependent()
        && ( dontsortpoly || !(*i)->isTransparent() );
      ++i )
  {
  }
  if (i == e)
  {
    d->draw->updateGL();
    emit refreshed();
  }
  else
    // could be optimized on a by-object basis
    refreshNow();
}

void AWindow3D::refreshTemp()
{
  if (!needsRedraw() || d->refreshneeded == Private::LightRefresh)
  {
    d->refreshneeded = Private::TempRefresh;
    ControlledWindow::Refresh();
  }
}

void AWindow3D::refreshTempNow()
{
  using carto::shared_ptr;

  // cout << "refreshTempNow...\n";

  d->refreshneeded = Private::FullRefresh;

  // const set<AObject *>	& tempobj = temporaryObjects();

  PrimList pl = d->draw->primitives();
  PrimList::iterator ip, ip2, iendobj;

  //cout << "re-order objects...\n";
  //	re-order objects for faster deletion
  map<unsigned, pair<AObject*, unsigned> > to2;
  map<unsigned, pair<AObject*, unsigned> >::iterator io2, eo2;
  map<AObject *, pair<unsigned, unsigned> >::iterator io, eo =
      d->tmpprims.end();

  for (io = d->tmpprims.begin(); io != eo; ++io)
    to2[io->second.first] = pair<AObject *, unsigned> (io->first,
        io->second.second);

  //cout << "done\n";

  //	convert unsigned pointers to iterators
  unsigned i, j, k;
  map<AObject *, pair<PrimList::iterator, PrimList::iterator> > pli;
  map<unsigned, pair<PrimList::iterator, PrimList::iterator> > orphan_chunks;

  // index of end-of-objects as iterator
  ip = pl.begin();
  i = 0;
  k = 0;

  // cout << "convert unsigned to iterators...\n";
  for (io2 = to2.begin(), eo2 = to2.end(); io2 != eo2; ++io2)
  {
    j = io2->first;
    ip2 = ip;
    k = i;
    while (i < j)
    {
      ++ip;
      ++i;
    }
    if (k < j) orphan_chunks[k] = make_pair(ip2, ip);
    ip2 = ip;
    j = io2->second.second;
    while (i < j)
    {
      ++ip;
      ++i;
    }
    pli[io2->second.first] = pair<PrimList::iterator, PrimList::iterator> (ip2,
        ip);
  }
  if (ip != pl.end())
  {
    orphan_chunks[i] = make_pair(ip, pl.end());
  }
  // cout << "done\n";

  list<AObject *> objs;
  list<AObject *>::iterator ioo = objs.begin(), eoo = objs.end(), nexto;
  AObject *ao;
  map<AObject *, pair<PrimList::iterator, PrimList::iterator> >::iterator ipl,
      epl = pli.end();

  processRenderingOrder(objs);

  //	rebuild lists
  // cout << "rebuild lists...\n";

  bool newobj = false;
  i = 0;
  map<AObject *, pair<PrimList::iterator, PrimList::iterator> >::iterator
      nextipl = pli.begin();

  PrimList plnew;
  ip = pl.begin();
  unsigned orphan, plnewlen = 0, oldlen;
  map<unsigned, pair<PrimList::iterator, PrimList::iterator> >::iterator ior =
      orphan_chunks.begin(), eor = orphan_chunks.end();

  // in real rendering order
  for (ioo = objs.begin(); ioo != eoo; ++ioo)
  {
    if( inAutoFusion( *ioo ) )
      continue;

    ao = *ioo;
    // copy orphan lists
    ipl = pli.find(ao);
    if (ipl != epl)
    {
      orphan = d->tmpprims[ipl->first].first;
      while (ior != eor && ior->first < orphan)
      {
        plnew.insert(plnew.end(), ior->second.first, ior->second.second);
        ++ior;
      }
      plnewlen = plnew.size();
    }

    if (isTemporary(ao))
    {
      if (ipl == epl)
      {
        newobj = true;
        // new temp object, copy orphans up to next object
        nexto = ioo;
        ++nexto;
        while (nexto != eoo)
        {
          io = d->tmpprims.find(*nexto);
          if (io != eo) break;
          ++nexto;
        }
        if (nexto != eoo)
        {
          orphan = io->second.first;
          while (ior != eor && ior->first < orphan)
          {
            plnew.insert(plnew.end(), ior->second.first, ior->second.second);
            ++ior;
          }
          plnewlen = plnew.size();
        }
      }

      oldlen = plnewlen;
      updateObject(ao, &plnew);
      plnewlen = plnew.size();
      if (plnewlen == oldlen) d->tmpprims.erase(ao);
    }
    else
    {
      // non-temp object: copy its lists without redrawing
      if (ipl != epl) // already drawn
      {
        oldlen = plnewlen;
        plnew.insert(plnew.end(), ipl->second.first, ipl->second.second);
        plnewlen = plnew.size();
        pair<unsigned, unsigned> & tp = d->tmpprims[ao];
        tp.first = oldlen;
        tp.second = plnewlen;
      }
    }
  }

  if (newobj || !to2.empty())
  {
    // copy end of list
    while (ior != eor)
    {
      plnew.insert(plnew.end(), ior->second.first, ior->second.second);
      ++ior;
    }

    d->draw->setPrimitives(plnew);
    d->draw->updateGL();
  }
  //cout << "refreshTempNow finished\n";
}

int AWindow3D::getSliceSliderPosition()
{
  return (d->slids->value());
}

int AWindow3D::getSliceSliderMaxPosition()
{
  return (d->slids->maximum());
}

int AWindow3D::getTimeSliderPosition()
{
  return (d->slidt->value());
}

int AWindow3D::getTimeSliderMaxPosition()
{
  return (d->slidt->maximum());
}

void AWindow3D::setSliceSliderPosition(int position)
{
  d->slids->setValue(position);
}

void AWindow3D::setTimeSliderPosition(int position)
{
  d->slidt->setValue(position);
}


void AWindow3D::updateTimeSliders()
{
  unsigned i, n = std::min( _timepos.size(), d->sliders.size() - 1 );
  for( i=0; i<n; ++i )
  {
    float timestep = 1.;
    if( windowGeometry() )
    {
      timestep = windowGeometry()->stepSize()[i + 3];
      if( timestep == 0.f )
        timestep = 1.f;
    }

    d->sliders[i+1]->setValue( int( _timepos[i] / timestep ) );
    d->slicelabels[i+1]->setText( QString::number(
      int( _timepos[i] / timestep ) ) );
  }
  n = d->sliders.size() - 1;
  for( ; i<n; ++i )
  {
    d->sliders[i+1]->setValue( 0 );
    d->slicelabels[i+1]->setText( QString::number( 0 ) );
  }
}


void AWindow3D::switchToolbox()
{
  view()->controlSwitch()->switchToolBoxVisible();
}

void AWindow3D::registerObjectModifier(ObjectModifier *mod)
{
  d->objmodifiers.push_back(mod);
}

void AWindow3D::unregisterObjectModifier(ObjectModifier *mod)
{
  list<ObjectModifier *>::iterator e = d->objmodifiers.end(), i = std::find(
      d->objmodifiers.begin(), e, mod);
  if (i != e) d->objmodifiers.erase(i);
}

void AWindow3D::update( const Observable* o, void* arg )
{
#ifdef ANA_DEBUG_UPDATE
  cout << "AWindow3D::update()\n";
#endif
  const AObject *ao = dynamic_cast<const AObject*> (o);
  bool temp = false;
  if( ao )
  {
    if( isTemporary(const_cast<AObject *> (ao)) )
      temp = true;
    if( ao->obsHasChanged(GLComponent::glREFERENTIAL) )
    {
#ifdef ANA_DEBUG_UPDATE
      cout << "object " << ao->name() << " (" << o
      << ") has changed ref in window " << Title() << " ("
      << this << ")\n";
#endif
      const anatomist::Referential *r1 = getReferential(), *r2 = ao->previousReferential();
      if (r1)
      {
        ATransformSet *ts = ATransformSet::instance();
        if (r2) ts->unregisterObserver(r1, r2, this);
        r2 = ao->getReferential();
        if (r2) ts->registerObserver(r1, r2, this);
      }
    }
    updateSaveButtonStatus();
  }
  else
  {
    const TransformationObserver *to =
        dynamic_cast<const TransformationObserver *> (o);
    if (to)
    {
      // try not trigger a full refresh if only local internal (hidden)
      // transformations are involved
      const set<const anatomist::Referential *> & refs = to->referentials();
      set<const anatomist::Referential *>::const_iterator ir, er = refs.end();
      unsigned nothidden = 0;
      bool hidden;
      for (ir = refs.begin(); ir != er; ++ir)
        if (!(*ir)->header().getProperty("hidden", hidden) || !hidden)
        {
          ++nothidden;
          if (nothidden >= 2) break;
        }
      if (nothidden <= 1) temp = true;
    }
  }

  if (temp)
    refreshTemp();
  else
    AWindow::update(o, arg);
}

void AWindow3D::setReferential( anatomist::Referential* ref )
{
  using carto::shared_ptr;

  ATransformSet *ts = ATransformSet::instance();
  anatomist::Referential *old = getReferential(), *r2;
  list<shared_ptr<AObject> >::iterator io, eo = _objects.end();

  if (old)
  // remove all observers for me
  for (io = _objects.begin(); io != eo; ++io)
  {
    r2 = (*io)->getReferential();
    ts->unregisterObserver(old, r2, this);
  }

  ControlledWindow::setReferential(ref);

  if (ref)
  // put back observers to all objects
  for (io = _objects.begin(); io != eo; ++io)
  {
    r2 = (*io)->getReferential();
    ts->registerObserver(ref, r2, this);
  }

  d->draw->setZDirection(ref ? !ref->isDirect() : false);
  updateLeftRightAnnotations();
  // refocus view on objects in the new referential
  focusView();
}

bool AWindow3D::linkedCursorOnSliderChange() const
{
  return d->linkonslider;
}

void AWindow3D::setLinkedCursorOnSliderChange(bool x)
{
  d->linkonslider = x;
}

void AWindow3D::setLinkedCursorPos()
{
  QDialog dial( this );
  dial.setWindowTitle( "Set linked cursor position" );
  dial.setModal( true );
  QVBoxLayout *l = new QVBoxLayout(&dial);
  l->setContentsMargins( 5, 5, 5, 5 );
  l->setSpacing(5);
  QLineEdit *le = new QLineEdit(&dial);
  l->addWidget(le);
  stringstream curpos;
  Point3df pos = getPosition();
  curpos << pos[0] << " " << pos[1] << " " << pos[2];
  le->setText(QString(curpos.str().c_str()));
  le->selectAll();
  QWidget *hb = new QWidget(&dial);
  QHBoxLayout *hlay = new QHBoxLayout( hb );
  l->addWidget(hb);
  hlay->setContentsMargins( 0, 0, 0, 0 );
  hlay->setSpacing( 5 );
  QPushButton *pb = new QPushButton(tr("OK"), hb);
  hlay->addWidget( pb );
  pb->setAutoDefault(true);
  pb->setDefault(true);
  connect(pb, SIGNAL(clicked()), &dial, SLOT(accept()));
  pb = new QPushButton(tr("Cancel"), hb);
  hlay->addWidget( pb );
  connect(pb, SIGNAL(clicked()), &dial, SLOT(reject()));

  if( dial.exec() )
  {
    QString txt = le->text();
    vector<float> nums;
    nums.reserve(4);
    int i, l = 0, m = 0, n = txt.length(), mm;
    bool ok = true;
    for (i = 0; i < 4 && l < n; ++i)
    {
      while( txt[l] == ' ' || txt[l] == '\t' || txt[l] == ',' )
        ++l;
      if( l == n )
        break; // reached the end
      m = txt.indexOf( ' ', l );
      mm = txt.indexOf( ',', l );
      if( mm != -1 )
      {
        if( m == -1 || m > mm )
          m = mm;
      }
      else if( m == -1 )
        m = txt.length();
      QString s = txt;
      s.remove(0, l);
      s.remove(m - l, txt.length() - m);
      // cout << "read: " << s.toStdString().c_str() << endl;
      nums.push_back(s.toFloat(&ok));
      if (!ok)
      {
        cerr << "unable to parse coords in string \"" 
          << txt.toStdString().data() << "\"" << endl;
      }
      l = m;
    }
    n = nums.size();
    if( n >= 1 )
    {
      while( nums.size() < 3 )
        nums.push_back( getPosition()[ nums.size() ] );
      LinkedCursorCommand *c = new LinkedCursorCommand( this, nums );
      theProcessor->execute(c);
    }
    else
      cerr << "not a 3D/4D position" << endl;
  }
}


void AWindow3D::findPrimitive()
{
  AObject *obj = 0;
  SelectFactory *sf = SelectFactory::factory();
  const map<unsigned, set<AObject *> > & sel = sf->selected();
  map<unsigned, set<AObject *> >::const_iterator is = sel.find( Group() );
  bool ambiguous = false;
  if( is != sel.end() )
  {
    const set<AObject *> & so = is->second;
    set<AObject *>::const_iterator io, eo = so.end();
    string s;
    for( io=so.begin(); io!=eo; ++io )
      if( hasObject( *io ) )
      {
        if( obj )
        {
          obj = 0;
          ambiguous = true;
          break;
        }
        else
          obj = *io;
      }
  }
  if( !obj && !ambiguous )
  {
    const set<AObject *> & objs = Objects();
    if( objs.size() == 1 )
      obj = *objs.begin();
  }

  if( !obj )
  {
    statusBar()->showMessage( "select one object", 1500 );
    return;
  }

  rc_ptr<ViewState> vs = viewState();
  GLComponent *glc = obj->glAPI();
  if( !glc || glc->glNumVertex( *vs ) == 0 )
  {
    statusBar()->showMessage( "object has no vertices", 1500 );
    return;
  }

  QDialog dial( this );
  dial.setWindowTitle( "Find vertex, polygon or texture" );
  dial.setModal( true );
  QGridLayout *l = new QGridLayout;
  l->setContentsMargins( 5, 5, 5, 5 );
  l->setSpacing( 5 );
  dial.setLayout( l );

  l->addWidget( new QLabel( "Vertex:" ), 0, 0 );
  l->addWidget( new QLabel( "Poygon:" ), 1, 0 );
  l->addWidget( new QLabel( "Texture value:" ), 2, 0 );

  QLineEdit *vle = new QLineEdit;
  QLineEdit *ple = new QLineEdit;
  QLineEdit *tle = new QLineEdit;
  l->addWidget( vle, 0, 1 );
  l->addWidget( ple, 1, 1 );
  l->addWidget( tle, 2, 1 );
//   tle->setEnabled( false ); // not implemented yet.

  QHBoxLayout *hlay = new QHBoxLayout;
  l->addLayout( hlay, 3, 0, 1, 2 );
  hlay->setContentsMargins( 0, 0, 0, 0 );
  hlay->setSpacing( 5 );
  QPushButton *pb = new QPushButton( tr("OK") );
  hlay->addWidget( pb );
  pb->setAutoDefault( true );
  pb->setDefault( true );
  connect(pb, SIGNAL(clicked()), &dial, SLOT(accept()));
  pb = new QPushButton( tr("Cancel") );
  hlay->addWidget( pb );
  connect(pb, SIGNAL(clicked()), &dial, SLOT(reject()));

  if( dial.exec() )
  {
    if( vle->text() != QString() )
    {
      bool ok = false;
      int v = vle->text().toInt( &ok );
      if( ok && v >= 0 )
        ok = positionToVertex( obj, unsigned( v ) );
      if( !ok || v < 0 )
      {
        statusBar()->showMessage( "incorrect vertex number", 1500 );
        return;
      }
    }
    else if( ple->text() != QString() )
    {
      bool ok = false;
      int p = ple->text().toInt( &ok );
      if( ok && p >= 0 )
        ok = positionToPolygon( obj, unsigned( p ) );
      if( !ok || p < 0 )
      {
        statusBar()->showMessage( "incorrect polygon number", 1500 );
        return;
      }
    }
    else if( tle->text() != QString() )
    {
      unsigned nt = glc->glNumTextures( *vs );
      if( nt == 0 )
      {
        statusBar()->showMessage( "object has no texture", 1500 );
        return;
      }

      bool ok = false;
      float t = tle->text().toFloat( &ok );
      if( !ok )
      {
        statusBar()->showMessage( "incorrect texture value", 1500 );
        return;
      }

      if( !positionToTexture( obj, t ) )
        statusBar()->showMessage( "could not find texture value", 1500 );
    }
  }
}


bool AWindow3D::positionToVertex( const AObject* obj, unsigned vert )
{
  const GLComponent *glc = obj->glAPI();
  if( !glc )
    return false;

  rc_ptr<ViewState> vs = viewState();
  size_t nv = glc->glNumVertex( *vs );
  if( vert >= nv )
    return false;

  const GLfloat* va = glc->glVertexArray( *vs );
  vector<float> vp( 3 );
  vp[0] = va[vert * 3];
  vp[1] = va[vert * 3 + 1];
  vp[2] = va[vert * 3 + 2];

  const anatomist::Referential *oref = obj->getReferential();
  const anatomist::Referential *wref = getReferential();
  anatomist::Transformation *trans = 0;
  if( oref && wref )
    trans = theAnatomist->getTransformation( oref, wref );

  if( trans )
  {
    Point3df tp = trans->transform( Point3df( vp ) );
    vp[0] = tp[0];
    vp[1] = tp[1];
    vp[2] = tp[2];
  }

  LinkedCursorCommand *c = new LinkedCursorCommand( this, vp );
  theProcessor->execute(c);

  return true;
}


bool AWindow3D::positionToPolygon( const AObject* obj, unsigned poly )
{
  const GLComponent *glc = obj->glAPI();
  if( !glc )
    return false;

  rc_ptr<ViewState> vs = viewState();
  size_t np = glc->glNumPolygon( *vs );
  if( poly >= np )
    return false;

  unsigned ps = glc->glPolygonSize( *vs );
  const GLfloat* va = glc->glVertexArray( *vs );
  const GLuint* pa = glc->glPolygonArray( *vs );

  Point3df vp( 0, 0, 0 );
  for( unsigned i=0; i<ps; ++i )
  {
    unsigned ip = pa[poly * ps + i];
    vp += Point3df( va[ip * 3], va[ip * 3 + 1], va[ip * 3 + 2] );
  }
  vp /= ps;

  const anatomist::Referential *oref = obj->getReferential();
  const anatomist::Referential *wref = getReferential();
  anatomist::Transformation *trans = 0;
  if( oref && wref )
    trans = theAnatomist->getTransformation( oref, wref );

  if( trans )
  {
    vp = trans->transform( Point3df( vp ) );
  }

  LinkedCursorCommand *c = new LinkedCursorCommand( this, vp.toStdVector() );
  theProcessor->execute(c);

  return true;
}


bool AWindow3D::positionToTexture( const AObject* obj, float texval )
{
  const GLComponent *glc = obj->glAPI();
  if( !glc )
    return false;

  rc_ptr<ViewState> vs = viewState();
  unsigned nt = glc->glNumTextures( *vs );
  if( nt == 0 )
    return false;

  unsigned dt = glc->glDimTex( *vs, 0 );
  unsigned ts = glc->glTexCoordSize( *vs, 0 );
  const GLfloat* ta = glc->glTexCoordArray( *vs, 0 );
  const GLComponent::TexExtrema & te = glc->glTexExtrema( 0 );

  float unscaled = ( texval - te.minquant[0] )
    / ( te.maxquant[0] - te.minquant[0] );
  cout << "unscaled tex: " << unscaled << endl;
  float eps = 1e-5;
  unsigned i, sel = 0;
  for( i=0; i<ts; ++i )
  {
    float tc = ta[i * dt];
    if( tc == unscaled )
    {
      sel = i;
      break;
    }
    if( std::abs( tc - unscaled ) < eps && sel == 0 )
      sel = i;
  }
  cout << "sel vertex: " << sel << endl;

  return positionToVertex( obj, sel );
}


AWindow3D* AWindow3D::rightEyeWindow()
{
  return d->righteye;
}


AWindow3D* AWindow3D::leftEyeWindow()
{
  return d->lefteye;
}

void AWindow3D::setRightEyeWindow(AWindow3D* w)
{
  if (d->righteye) d->righteye->setLeftEyeWindow(0);
  d->righteye = w;
  d->draw->setRightEye(w ? dynamic_cast<GLWidgetManager *> (w->view()) : 0);
  if (w) w->setLeftEyeWindow(this);
}

void AWindow3D::setLeftEyeWindow(AWindow3D* w)
{
  if (d->lefteye && w) d->lefteye->setRightEyeWindow(0);
  d->lefteye = w;
  d->draw->setLeftEye(w ? dynamic_cast<GLWidgetManager *> (w->view()) : 0);
}

void AWindow3D::openStereoView()
{
  CreateWindowCommand *c = new CreateWindowCommand("3D");
  theProcessor->execute(c);
  AWindow3D *w = static_cast<AWindow3D *> (c->createdWindow());
  setRightEyeWindow(w);
  set<AWindow *> sw;
  sw.insert(w);
  AddObjectCommand *c2 = new AddObjectCommand(Objects(), sw);
  theProcessor->execute(c2);
}

void AWindow3D::showToolBars(int state)
{
  if (state < 0 || state >= 2) state = 1 - toolBarsVisible();
  if (state)
  {
    d->refbox->show();
    if (d->statusbarvisible) statusBar()->show();
  }
  else
  {
    d->refbox->hide();
    statusBar()->hide();
  }
  ControlledWindow::showToolBars(state);
}

void AWindow3D::toggleStatusBarVisibility()
{
  Object p = Object::value(Dictionary());
  set<AWindow *> sw;
  sw.insert(this);
  p->value<Dictionary> ()["show_cursor_position"] = Object::value(2);
  WindowConfigCommand *c = new WindowConfigCommand(sw, *p);
  theProcessor->execute(c);
}

void AWindow3D::showStatusBar(int x)
{
  if (x < 0 || x >= 2) x = !d->statusbarvisible;
  if (x != d->statusbarvisible)
  {
    d->statusbarvisible = x;
    if (x && toolBarsVisible())
      statusBar()->show();
    else
      statusBar()->hide();
  }
}

void AWindow3D::renderAfter(AObject* obj, AObject* after)
{
  if (after)
    d->renderconstraints[obj] = Private::ConstrainedObject(
        Private::RenderAfter, after);
  else
    d->renderconstraints.erase(obj);
}

void AWindow3D::renderBefore(AObject* obj, AObject* before)
{
  if (before)
    d->renderconstraints[obj] = Private::ConstrainedObject(
        Private::RenderBefore, before);
  else
    d->renderconstraints.erase(obj);
}

namespace
{

  GLfloat selectedOpacity(const AWindow3D* win, AObject* obj)
  {
    if (SelectFactory::factory()->isSelected(win->Group(), obj))
    {
      SelectFactory::HColor col = SelectFactory::factory()->highlightColor(obj);

      if (GraphParams::graphParams()->selectRenderMode == 0 && !col.na)
        return col.a;
    }
    GLfloat difop = obj->GetMaterial().Diffuse()[3];
    if( difop < 1. )
      return difop;
    if( obj->isTransparent() )
      return 0.99; // transparent palette
    return 1.; // opaque object
  }

}

list<AObject *>::iterator AWindow3D::processRenderingOrder(
    list<AObject *> & ordered) const
{
  using carto::shared_ptr;

  list<shared_ptr<AObject> >::const_iterator i, e = _objects.end();
  map<AObject *, Private::ConstrainedObject>::iterator ic, ec =
      d->renderconstraints.end();
  AObject *ao;
  multimap<float, AObject *> blended;
  multimap<float, AObject *>::reverse_iterator ib, eb;
  list<AObject *>::iterator transparent = ordered.end();
  map<AObject *, list<AObject *>::iterator> rev;
  map<AObject *, list<AObject *>::iterator>::iterator ir, er = rev.end();
  list<AObject *>::iterator io, eo = ordered.end();
  bool ctrans = false;
  GLfloat op;

  for (i = _objects.begin(); i != _objects.end(); ++i)
  {
    if( inAutoFusion( i->get() ) )
      continue;

    ao = i->get();
    ic = d->renderconstraints.find(ao);
    if (ic == ec) // non-constrained position
    {
      // not constrained: use opacity
      op = selectedOpacity(this, ao);
      if (op < 1.)
        blended.insert(pair<float, AObject *> (op, ao));
      else
      {
        rev[ao] = ordered.insert(eo, ao);
      }
    }
  }
  // append transparent objects
  for (ib = blended.rbegin(), eb = blended.rend(); ib != eb; ++ib)
  {
    rev[ib->second] = ordered.insert(eo, ib->second);
    if (transparent == ordered.end()) transparent = rev[ib->second];
  }

  // now insert constrained objects
  map<AObject *, Private::ConstrainedObject> c1, c2;
  map<AObject *, Private::ConstrainedObject> *constrained =
      &d->renderconstraints, *notdone = &c1;
  do
  {
    if (!notdone->empty())
    {
      constrained = notdone;
      if (notdone == &c1)
        notdone = &c2;
      else
        notdone = &c1;
      notdone->clear();
    }

    for (ic = constrained->begin(), ec = constrained->end(); ic != ec; ++ic)
    {
      ir = rev.find(ic->second.second);
      if (ir == er) // not found: maybe another constrained object
      {
        (*notdone)[ic->first] = ic->second;
      }
      else
      {
        io = ir->second;
        if (ic->second.first == Private::RenderAfter) ++io;
        io = ordered.insert(io, ic->first);
        rev[ic->first] = io;
        if (ic->first->isTransparent()) ctrans = true;
      }
    }
  }
  while (!notdone->empty() && notdone->size() != constrained->size());

  // add randomly remaining objects
  if (!notdone->empty())
  {
    for (ic = notdone->begin(), ec = notdone->end(); ic != ec; ++ic)
    {
      ordered.push_back(ic->first);
      if (ic->first->isTransparent()) ctrans = true;
    }
  }

  // if there were any transparent constrained objects, recount the first
  // transparent one
  if (ctrans) for (io = ordered.begin(); io != transparent; ++io)
    if ((*io)->isTransparent())
    {
      transparent = io;
      break;
    }

  return transparent;
}


QSlider* AWindow3D::getSliceSlider() const
{
  return d->slids;
}


AObject* AWindow3D::objectAtCursorPosition(int x, int y)
{
  if( x < 0 || y < 0 || x >= d->draw->qglWidget()->width()
    || y >= d->draw->qglWidget()->height() )
    return 0;
  // cerr << "objectAtCursorPosition " << x << ", " << y << endl;
  AObject *obj = 0;
  // render in ViewState::glSELECTRENDER_OBJECT mode (if needed)
  renderSelectionBuffer(ViewState::glSELECTRENDER_OBJECT);
  // read the color buffer at pos x,y
  d->draw->qglWidget()->makeCurrent();
  glFlush(); // or glFinish() ?
  GLubyte r, g, b;
  d->draw->readBackBuffer(x, d->draw->qglWidget()->height() - y, r, g, b);
  // convert color -> ID
  int id = (r << 16) | (g << 8) | b;
  /*  cout << "RGBA " << x << ", " << y << ": " << (unsigned) r << ", " << (unsigned) g << ", " << (unsigned) b << " : ID: " << id << endl;*/
  // get object with the same ID
  obj = objectWithGLID(id);
  // cout << "object: " << obj << endl;
  return obj;
}

list<AObject*> *AWindow3D::objectsAtCursorPosition(int /*x*/, int /*y*/, int /*tolerenceRadius*/)
{
  // TODO
  list<AObject *> *objs = 0;
  // render in ViewState::glSELECTRENDER_OBJECTS mode (if needed)
  renderSelectionBuffer(ViewState::glSELECTRENDER_OBJECTS);
  // get the selection records
  // get IDs
  // get objects with the same IDs
  return objs;
}

//ARN 09/10
int AWindow3D::polygonAtCursorPosition(int x, int y, const AObject* obj)
{
  unsigned poly = 0;
  if (x < 0 || y < 0 || x >= d->draw->qglWidget()->width() || y
      >= d->draw->qglWidget()->height()) return -1;

  // cout << "polygonAtCursorPosition\n";

  GLubyte r, g, b;

  if (surfpaintIsVisible())
  {
    //cout << "with optimization for surfpaint\n";

    GLubyte* tex = d->draw->getTextureFromBackBuffer();

    r = tex[3 * (d->draw->qglWidget()->height() - y)
        * d->draw->qglWidget()->width() + 3 * x];
    g = tex[3 * (d->draw->qglWidget()->height() - y)
        * d->draw->qglWidget()->width() + 3 * x + 1];
    b = tex[3 * (d->draw->qglWidget()->height() - y)
        * d->draw->qglWidget()->width() + 3 * x + 2];

    //  cout << "RGBA " << x << ", " << y << ": " << (unsigned) r << ", "
    //      << (unsigned) g << ", " << (unsigned) b << " : ID: " << poly << endl;
    //  cout << "ID polygon selected: " << poly << endl;
  }
  else
  {
    // cout << "without optimization\n";
    // render in ViewState::glSELECTRENDER_POLYGON mode (if needed)
    renderSelectionBuffer( ViewState::glSELECTRENDER_POLYGON, obj );
    // read the color buffer at pos x,y
    d->draw->qglWidget()->makeCurrent();
    d->draw->bindOtherFramebuffer( GLWidgetManager::PolygonSelect );
    glFlush(); // or glFinish() ?
    d->draw->readBackBuffer( x, d->draw->qglWidget()->height() - y, r, g, b );
  }

  // convert color -> ID
  poly = (r << 16) | (g << 8) | b;
  // polygon num is this ID

  return poly;
}

void AWindow3D::renderSelectionBuffer(ViewState::glSelectRenderMode mode,
    const AObject *selectedobject)
{
  /* cout << "renderSelectionBuffer... mode: " << mode << " for object: "
       << selectedobject;
  if( selectedobject )
    cout << ": " << selectedobject->name();
  cout << endl;
  */

  d->refreshneeded = Private::FullRefresh;
  d->draw->qglWidget()->makeCurrent();
  d->draw->bindOtherFramebuffer( GLWidgetManager::ObjectSelect );

  list<AObject *> renderobj;
  list<AObject *>::iterator transparent = processRenderingOrder(renderobj);
  list<AObject*>::iterator al, el = renderobj.end();

  GLPrimitives primitives;

  //	Rendering mode primitive (must be first)
  GLList *renderpr = new GLList;
  renderpr->generate();
  GLuint renderGLL = renderpr->item();
  if (!renderGLL) AWarning("AWindow3D::Refresh: OpenGL error.");

  glNewList(renderGLL, GL_COMPILE);

  glPushAttrib( GL_ALL_ATTRIB_BITS);
  glLineWidth(1);
  glShadeModel( GL_FLAT);
  glDisable( GL_LINE_SMOOTH);
  glDisable( GL_POLYGON_SMOOTH);
  glDisable( GL_LIGHTING);
  glPolygonOffset(0, 0);
  glDisable( GL_POLYGON_OFFSET_FILL);
  glDisable( GL_FOG);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  glDisable( GL_BLEND);
  // clipping planes
  GLdouble plane[4];
  Point3df dir = d->slicequat.transformInverse(Point3df(0, 0, -1));
  plane[0] = dir[0];
  plane[1] = dir[1];
  plane[2] = dir[2];
  plane[3] = -dir.dot(_position) + d->clipdist;
  switch (clipMode())
  {
    case Single:
      glEnable( GL_CLIP_PLANE0);
      glDisable( GL_CLIP_PLANE1);
      glClipPlane(GL_CLIP_PLANE0, plane);
      // glLightModeli( GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE );
      break;
    case Double:
      glEnable(GL_CLIP_PLANE0);
      glEnable(GL_CLIP_PLANE1);
      glClipPlane(GL_CLIP_PLANE0, plane);
      plane[0] *= -1;
      plane[1] *= -1;
      plane[2] *= -1;
      plane[3] = dir.dot(_position) + d->clipdist;
      glClipPlane(GL_CLIP_PLANE1, plane);
      // glLightModeli( GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE );
      break;
    default:
      glDisable(GL_CLIP_PLANE0);
      glDisable(GL_CLIP_PLANE1);
      // glLightModeli( GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE );
      break;
  }
  glEndList();
  primitives.push_back(RefGLItem(renderpr));

  //	Draw objects
  for( al = renderobj.begin(); al != el; ++al )
    if( mode != ViewState::glSELECTRENDER_POLYGON || *al == selectedobject )
      updateObject( *al, &primitives, mode );

  renderpr = new GLList;
  renderpr->generate();
  renderGLL = renderpr->item();
  if (!renderGLL) AWarning("AWindow3D::Refresh: OpenGL error.");

  glNewList(renderGLL, GL_COMPILE);
  glPopAttrib();
  glEndList();
  primitives.push_back(RefGLItem(renderpr));

  // d->draw->setPrimitives( d->primitives );
  d->draw->setSelectionPrimitives(primitives);

  // perform rendering, without swapBuffers

  d->draw->renderBackBuffer(mode);
}


bool AWindow3D::toopTipsEnabled() const
{
  return d->tooltip != 0;
}


void AWindow3D::enableToolTips( bool x )
{
  if( x )
  {
    if( !d->tooltip )
      d->tooltip = new QAViewToolTip( this, d->draw->qglWidget() );
  }
  else if( d->tooltip )
  {
    QAViewToolTip::hide();
    delete d->tooltip;
    d->tooltip = 0;
  }
}


bool AWindow3D::polygonsSortingEnabled() const
{
  return d->sortPolygons;
}


void AWindow3D::setPolygonsSortingEnabled( bool enabled )
{
  if( enabled != d->sortPolygons )
  {
    d->sortPolygons = enabled;
    if( enabled )
      Refresh();
    setChanged();
    notifyObservers( this );
  }
}


bool AWindow3D::polygonsSortingDirection() const
{
  return d->sortPolygonsDirection;
}


void AWindow3D::setPolygonsSortingDirection( bool dir )
{
  if( dir != d->sortPolygonsDirection )
  {
    d->sortPolygonsDirection = dir;
    Refresh();
    setChanged();
    notifyObservers( this );
  }
}


void AWindow3D::sortPolygons( bool force )
{
  GLWidgetManager *wm = dynamic_cast<GLWidgetManager *>( view() );
  if( !wm || ( !force && ( !wm->hasCameraChanged() || !d->sortPolygons ) ) )
    return;

  set<MObject *> mobjs;
  set<ASurface<3> *> meshes;
  set<AObject *>::iterator io, eo = _sobjects.end();
  MObject::iterator im, em;
  set<ASurface<3> *>::iterator is, es = meshes.end();
  float timestep = getTime(), timemesh;

  for( io=_sobjects.begin(); io!=eo; ++io )
  {
    ASurface<3> *mesh = dynamic_cast<ASurface<3> *>( *io );
    if( mesh && mesh->isTransparent() )
      meshes.insert( mesh );
    else
    {
      MObject *mobj = dynamic_cast<MObject *>( *io );
      if( mobj )
        mobjs.insert( mobj );
    }
  }

  while( !mobjs.empty() )
  {
    MObject *obj = *mobjs.begin();
    mobjs.erase( mobjs.begin() );
    for( im=obj->begin(), em=obj->end(); im!=em; ++im )
    {
      ASurface<3> *mesh = dynamic_cast<ASurface<3> *>( *im );
      if( mesh && mesh->isTransparent() )
        meshes.insert( mesh );
      else
      {
        MObject *mobj = dynamic_cast<MObject *>( *im );
        if( mobj )
          mobjs.insert( mobj );
      }
    }
  }

  anatomist::Transformation *t;
  Quaternion q = wm->quaternion();
  Point3df direction = q.transform( Point3df( 0, 0, 1 ) );
  if( wm->invertedZ() )
    direction[2] *= -1;
  Point3df transDir;

  for( is=meshes.begin(); is!=es; ++is )
  {
    timemesh = (*is)->actualTime( timestep );
    t = theAnatomist->getTransformation( getReferential(),
                                         (*is)->getReferential() );
    if( t )
      transDir = t->transform( direction );
    else
      transDir = direction;
    if( d->sortPolygonsDirection )
      transDir *= -1;
    SurfaceManip::sortPolygonsAlongDirection(
      *(*is)->surface(), int( timemesh ), transDir );
    (*is)->glSetChanged( GLComponent::glGEOMETRY );
    (*is)->notifyObservers( this );
  }
}


QImage AWindow3D::snapshotImage( int width, int height, int bufmode )
{
  GLWidgetManager *wm = dynamic_cast<GLWidgetManager *>( view() );
  if( wm && wm->hasCameraChanged() && !d->sortPolygons )
  {
    sortPolygons( true );
    Refresh();
  }

  triggeredRefresh();
  return d->draw->snapshotImage( bufmode, width, height );
}


void AWindow3D::saveSnapshot()
{
  GLWidgetManager *wm = dynamic_cast<GLWidgetManager *>( view() );
  if( wm && wm->hasCameraChanged() && !d->sortPolygons )
  {
    sortPolygons( true );
    Refresh();
  }

  triggeredRefresh();
  wm->saveContents();
}


void AWindow3D::saveSnapshotWithCustomSize()
{
  GLWidgetManager *wm = dynamic_cast<GLWidgetManager *>( view() );
  if( wm && wm->hasCameraChanged() && !d->sortPolygons )
  {
    sortPolygons( true );
    Refresh();
  }

  triggeredRefresh();
  wm->saveContentsWithCustomSize();
}


rc_ptr<ViewState> AWindow3D::viewState(bool slice)
{
  rc_ptr<ViewState> vs;
  if( d->viewtype == ThreeD && !slice )
  {
    vs.reset( new ViewState( _timepos, this,
                             ViewState::glSELECTRENDER_NONE ) );
  }
  else
  {
    vs.reset(
      new SliceViewState( _timepos, true, _position, &d->slicequat,
                          getReferential(),
                          windowGeometry(), &d->draw->quaternion(), this,
                          ViewState::glSELECTRENDER_NONE ) );
  }
  return vs;
}


void AWindow3D::updateSaveButtonStatus()
{
  if( !d->savebt )
    return;

  list<carto::shared_ptr<AObject> >::iterator io, eo = _objects.end();
  list<AObject *> tosave, toconfirm;
  bool useful = false;

  for( io=_objects.begin(); io!=eo; ++io )
    if( (*io)->savable() && (*io)->userModified() )
    {
      useful = true;
      break;
    }
  d->savebt->setEnabled( useful );
}


void AWindow3D::saveObject()
{
  // cout << "Save object\n";

  list<carto::shared_ptr<AObject> >::iterator io, eo = _objects.end();
  list<AObject *> tosave, toconfirm;

  for( io=_objects.begin(); io!=eo; ++io )
    if( (*io)->savable() && (*io)->userModified() )
    {
      if( !(*io)->allowsOverwriteOnSave() )
        toconfirm.push_back( io->get() );
      else
        tosave.push_back( io->get() );
    }

  if( toconfirm.size() != 0 )
  {
    set<AObject *> seto( toconfirm.begin(), toconfirm.end() );
    set<AObject *> sel( tosave.begin(), tosave.end() );
    seto.insert( sel.begin(), sel.end() );
    ObjectChooseDialog d( seto, sel, 0, this );
    d.setObjectsViewMode( ObjectParamSelect::Initial );
    d.setWindowTitle( d.tr( "Save and overwrite selected objects" ) );

    if( d.exec() == QDialog::Accepted )
    {
      set<AObject *> sel = d.selected();
      tosave.clear();
      tosave.insert( tosave.end(), sel.begin(), sel.end() );
      if( !tosave.empty() )
      {
        if( QMessageBox::warning(
            this, tr( "Confirm overwtite files" ),
            tr( "The selected objects will be saved in their current file "
                "names, overwriting the previous files." ),
                QMessageBox::Ok | QMessageBox::Cancel )
            != QMessageBox::Ok )
          return;
      }
    }
    else
      return;
  }

  cout << "saving: " << tosave.size() << endl;

  list<AObject *>::iterator io1, eo1 = tosave.end();

  for( io1=tosave.begin(); io1!=eo1; ++io1 )
    (*io1)->save( (*io1)->fileName(), true );
}

