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


#include <anatomist/control/listboxeditor.h>
#include <anatomist/browser/stringEdit.h>
#include <anatomist/browser/qwObjectBrowser.h>
#include <qlayout.h>


using namespace anatomist;
using namespace carto;
using namespace std;


listboxeditor::listboxeditor( const string & /*text*/, int x, int y,
                              unsigned w,
                              unsigned h, QObjectBrowser* br,
                              GenericObject* ao,
                              const string & att, QTreeWidgetItem* item,
                              QWidget* parent, const char* name,
                              Qt::WindowFlags f )
  : QDialog( parent, f ), _browser( br ),
    _ao( ao ), _att( att.c_str() ), 
    _item( item )
{
  if( name )
  {
    setObjectName(name);
    setWindowTitle( name );
  }
  setModal(false);
  setAttribute(Qt::WA_DeleteOnClose);

  setFocusPolicy( Qt::StrongFocus );

  QComboBox *boxchoice = new QComboBox( parent );
  boxchoice->setObjectName( name );
  //boxchoice->isPopup = true;
  //boxchoice->isModal = true

  boxchoice->addItem( "Both meridian and parallel" );
  boxchoice->addItem( "meridian" );
  boxchoice->addItem( "parallel" );

  QVBoxLayout	*lay = new QVBoxLayout( this );
  lay->setMargin( 0 );
  lay->setSpacing( 0 );
  _te = new QCancelLineEdit( this, "typeedit" );
  _te->setFrame( false );
//  	_te->setText( text.c_str() );
  lay->addWidget( _te );
  _te->setFocus();
  setGeometry( x, y, w, h );

  connect( _te, SIGNAL( clicked() ), this, SLOT( change(boxchoice ) ) );
  //connect( _te, SIGNAL( returnPressed() ), this, SLOT( accept() ) );
  //connect( (QCancelLineEdit *) _te, SIGNAL( cancel() ), this,
  //   	SLOT( reject() ) );
}


listboxeditor::~listboxeditor()
{
  _browser->editCancel();
}


void listboxeditor::change(QComboBox *boxchoice )
{
  hide();
  _att = boxchoice->currentText();
  _browser->editValidate();
  //QDialog::accept();
  close();
}

QString listboxeditor::get_result()
{
	return(_att);
}

void listboxeditor::reject()
{
  static bool	recurs = false;
  if( !recurs )
    {
      recurs = true;

      QDialog::reject();
      close();

      recurs = false;
    }
}

/*
string listboxeditor::send_text() const
{
  return( _te->text().utf8().data() );
}
*/
/*
void listboxeditor::receiveValue( const string & val )
{
  string	txt = _le->text().utf8().data();
  if( txt[ txt.size() - 1 ] == '+' )
    _le->setText( (txt + val).c_str() );
  else
    _le->setText( val.c_str() );
  accept();
}
*/
