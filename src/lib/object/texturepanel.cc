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

#include <anatomist/object/texturepanel.h>
#include <anatomist/object/Object.h>
#include <anatomist/surface/glcomponent.h>
#include <anatomist/commands/cTexturingParams.h>
#include <anatomist/processor/Processor.h>
#include <anatomist/object/qtextureparams.h>
#include <qlayout.h>
#include <aims/qtcompat/qvbox.h>
#include <aims/qtcompat/qvbuttongroup.h>
#include <aims/qtcompat/qbuttongroup.h>
#include <aims/qtcompat/qhgroupbox.h>
#include <qradiobutton.h>
#include <qslider.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qcombobox.h>

using namespace anatomist;
using namespace std;

struct QTexturePanel::Private
{
  Private( const set<AObject *> & obj );

  set<AObject *>		objects;
  Q3ButtonGroup			*modebox;
  Q3ButtonGroup			*filtbox;
  QHGroupBox			*ratebox;
  QGroupBox                     *genbox;
  QButtonGroup                  *geng;
  Q3ButtonGroup			*rgbintbox;
  QCheckBox			*rgbint;
  QSlider			*mixsl;
  QLabel			*mixlb;
  QPushButton			*genparambutton;
  int				mode;
  int				filt;
  int				genmode;
  int				rate;
  bool				rgbinterpol;
  bool				updating;
  unsigned			tex;
  vector<bool>			partvisible;
  bool				recurs;
  bool				uptodate;
  QComboBox                     *modes;
  vector<QRadioButton *>	filters;
  vector<QRadioButton *>	autotex;
  vector<float>			genparams[3];
  vector<QString>               texModesStrings;
};


QTexturePanel::Private::Private( const set<AObject *> & obj )
  : objects( obj ), mode( 0 ), filt( 0 ), rgbinterpol( false ), 
    updating( false ), tex( 0 ), 
    partvisible( 5 ), recurs( false ), uptodate( true ), modes( 0 )
{
  partvisible[0] = true;
  partvisible[1] = true;
  partvisible[2] = true;
  partvisible[3] = true;
  partvisible[4] = true;
  texModesStrings.reserve( GLComponent::glGEOMETRIC_SQRT + 1 );
  texModesStrings.push_back( QTexturePanel::tr( "Geometric" ) );
  texModesStrings.push_back( QTexturePanel::tr( "Linear" ) );
  texModesStrings.push_back( QTexturePanel::tr( "Replace" ) );
  texModesStrings.push_back( QTexturePanel::tr( "Decal" ) );
  texModesStrings.push_back( QTexturePanel::tr( "Blend" ) );
  texModesStrings.push_back( QTexturePanel::tr( "Add" ) );
  texModesStrings.push_back( QTexturePanel::tr( "Combine" ) );
  texModesStrings.push_back( QTexturePanel::tr( "Linear / A if A is white" ) );
  texModesStrings.push_back( QTexturePanel::tr( "Linear / A if B is white" ) );
  texModesStrings.push_back( QTexturePanel::tr( "Linear / A if A is black" ) );
  texModesStrings.push_back( QTexturePanel::tr( "Linear / A if B is black" ) );
  texModesStrings.push_back( QTexturePanel::tr( "Linear / B if A is white" ) );
  texModesStrings.push_back( QTexturePanel::tr( "Linear / B if B is white" ) );
  texModesStrings.push_back( QTexturePanel::tr( "Linear / B if A is black" ) );
  texModesStrings.push_back( QTexturePanel::tr( "Linear / B if B is black" ) );
  texModesStrings.push_back( QTexturePanel::tr( "Linear / A if A is opaque"
    ) );
  texModesStrings.push_back( QTexturePanel::tr( "Linear / A if B is not opaque"
    ) );
  texModesStrings.push_back( QTexturePanel::tr( "Linear / B if B is opaque"
    ) );
  texModesStrings.push_back( QTexturePanel::tr( "Linear / B if A is not opaque"
    ) );
  texModesStrings.push_back( QTexturePanel::tr( "Max channel" ) );
  texModesStrings.push_back( QTexturePanel::tr( "Min channel" ) );
  texModesStrings.push_back( QTexturePanel::tr( "Max opacity" ) );
  texModesStrings.push_back( QTexturePanel::tr( "Min opacity" ) );
  texModesStrings.push_back( QTexturePanel::tr( "Geometric (square root)" ) );
}


QTexturePanel::QTexturePanel( const set<AObject *> & obj, 
                              QWidget* parent, const char *name )
  : QWidget( parent ), Observer(), d( new Private( obj ) )
{
  setCaption( name );
  setObjectName( name );

  QHBoxLayout	*mainlay = new QHBoxLayout( this );
  QVBox		*vbox = new QVBox( this );
  // mainlay->setMargin( 10 );
  mainlay->setSpacing( 10 );
  vbox->setSpacing( 10 );
  mainlay->addWidget( vbox );

  d->modebox = new QVButtonGroup( tr( "Mapping mode" ), vbox );
  d->modes = new QComboBox( d->modebox );

  d->ratebox = new QHGroupBox( tr( "Mixing rate" ), vbox );
  d->mixsl = new QSlider( Qt::Horizontal, d->ratebox );
  d->mixsl->setMinValue( 0 );
  d->mixsl->setMaxValue( 100 );
  d->mixsl->setLineStep( 1 );
  d->mixsl->setPageStep( 1 );
  d->mixlb = new QLabel( "100", d->ratebox );
  d->mixlb->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, 
                                        QSizePolicy::Fixed ) );

  vbox = new QVBox( this );
  mainlay->addWidget( vbox );
  vbox->setSpacing( 10 );

  d->genbox = new QGroupBox( tr( "Texture generation" ), vbox );
  QButtonGroup * geng = new QButtonGroup( d->genbox );
  d->geng = geng;
  QVBoxLayout *vlay = new QVBoxLayout( d->genbox );
  d->genbox->setLayout( vlay );
  QRadioButton *btn = new QRadioButton( tr( "None" ), d->genbox );
  vlay->addWidget( btn );
  geng->addButton( btn );
  geng->setId( btn, 0 );
  btn = new QRadioButton( tr( "Linear - object" ), d->genbox );
  vlay->addWidget( btn );
  geng->addButton( btn );
  geng->setId( btn, 1 );
  btn = new QRadioButton( tr( "Linear - eye" ), d->genbox );
  vlay->addWidget( btn );
  geng->addButton( btn );
  geng->setId( btn, 2 );
  btn = new QRadioButton( tr( "Sphere reflection" ), d->genbox );
  vlay->addWidget( btn );
  geng->addButton( btn );
  geng->setId( btn, 3 );
  btn = new QRadioButton( tr( "Reflection" ), d->genbox );
  vlay->addWidget( btn );
  geng->addButton( btn );
  geng->setId( btn, 4 );
  btn = new QRadioButton( tr( "Normal" ), d->genbox );
  vlay->addWidget( btn );
  geng->addButton( btn );
  geng->setId( btn, 5 );
  QPushButton	*gpb = new QPushButton( tr( "Parameters..." ), d->genbox, 
                                        "genparams_button" );
  vlay->addWidget( gpb );
  geng->setExclusive( true );
  d->genparambutton = gpb;
  gpb->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );

  d->filtbox = new QVButtonGroup( tr( "Texture filtering" ), vbox );
  new QRadioButton( tr( "None" ), d->filtbox );
  new QRadioButton( tr( "Linear filter" ), d->filtbox );

  d->rgbintbox = new QVButtonGroup( tr( "Texture interpolation" ), vbox );
  d->rgbint = new QCheckBox( tr( "RGB space interpolation (label textures)" ), 
                             d->rgbintbox );

  updateWindow();

  connect( d->modes, SIGNAL( activated( int ) ), SLOT( modeChanged( int ) ) );
  connect( d->filtbox, SIGNAL( clicked( int ) ), 
           SLOT( filteringChanged( int ) ) );
  connect( d->geng, SIGNAL( buttonClicked( int ) ),
           SLOT( generationChanged( int ) ) );
  connect( d->mixsl, SIGNAL( valueChanged( int ) ), 
           SLOT( rateChanged( int ) ) );
  connect( d->rgbint, SIGNAL( toggled( bool ) ), this, 
           SLOT( rgbInterpolation( bool ) ) );
  connect( gpb, SIGNAL( clicked() ), this, SLOT( generationParamsDialog() ) );

  set<AObject *>::const_iterator	io, eo = obj.end();
  for( io=obj.begin(); io!=eo; ++io )
    (*io)->addObserver( this );
}


QTexturePanel::~QTexturePanel()
{
  //cout << "~QTexturePanel\n";

  set<AObject *>			obj = d->objects;
  d->objects.clear();
  cleanupObserver();

  if( !d->uptodate )
    {
      TexturingParamsCommand 
        *c = new TexturingParamsCommand( obj, d->tex, d->mode, d->filt, 
                                         d->genmode, 0.01 * d->rate );
      theProcessor->execute( c );
    }

  delete d;
  //cout << "~QTexturePanel done\n";
}


void QTexturePanel::runCommand()
{
  // cout << "QTexturePanel::runCommand: uptodate: " << d->uptodate << endl;
  if( d->uptodate )
    return;
  if( !d->objects.empty() )
    {
      float	*gp1 = 0, *gp2 = 0, *gp3 = 0;
      if( d->genparams[0].size() == 4 )
        gp1 = &d->genparams[0][0];
      if( d->genparams[1].size() == 4 )
        gp2 = &d->genparams[1][0];
      if( d->genparams[2].size() == 4 )
        gp3 = &d->genparams[2][0];
      TexturingParamsCommand 
        *c = new TexturingParamsCommand( d->objects, d->tex, d->mode, d->filt, 
                                         d->genmode, 0.01 * d->rate, 
                                         d->rgbinterpol, gp1, gp2, gp3 );
      theProcessor->execute( c );
    }
  d->uptodate = true;
}


const set<AObject *> & QTexturePanel::objects() const
{
  return d->objects;
}


void QTexturePanel::update( const Observable* /*observable*/, 
                            void* /*arg*/ )
{
  if( d->updating )
    return;
  d->updating = true;
  updateWindow();
  d->updating = false;
}


void QTexturePanel::unregisterObservable( anatomist::Observable* obs )
{
  Observer::unregisterObservable( obs );
  AObject	*o = dynamic_cast<AObject *>( obs );
  if( !o )
    return;
  set<AObject *>::iterator	i = d->objects.find( o );
  if( i != d->objects.end() )
    {
      runCommand();
      d->objects.erase( o );
      updateWindow();
    }
}


void QTexturePanel::updateAutoTexParams()
{
  // cout << "updateAutoTexParams\n";
  AObject	*ao = *d->objects.begin();
  vector<bool>	mv( 5 );

  GLComponent	*c = ao->glAPI();
  if( c )
    {
      GLComponent::glAutoTexturingMode	tm = c->glAutoTexMode( d->tex );
      c->glSetAutoTexMode( (GLComponent::glAutoTexturingMode) d->genmode, 
                           d->tex );
      const float	*par = c->glAutoTexParams( 0, d->tex );
      if( par )
        {
          d->genparams[0].clear();
          d->genparams[0].reserve( 4 );
          d->genparams[0].push_back( par[0] );
          d->genparams[0].push_back( par[1] );
          d->genparams[0].push_back( par[2] );
          d->genparams[0].push_back( par[3] );
        }
      par = c->glAutoTexParams( 1, d->tex );
      if( par )
        {
          d->genparams[1].clear();
          d->genparams[1].reserve( 4 );
          d->genparams[1].push_back( par[0] );
          d->genparams[1].push_back( par[1] );
          d->genparams[1].push_back( par[2] );
          d->genparams[1].push_back( par[3] );
        }
      par = c->glAutoTexParams( 2, d->tex );
      if( par )
        {
          d->genparams[2].clear();
          d->genparams[2].reserve( 4 );
          d->genparams[2].push_back( par[0] );
          d->genparams[2].push_back( par[1] );
          d->genparams[2].push_back( par[2] );
          d->genparams[2].push_back( par[3] );
        }
      c->glSetAutoTexMode( tm, d->tex );
    }
}


void QTexturePanel::updateWindow()
{
  if( d->recurs )
    return;
  d->recurs = true;

  if( d->objects.empty() )
    return;

  d->mixsl->blockSignals( true );

  AObject	*ao = *d->objects.begin();
  // cout << "obj name: " << ao->name() << ", tex: " << d->tex << endl;
  vector<bool>	mv( 5 );

  GLComponent	*c = ao->glAPI();
  if( c )
    {
      d->mode = c->glTexMode( d->tex );
      d->filt = c->glTexFiltering( d->tex );
      d->rgbinterpol = c->glTexRGBInterpolation( d->tex );
      d->genmode = c->glAutoTexMode( d->tex );
      d->rate = (int) rint( c->glTexRate( d->tex ) * 100 );
      d->filtbox->setButton( d->filt );
      d->rgbint->setChecked( d->rgbinterpol );
      d->geng->button( d->genmode )->setChecked( true );
      d->mixsl->setValue( d->rate );
      d->mixlb->setText( QString::number( d->rate ) );

      updateAutoTexParams();

      set<GLComponent::glTextureMode>	modes = c->glAllowedTexModes( d->tex );
      set<GLComponent::glTextureMode>::iterator	is, es = modes.end();
      int i = 0, nm, mode = 0;
      while( d->modes->count() != 0 )
        d->modes->removeItem( 0 );
      for( is=modes.begin(); is!=es; ++is, ++i )
      {
        d->modes->addItem( d->texModesStrings[ *is ] );
        if( *is == d->mode )
          mode = i;
      }
      d->modes->setCurrentItem( mode );

      if( modes.size() > 1 )
        mv[ Mode ] = true;

      set<GLComponent::glTextureFiltering> 
        filts = c->glAllowedTexFilterings( d->tex );
      set<GLComponent::glTextureFiltering>::iterator	itf, etf = filts.end();
      nm = d->filters.size();
      i = 0;
      for( itf=filts.begin(); itf!=etf; ++itf )
        {
          for( ; i<*itf && i<nm; ++i )
            d->filters[i]->hide();
          if( *itf < nm )
            d->filters[*itf]->show();
          ++i;
        }
      for( ; i<nm; ++i )
        d->filters[i]->hide();
      if( filts.size() > 1 )
        mv[ Filtering ] = true;

      if( c->glAllowedTexRate( d->tex ) )
        mv[ Rate ] = true;

      set<GLComponent::glAutoTexturingMode> 
        gens = c->glAllowedAutoTexModes( d->tex );
      set<GLComponent::glAutoTexturingMode>::iterator	itg, etg = gens.end();
      nm = d->autotex.size();
      i = 0;
      for( itg=gens.begin(); itg!=etg; ++itg )
        {
          for( ; i<*itg && i<nm; ++i )
            d->autotex[i]->hide();
          if( *itg < nm )
            d->autotex[*itg]->show();
          ++i;
        }
      for( ; i<nm; ++i )
        d->autotex[i]->hide();
      if( gens.size() > 1 )
        mv[ Generation ] = true;

      if( c->glAllowedTexRGBInterpolation( d->tex ) )
        mv[ Interpolation ] = true;

      if( c->glAutoTexMode( d->tex ) == GLComponent::glTEX_OBJECT_LINEAR 
          || c->glAutoTexMode( d->tex ) == GLComponent::glTEX_EYE_LINEAR )
        d->genparambutton->setEnabled( true );
      else
        d->genparambutton->setEnabled( false );
     }
  else
    d->genparambutton->setEnabled( false );

  if( d->partvisible[ Mode ] && mv[ Mode ] )
    d->modebox->show();
  else
    d->modebox->hide();
  if( d->partvisible[ Rate ] && mv[ Rate ] )
    d->ratebox->show();
  else
    d->ratebox->hide();
  if( d->partvisible[ Filtering ] && mv[ Filtering ] )
    d->filtbox->show();
  else
    d->filtbox->hide();
  if( d->partvisible[ Generation ] && mv[ Generation ] )
    d->genbox->show();
  else
    d->genbox->hide();
  if( d->partvisible[ Interpolation ] && mv[ Interpolation ] )
    d->rgbintbox->show();
  else
    d->rgbintbox->hide();

  d->mixsl->blockSignals( false );

  d->recurs = false;
}


void QTexturePanel::modeChanged( int x )
{
  //cout << "QTexturePanel::modeChanged " << x << " / " << d->mode << endl;
  if( d->mode != x )
  {
    d->mode = x;
    d->uptodate = false;
    QString m = d->modes->currentText();
    int i, n = d->texModesStrings.size(), mode = 0;
    for( i=0; i<n; ++i )
    {
      if( d->texModesStrings[i] == m )
      {
        mode = i;
        break;
      }
    }
    d->mode = mode;
    runCommand();
  }
}


void QTexturePanel::generationChanged( int x )
{
  if( x >= 6 )
    return;
  // cout << "QTexturePanel::generationChanged: " << x << endl;
  if( d->genmode != x )
    {
      d->genmode = x;
      d->uptodate = false;

      // texgen params are not up-to-date anymore
      updateAutoTexParams();

      runCommand();
      // updateObjects();
    }
}


void QTexturePanel::rateChanged( int x )
{
  if( d->rate != x )
    {
      d->rate = x;
      d->mixlb->setText( QString::number( x ) );
      d->uptodate = false;
      updateObjects();
    }
}


void QTexturePanel::filteringChanged( int x )
{
  if( d->filt != x )
    {
      d->filt = x;
      d->uptodate = false;
      runCommand();
      // updateObjects();
    }
}


void QTexturePanel::updateObjects()
{
  // cout << "QTexturePanel::updateObjects()\n";
  set<AObject *>::const_iterator	i, e = d->objects.end();

  for( i=d->objects.begin(); i!=e; ++i )
    {
      // cout << *i << endl;
      GLComponent	*c = (*i)->glAPI();
      if( c )
        {
          // cout << "GLcomponent: " << c << endl;
          c->glSetTexMode( (GLComponent::glTextureMode) d->mode, d->tex );
          c->glSetTexFiltering( (GLComponent::glTextureFiltering) d->filt, 
                                d->tex );
          c->glSetTexRGBInterpolation( d->rgbinterpol, d->tex );
          c->glSetAutoTexMode( (GLComponent::glAutoTexturingMode) d->genmode, 
                               d->tex );
          c->glSetTexRate( float(d->rate) / 100, d->tex );

          if( d->genparams[0].size() == 4 )
            c->glSetAutoTexParams( &d->genparams[0][0], 0, d->tex );
          if( d->genparams[1].size() == 4 )
            c->glSetAutoTexParams( &d->genparams[1][0], 1, d->tex );
          if( d->genparams[2].size() == 4 )
            c->glSetAutoTexParams( &d->genparams[2][0], 2, d->tex );

          // cout << "notify observers for " << *i << "/" << c << endl;
          (*i)->notifyObservers( this );
        }
    }
}


unsigned QTexturePanel::activeTexture() const
{
  return d->tex;
}


void QTexturePanel::setActiveTexture( unsigned tex )
{
  runCommand();
  d->tex = tex;
  if( !d->objects.empty() )
    {
      GLComponent	*c = (*d->objects.begin())->glAPI();
      if( c )
        {
          d->mode = c->glTexMode( tex );
          d->filt = c->glTexFiltering( tex );
          d->genmode = c->glAutoTexMode( tex );
          d->rate = int( c->glTexRate( tex ) * 100 );
        }
    }
  updateWindow();
}


bool QTexturePanel::isVisible( Part p ) const
{
  return d->partvisible[ p ];
}


void QTexturePanel::setVisibility( Part p, bool x )
{
  d->partvisible[ p ] = x;
  updateWindow();
}


void QTexturePanel::setObjects( const set<AObject *> & obj )
{
  bool	r = d->recurs;
  d->recurs = true;
  runCommand();

  set<AObject *>::const_iterator	io, fo = d->objects.end();
  for( io=d->objects.begin(); io!=fo; ++io )
    (*io)->deleteObserver( this );

  d->objects = obj;

  for( io=d->objects.begin(); io!=fo; ++io )
    (*io)->addObserver( this );

  d->recurs = r;
  setActiveTexture( 0 );
}


void QTexturePanel::rgbInterpolation( bool x )
{
  if( d->rgbinterpol != x )
    {
      d->rgbinterpol = x;
      d->uptodate = false;
      runCommand();
    }
}


void QTexturePanel::generationParamsDialog()
{
  QTextureParams	tp( this, "generationParams", true );

  tp.setParams( 0, d->genparams[0] );
  tp.setParams( 1, d->genparams[1] );
  tp.setParams( 2, d->genparams[2] );

  bool	res = tp.exec();

  if( res )
    {
      d->uptodate = false;
      d->genparams[0] = tp.params( 0 );
      d->genparams[1] = tp.params( 1 );
      d->genparams[2] = tp.params( 2 );
      runCommand();
    }
}


/*
void QTexturePanel::setAllowedTextureModes( const vector<int> & at )
{
  d->allowedTexModes = at;
  // TODO: update interface
}
*/

