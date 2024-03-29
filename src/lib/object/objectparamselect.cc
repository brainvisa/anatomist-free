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

#include <anatomist/object/objectparamselect.h>
#include <anatomist/object/Object.h>
#include <anatomist/application/Anatomist.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qdialog.h>
#include <qlistwidget.h>
#include <qlayout.h>
#include <qcombobox.h>
#include <qvalidator.h>
#include <qlistwidget.h>

using namespace anatomist;
using namespace std;

// ----------

struct ObjectParamSelect::Private
{
  Private( const set<AObject *> & o );

  list<ObjectParamSelect::Filter>	filters;
  set<AObject *>			objects;
  ObjectParamSelect::ViewMode		viewmode;
  QLineEdit				*label;
};


ObjectParamSelect::Private::Private( const set<AObject *> & o )
  : objects( o ), viewmode( ObjectParamSelect::TopLevel )
{
}


namespace
{

  QString labelname( const set<AObject *> & obj )
  {
    if( obj.empty() )
      return ObjectParamSelect::tr( "<no object>" );

    set<AObject *>::const_iterator	io, eo = obj.end();
    QString	name;
    for( io=obj.begin(); io!=eo && name.length() < 200; ++io )
      {
        if( !name.isEmpty() )
          name += ", ";
        name += (*io)->name().c_str();
      }
    if( io != eo )
      name += "...";
    return name;
  }

}


ObjectParamSelect::ObjectParamSelect( const set<AObject *> & o, 
                                      QWidget* parent )
  : QWidget( parent ), d( new Private( o ) )
{
	drawContents();
}


ObjectParamSelect::~ObjectParamSelect()
{
  delete d;
}


void ObjectParamSelect::updateLabel( const set<AObject *> & obj )
{
  d->label->setText( labelname( obj ) );
}


void ObjectParamSelect::drawContents()
{
  QHBoxLayout *layout = new QHBoxLayout( this );
  layout->setSpacing( 5 );
  
  //setContentsMargins( 5, 5, 5, 5 );
  QLineEdit	*l = new QLineEdit( labelname( d->objects ), this );
  d->label = l;
  l->setReadOnly( true );
  QPushButton	*chseobj = new QPushButton( "...", this );
  chseobj->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, 
                                       QSizePolicy::Fixed ) );
  setSizePolicy( QSizePolicy( QSizePolicy::Preferred, 
                              QSizePolicy::Fixed ) );
  layout->addWidget( l );
  layout->addWidget( chseobj );
  connect( chseobj, SIGNAL( clicked() ), this, SIGNAL( selectionStarts() ) );
}


void ObjectParamSelect::selectObjects()
{
  selectObjects( d->objects, d->objects );
}


void ObjectParamSelect::selectObjects( const set<AObject *> & obj, 
                                       const set<AObject *> & sel )
{
  //cout << "selectObjects init: " << obj.size() << ", sel: " << sel.size() 
  //     << endl;
  ObjectChooseDialog	dial( obj, sel, this, 
                              theAnatomist->getQWidgetAncestor() );
  dial.setObjectsViewMode( d->viewmode );

  int	res = dial.exec();
  d->viewmode = (ViewMode) dial.objectsViewMode();

  if( res )
    {
      set<AObject *>			obj2;
      set<AObject *>::const_iterator	io, eo = sel.end(), eo2 = obj.end(), 
        eo3;
      const QListWidget			*lb = dial.list();
      int				i, n = lb->count();

      for( io=sel.begin(), i=0; i<n && io!=eo; ++io )
        if( filter( *io ) )
          {
            if( lb->item( i )->isSelected() )
              obj2.insert( *io );
            ++i;
          }
      for( io=obj.begin(); i<n && io!=eo2; ++io )
        if( filter( *io ) && sel.find( *io ) == eo )
          {
            if( lb->item( i )->isSelected() )
              obj2.insert( *io );
            ++i;
          }
      switch( d->viewmode )
        {
        case All:
          {
            set<AObject *>	o = theAnatomist->getObjects();
            for( io=o.begin(), eo3=o.end(); i<n && io!=eo3; ++io )
              if( filter( *io ) && sel.find( *io ) == eo 
                  && obj.find( *io ) == eo2 )
                {
                  if( lb->item( i )->isSelected() )
                    obj2.insert( *io );
                  ++i;
                }
          }
          break;
        case TopLevel:
          {
            set<AObject *>	o = theAnatomist->getObjects();
            for( io=o.begin(), eo3=o.end(); i<n && io!=eo3; ++io )
              if( filter( *io ) && sel.find( *io ) == eo 
                  && obj.find( *io ) == eo2 
                  && ( (*io)->Visible() || (*io)->Parents().empty() ) )
                {
                  if( lb->item( i )->isSelected() )
                    obj2.insert( *io );
                  ++i;
                }
          }
          break;
        default:
          break;
        }

      // cout << "selected objects: " << obj2.size() << endl;
      d->objects = obj2;
      d->label->setText( labelname( obj2 ) );

      emit objectsSelected( obj2 );
    }
}


bool ObjectParamSelect::filter( const AObject* o ) const
{
  list<Filter>::const_iterator	i, e = d->filters.end();
  for( i=d->filters.begin(); i!=e && (*i)( o ); ++i ) {}
  return ( i == e );
}


void ObjectParamSelect::addFilter( Filter f )
{
  d->filters.push_back( f );
}


void ObjectParamSelect::setObjectsViewMode( int x )
{
  d->viewmode = (ViewMode) x;
}


const set<AObject *> & ObjectParamSelect::selectedObjects() const
{
  return d->objects;
}

// ------------

struct ObjectChooseDialog::Private
{
  Private( const set<AObject *> & o, const set<AObject *> & sl, 
           ObjectParamSelect* s );

  QListWidget		*list;
  QComboBox		*viewbox;
  const set<AObject *>	& obj;
  const set<AObject *>	& sel;
  ObjectParamSelect	*psel;
  int			viewmode;
};


ObjectChooseDialog::Private::Private( const set<AObject *> & o, 
                                      const set<AObject *> & sl, 
                                      ObjectParamSelect* s )
  : list( 0 ), viewbox( 0 ), obj( o ), sel( sl ), psel( s ), 
    viewmode( ObjectParamSelect::TopLevel )
{
}


ObjectChooseDialog::ObjectChooseDialog( const set<AObject *> & obj, 
                                        const set<AObject *> & sel, 
                                        ObjectParamSelect* s, QWidget* parent )
  : QDialog( parent ), d( new Private( obj, sel, s ) )
{
  setWindowTitle( ObjectParamSelect::tr( "Set parameters on these objects:" ) );
  setObjectName("selectobject");
  setModal(true);
  QVBoxLayout	*l = new QVBoxLayout( this );
  l->setContentsMargins( 5, 5, 5, 5 );
  l->setSpacing( 5 );
  QListWidget	*lb = new QListWidget( this );
  d->list = lb;
  l->addWidget( lb );
  lb->setSelectionMode( QListWidget::ExtendedSelection );

  QComboBox	*all 
    = new QComboBox( this );
  all->addItem( ObjectParamSelect::tr( "Show initial objects" ) );
  all->addItem( ObjectParamSelect::tr( "Show all objects" ) );
  all->addItem( ObjectParamSelect::tr( "Show top-level objects" ) );
  l->addWidget( all );
  d->viewbox = all;

  QWidget *hb = new QWidget( this );
  QHBoxLayout *hbLayout = new QHBoxLayout( hb );
  l->addWidget( hb );
  hbLayout->setSpacing( 5 );
  QPushButton	*pb = new QPushButton( ObjectParamSelect::tr( "OK" ), hb );
  hbLayout->addWidget( pb );
  pb->setDefault( true );
  connect( pb, SIGNAL( clicked() ), this, SLOT( accept() ) );
  pb = new QPushButton( ObjectParamSelect::tr( "Cancel" ), hb );
  hbLayout->addWidget( pb );
  connect( pb, SIGNAL( clicked() ), this, SLOT( reject() ) );
  connect( all, SIGNAL( activated( int ) ), this, 
           SLOT( setObjectsViewMode( int ) ) );
}


ObjectChooseDialog::~ObjectChooseDialog()
{
  delete d;
}


void ObjectChooseDialog::setObjectsViewMode( int x )
{
  d->viewmode = x;
  d->viewbox->blockSignals( true );
  d->viewbox->setCurrentIndex( x );
  d->viewbox->blockSignals( false );

  d->list->clear();

  set<AObject *>::const_iterator 
    io, eo = d->obj.end(), no = d->sel.end();
  int	i = 0;
  for( io=d->sel.begin(); io!=no; ++io )
    if( !d->psel || d->psel->filter( *io ) )
      {
        d->list->addItem( (*io)->name().c_str() );
        d->list->item( i )->setSelected( true );
        ++i;
      }
  for( io=d->obj.begin(); io!=eo; ++io )
    if( ( !d->psel || d->psel->filter( *io ) ) && d->sel.find( *io ) == no )
      d->list->addItem( (*io)->name().c_str() );

  switch( x )
    {
    case ObjectParamSelect::All:
      {
        set<AObject *>			o = theAnatomist->getObjects();
        set<AObject *>::const_iterator	io, eo = o.end(), no = d->obj.end(), 
          no2 = d->sel.end();

        for( io=o.begin(); io!=eo; ++io )
          if( ( !d->psel || d->psel->filter( *io ) )
              && d->obj.find( *io ) == no
              && d->sel.find( *io ) == no2 )
            d->list->addItem( (*io)->name().c_str() );
      }
      break;
    case ObjectParamSelect::TopLevel:
      {
        set<AObject *>			o = theAnatomist->getObjects();
        set<AObject *>::const_iterator	io, eo = o.end(), no = d->obj.end(), 
          no2 = d->sel.end();

        for( io=o.begin(); io!=eo; ++io )
          if( ( !d->psel || d->psel->filter( *io ) )
              && d->obj.find( *io ) == no
              && d->sel.find( *io ) == no2 
              && ( (*io)->Visible() || (*io)->Parents().empty() ) )
            d->list->addItem( (*io)->name().c_str() );
      }
      break;
    default:
      break;
    }
}


const QListWidget *ObjectChooseDialog::list() const
{
  return d->list;
}


int ObjectChooseDialog::objectsViewMode() const
{
  return d->viewmode;
}


set<AObject *> ObjectChooseDialog::selected() const
{
  set<AObject *>			obj2;
  set<AObject *>::const_iterator io, eo = d->sel.end(), eo2 = d->obj.end(),
    eo3;
  const QListWidget *lb = d->list;
  int i, n = lb->count();

  for( io=d->sel.begin(), i=0; i<n && io!=eo; ++io )
    if( !d->psel || d->psel->filter( *io ) )
      {
        if( lb->item( i )->isSelected() )
          obj2.insert( *io );
        ++i;
      }
  for( io=d->obj.begin(); i<n && io!=eo2; ++io )
    if( ( !d->psel || d->psel->filter( *io ) ) && d->sel.find( *io ) == eo )
      {
        if( lb->item( i )->isSelected() )
          obj2.insert( *io );
        ++i;
      }
  switch( d->viewmode )
    {
    case ObjectParamSelect::All:
      {
        set<AObject *>	o = theAnatomist->getObjects();
        for( io=o.begin(), eo3=o.end(); i<n && io!=eo3; ++io )
          if( ( !d->psel || d->psel->filter( *io ) )
              && d->sel.find( *io ) == eo
              && d->obj.find( *io ) == eo2 )
            {
              if( lb->item( i )->isSelected() )
                obj2.insert( *io );
              ++i;
            }
      }
      break;
    case ObjectParamSelect::TopLevel:
      {
        set<AObject *>	o = theAnatomist->getObjects();
        for( io=o.begin(), eo3=o.end(); i<n && io!=eo3; ++io )
          if( ( !d->psel || d->psel->filter( *io ) )
              && d->sel.find( *io ) == eo
              && d->obj.find( *io ) == eo2
              && ( (*io)->Visible() || (*io)->Parents().empty() ) )
            {
              if( lb->item( i )->isSelected() )
                obj2.insert( *io );
              ++i;
            }
      }
      break;
    default:
      break;
    }

  return obj2;
}


