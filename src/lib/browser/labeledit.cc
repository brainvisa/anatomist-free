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


#include <anatomist/browser/labeledit.h>
#include <anatomist/browser/stringEdit.h>
#include <anatomist/browser/qwObjectBrowser.h>
#include <qlayout.h>
#if QT_VERSION >= 0x040000
namespace Qt {}
using namespace Qt;
#endif


using namespace anatomist;
using namespace carto;
using namespace std;


struct QLabelEdit::Private
{
  Private() {}
  Private( const set<GenericObject *> &, const set<Q3ListViewItem *> & );
  set<GenericObject *>  aobj;
  set<Q3ListViewItem *> items;
};


QLabelEdit::Private::Private( const set<GenericObject *> & ao,
                              const set<Q3ListViewItem *> & item )
  : aobj( ao ), items( item )
{
}


QLabelEdit::QLabelEdit( const string & text, int x, int y, unsigned w, 
			unsigned h, QObjectBrowser* br, GenericObject* ao, 
			const string & att, Q3ListViewItem* item, 
			QWidget* parent, const char* name, WFlags f )
  : QDialog( parent, name, false, f | WDestructiveClose ), _browser( br ), 
    _att( att ), d( new Private )
{
  d->aobj.insert( ao );
  d->items.insert( item );
  drawContents( text, x, y, w, h, name );
}


QLabelEdit::QLabelEdit( const string & text, int x, int y, unsigned w, 
			unsigned h, QObjectBrowser* br,
                        const set<GenericObject*> & ao,
			const string & att,
                        const set<Q3ListViewItem*> & items, 
			QWidget* parent, const char* name, WFlags f )
  : QDialog( parent, name, false, f | WDestructiveClose ), _browser( br ), 
    _att( att ), d( new Private( ao, items ) )
{
  drawContents( text, x, y, w, h, name );
}


QLabelEdit::~QLabelEdit()
{
  _browser->editCancel();
  delete d;
}


void QLabelEdit::drawContents( const std::string & text, int x, int y,
                               unsigned w, unsigned h, const char* name )
{
  if( name )
    setCaption( name );
  setFocusPolicy( StrongFocus );
  QVBoxLayout	*lay = new QVBoxLayout( this, 0, 0, "lay" );
  _le = new QCancelLineEdit( this, "lineedit" );
  _le->setFrame( false );
  _le->setText( text.c_str() );
  lay->addWidget( _le );
  _le->setFocus();
  setGeometry( x, y, w, h );
  connect( _le, SIGNAL( returnPressed() ), this, SLOT( accept() ) );
  connect( (QCancelLineEdit *) _le, SIGNAL( cancel() ), this, 
	   SLOT( reject() ) );
}


set<GenericObject*> QLabelEdit::attributedObjects() const
{
  return d->aobj;
}


GenericObject* QLabelEdit::attributed() const
{
  return *d->aobj.begin();
}


set<Q3ListViewItem*> QLabelEdit::items() const
{
  return d->items;
}


Q3ListViewItem* QLabelEdit::item() const
{
  return *d->items.begin();
}


void QLabelEdit::accept()
{
  hide();
  setResult( Accepted );
  _browser->editValidate();
  QDialog::accept();
}


void QLabelEdit::reject()
{
  static bool	recurs = false;
  if( !recurs )
    {
      recurs = true;

      QDialog::reject();
      close( true );

      recurs = false;
    }
}


string QLabelEdit::text() const
{
  return( _le->text().utf8().data() );
}


void QLabelEdit::receiveValue( const string & val )
{
  string	txt = _le->text().utf8().data();
  if( txt[ txt.size() - 1 ] == '+' )
    _le->setText( (txt + val).c_str() );
  else
    _le->setText( val.c_str() );
  accept();
}
