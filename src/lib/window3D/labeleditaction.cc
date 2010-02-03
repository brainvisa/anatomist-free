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
#include <qtoolbar.h>
#include <aims/qtcompat/qtoolbutton.h>
#if QT_VERSION < 0x040000
#include <qtooltip.h>
#endif

using namespace anatomist;
using namespace carto;
using namespace std;


Action * 
LabelEditAction::creator()
{
  return new LabelEditAction;
}

LabelEditAction::LabelEditAction() : Action(), _color( 192, 192, 192 )
{
}


LabelEditAction::LabelEditAction( const LabelEditAction & a )
  : Action( a ), _label( a._label ), _color( a._color )
{
}


LabelEditAction::~LabelEditAction()
{
}

string LabelEditAction::name() const
{
  return( "LabelEditAction" );
}


QWidget* LabelEditAction::actionView()
{
  return( 0 );
}


bool LabelEditAction::viewableAction()
{
  return( false );
}


void LabelEditAction::pick()
{
  SelectFactory *fac = SelectFactory::factory();
  ControlledWindow *w = dynamic_cast<ControlledWindow *>( view()->window() );
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
  AWindow *w = view()->window();
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
  _label = x;

  // set label to toolbar
  ControlledWindow *w = dynamic_cast<ControlledWindow *>( view()->window() );
  if( !w )
    return;
  QLabel *ql = dynamic_cast<QLabel *>( w->child( "selectionLabel" ) );
  if( !ql )
    return; // no QLabel found

  // determine label color: find in hierarchies
  bool colfound = false;
  QColor  col( 192, 192, 192 );
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
          col = QColor( int( mat.Diffuse(0) * 255.9 ),
                        int( mat.Diffuse(1) * 255.9 ),
                        int( mat.Diffuse(2) * 255.9 ) );
          colfound = true;
        }
      }
    }
    else
      col = QColor( _color.red(), _color.green(), _color.blue() );
  }

  if( colfound )
    _color = AimsRGB( col.red(), col.green(), col.blue() );
  QPixmap pix( 20, 20 );
  pix.fill( col );
  ql->setPixmap( pix );
  QString text;
  text = ControlledWindow::tr( "Selected label: " );
  if( x.empty() )
    text += ControlledWindow::tr( "<none>" );
  else
    text += x.c_str();
  text += "\n\n";
  text += ControlledWindow::tr( "Label picker: <space> to pick the label "
      "from currently selected objects\n  <ctrl>+<return> to change the label "
      "of currently selected objects to this label" );
#if QT_VERSION >= 0x040000
  ql->setToolTip( text );
#else
  QToolTip::add( ql, text );
#endif
}


