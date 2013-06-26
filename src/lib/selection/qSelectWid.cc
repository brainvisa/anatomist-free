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


#include <anatomist/selection/qSelectWid.h>
#include <anatomist/selection/selectFactory.h>
#include <anatomist/object/Object.h>
#include <anatomist/application/Anatomist.h>
#include <qlayout.h>
#include <qframe.h>
#include <qpushbutton.h>


using namespace anatomist;
using namespace std;


QSelectWidget::QSelectWidget( unsigned gr, const set<AObject *> & objects, 
			      QWidget * parent, const char * name )
  : QDialog( parent), WSelectChooser( gr, objects )
{
  setObjectName(name);
  setModal(true);
  init( gr, objects );
}


QSelectWidget::QSelectWidget( unsigned gr, const set<AObject *> & objects )
  : QDialog( theAnatomist->getQWidgetAncestor() ), 
    WSelectChooser( gr, objects )
{
  setObjectName("QSelectWidget");
  setModal(true);
  init( gr, objects );
}


void QSelectWidget::init( unsigned gr, const set<AObject *> & objects )
{
  setWindowTitle( tr( "Select object(s)" ) );

  QVBoxLayout	*lay = new QVBoxLayout( this, 5, 5 );
  Q3ListView	*l = new Q3ListView( this, "listview" );

  _listW = l;
  setGroup( gr );
  lay->addWidget( l );
  l->addColumn( tr( "Selectable objects" ) );
  l->setMultiSelection( true );

  Q3ListViewItem				*li;
  set<AObject *>::const_iterator	io, fo=objects.end();
  SelectFactory				*fac = SelectFactory::factory();
  bool					sel; //, sh;
  set<AObject *>			tosel;
  SelectFactory::HColor			col( 1., 0., 0., 1., true );
  /*set<AWindow *>			win 
    = theAnatomist->getWindowsInGroup( group() );
    set<AWindow *>::const_iterator	iw, fw=win.end();*/

  for( io=objects.begin(); io!=fo; ++io )
    {
      li = new Q3ListViewItem( l, (*io)->name().c_str() );
      _objects[li] = *io;
      sel = fac->isSelected( group(), *io );
      if( sel )
	{
	  _select[ *io ] = 2;	// 1 for displayed but not selected
	  l->setSelected( li, true );
	  tosel.insert( *io );
	  //cout << "selected : " << (*io)->name() << endl;
	}
      else _select[ *io ] = 0;
    }
  fac->select( group(), tosel, &col );
  fac->refresh();

  l->setMinimumSize( 150, 100 );

  QFrame	*fr = new QFrame( this );
  QPushButton	*bok = new QPushButton( tr( "Select" ), fr );
  QPushButton	*bcc = new QPushButton( tr( "Cancel" ), fr );
  QHBoxLayout	*lay2 = new QHBoxLayout( fr, 5 );

  bok->setFixedSize( bok->sizeHint() );
  bok->setDefault( true );
  bcc->setFixedSize( bcc->sizeHint() );
  lay2->addStretch( 1 );
  lay2->addWidget( bok );
  lay2->addStretch( 1 );
  lay2->addWidget( bcc );
  lay2->addStretch( 1 );

  //cout << fr->height() << "  " << fr->sizeHint().height() << endl;
  fr->setFixedHeight( fr->sizeHint().height() );

  lay->addWidget( fr );

  connect( bok, SIGNAL( clicked() ), this, SLOT( accept() ) );
  connect( bcc, SIGNAL( clicked() ), this, SLOT( reject() ) );
  connect( l, SIGNAL( selectionChanged() ), this, SLOT( updateSelection() ) );
}


QSelectWidget::~QSelectWidget()
{
}


set<AObject *> QSelectWidget::selectedItems() const
{
  map<Q3ListViewItem *, AObject *>::const_iterator	io, fo=_objects.end();
  set<AObject *>	li;

  for( io=_objects.begin(); io!=fo; ++io )
    if( _listW->isSelected( (*io).first ) )
      li.insert( (*io).second );

  return( li );
}


void QSelectWidget::updateSelection()
{
  set<AObject *>	sel, unsel;
  SelectFactory		*fac = SelectFactory::factory();
  map<Q3ListViewItem *, AObject *>::const_iterator	io, fo=_objects.end();
  SelectFactory::HColor	col( 1., 0., 0., 1., true );

  for( io=_objects.begin(); io!=fo; ++io )
    if( _listW->isSelected( (*io).first ) )
      {
	if( !fac->isSelected( group(), (*io).second ) )
	  sel.insert( (*io).second );
      }
    else if( fac->isSelected( group(), (*io).second ) )
      unsel.insert( (*io).second );

  fac->unselect( group(), unsel );
  fac->select( group(), sel, &col );
  fac->refresh();
}


void QSelectWidget::restoreSelection()
{
  SelectFactory				*fac = SelectFactory::factory();
  map<AObject *, int>::const_iterator	is, fs=_select.end();
  set<AObject *>			sel, unsel;

  for( is=_select.begin(); is!=fs; ++is )
    if( (*is).second == 2 )
      {
	sel.insert( (*is).first );
      }
    else if( fac->isSelected( group(), (*is).first ) )
      unsel.insert( (*is).first );

  fac->unselect( group(), unsel );
  fac->select( group(), sel );
  //fac->refresh();
}


void QSelectWidget::accept()
{
  restoreSelection();
  QDialog::accept();
}


void QSelectWidget::reject()
{
  restoreSelection();
  QDialog::reject();
}
