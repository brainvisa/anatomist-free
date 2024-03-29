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


#include <anatomist/window3D/labeleditaction.h>
#include <anatomist/selection/selectFactory.h>
#include <anatomist/window/controlledWindow.h>
#include <anatomist/controler/view.h>
#include <anatomist/control/graphParams.h>
#include <anatomist/object/Object.h>
#include <anatomist/graph/attribAObject.h>
#include <anatomist/graph/GraphObject.h>
#include <anatomist/graph/Graph.h>
#include <anatomist/hierarchy/hierarchy.h>
#include <graph/tree/tree.h>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsProxyWidget>

using namespace anatomist;
using namespace carto;
using namespace std;


struct LabelEditAction::Private
{
  Private() {}
};


Action * 
LabelEditAction::creator()
{
  return new LabelEditAction;
}

LabelEditAction::LabelEditAction()
  : Action(), _color( 192, 192, 192 ), d( new Private )
{
}


LabelEditAction::LabelEditAction( const LabelEditAction & a )
  : Action( a ), _label( a._label ), _color( a._color ), d( new Private )
{
}


LabelEditAction::~LabelEditAction()
{
  delete d;
}

string LabelEditAction::name() const
{
  return( "LabelEditAction" );
}


QWidget* LabelEditAction::actionView( QWidget* )
{
  return( 0 );
}


bool LabelEditAction::viewableAction() const
{
  return( false );
}


void LabelEditAction::pick()
{
  SelectFactory *fac = SelectFactory::factory();
  ControlledWindow *w = dynamic_cast<ControlledWindow *>( view()->aWindow() );
  if( !w )
    return;
  const map<unsigned, set< AObject *> > & sel = fac->selected();
  map<unsigned, set< AObject *> >::const_iterator i = sel.find( w->Group() );
  if( i == sel.end() )
    return;
  const set<AObject *> & obj = i->second;
  set<AObject *>::const_iterator io, eo = obj.end();
  string label, label2, att, attbis = GraphParams::graphParams()->attribute;
  AObject *gobj = 0;
  for( io=obj.begin(); io!=eo; ++io )
  {
    AttributedAObject *ato = dynamic_cast<AttributedAObject *>( *io );
    if( ato )
    {
      AObject::ParentList::const_iterator ip, ep = (*io)->parents().end();
      att = attbis;
      for( ip=(*io)->parents().begin(); ip!=ep; ++ip )
        if( (*ip)->type() == AObject::GRAPH )
        {
          att = static_cast<const AGraph *>( *ip )->labelProperty();
          break;
        }
      if( ato->attributed()->getProperty( att, label2 ) && !label2.empty() )
      {
        if( label.empty() )
          label = label2;
        else if( label != label2 )
          return; // inconsistency
        gobj = *io;
      }
    }
  }
  cout << "label: " << label << endl;
  if( label.empty() )
    return;
  setLabel( label, gobj );
}


void LabelEditAction::edit()
{
  if( _label.empty() )
    return;
  SelectFactory *fac = SelectFactory::factory();
  AWindow *w = view()->aWindow();
  if( !w )
    return;
  const map<unsigned, set< AObject *> > & sel = fac->selected();
  map<unsigned, set< AObject *> >::const_iterator i = sel.find( w->Group() );
  if( i == sel.end() )
    return;
  const set<AObject *> & obj = i->second;
  set<AObject *> aobj;
  set<AObject *>::const_iterator io, eo = obj.end();
  string att, attbis = GraphParams::graphParams()->attribute;
  for( io=obj.begin(); io!=eo; ++io )
  {
    AttributedAObject *ato = dynamic_cast<AttributedAObject *>( *io );
    if( ato )
    {
      AObject::ParentList::const_iterator ip, ep = (*io)->parents().end();
      att = attbis;
      for( ip=(*io)->parents().begin(); ip!=ep; ++ip )
        if( (*ip)->type() == AObject::GRAPH )
        {
          att = static_cast<const AGraph *>( *ip )->labelProperty();
          break;
        }
      aobj.insert( *io );
      ato->attributed()->setProperty( att, _label );
      (*io)->setChanged();
      (*io)->internalUpdate();
      if( ip != ep )
        (*ip)->setUserModified( true );
    }
  }
  for( io=aobj.begin(), eo=aobj.end(); io!=eo; ++io )
    (*io)->notifyObservers( this );
  // is this a good idea or not...?
  fac->unselectAll( w->Group() );
}


string LabelEditAction::label() const
{
  return _label;
}


void LabelEditAction::setLabel( const string & x, const AObject *obj )
{
  // determine label color: find in hierarchies
  AimsRGB  col( 192, 192, 192 );
  if( !x.empty() )
  {
    if( obj )
    {
      Hierarchy *hie = Hierarchy::findMatchingNomenclature( obj );
      if( hie )
      {
        Material  mat;
        if( GraphParams::nomenclatureColorForLabel( x, hie, mat ) )
        {
          col = AimsRGB( int( mat.Diffuse(0) * 255.9 ),
                         int( mat.Diffuse(1) * 255.9 ),
                         int( mat.Diffuse(2) * 255.9 ) );
        }
      }
    }
    else
      col = _color;
  }

  // propagate to all windows in the same group
  set<AWindow *> wins = theAnatomist->getWindowsInGroup(
      view()->aWindow()->Group() );
  set<AWindow *>::iterator iw, ew = wins.end();
  for( iw=wins.begin(); iw!=ew; ++iw )
  {
    ControlledWindow *w = dynamic_cast<ControlledWindow *>( *iw );
    if( !w )
      continue;
    Action *ac = w->view()->controlSwitch()->getAction( name() );
    if( ac )
    {
      LabelEditAction * lac = dynamic_cast<LabelEditAction *>( ac );
      if( lac )
        lac->setLabel( x, col );
    }
  }
}

void LabelEditAction::setLabel( const std::string & l, const AimsRGB & color )
{
  _label = l;
  _color = color;

  ControlledWindow *w = dynamic_cast<ControlledWindow *>( view()->aWindow() );
  if( !w )
    return;
  QLabel *ql = w->findChild<QLabel *>( "selectionLabel" );
  if( !ql )
  {
    // label not found - maybe in a QGraphcsView ?
    QGraphicsView *gv = w->findChild<QGraphicsView *>();
    if( !gv || !gv->scene() )
      return; // no QLabel found

    QList<QGraphicsItem *> items = gv->scene()->items();
    foreach( QGraphicsItem *item, items )
    {
      QGraphicsProxyWidget *pw
        = qgraphicsitem_cast<QGraphicsProxyWidget *>( item );
      if( pw )
      {
        if( pw->widget()->objectName() == "selectionLabel" )
        {
          ql = dynamic_cast<QLabel *>( pw->widget() );
          if( ql )
            break;
        }
      }
    }
    if( !ql )
      return;
  }

  QColor col( color.red(), color.green(), color.blue() );
  QPixmap pix( 20, 20 );
  pix.fill( col );
  ql->setPixmap( pix );
  QString text;
  text = ControlledWindow::tr( "Selected label: " );
  if( l.empty() )
    text += ControlledWindow::tr( "<none>" );
  else
    text += l.c_str();
  text += "\n\n";
  text += ControlledWindow::tr( "Label picker: <space> to pick the label "
      "from currently selected objects\n  <ctrl>+<return> to change the label "
      "of currently selected objects to this label" );
  ql->setToolTip( text );
}


