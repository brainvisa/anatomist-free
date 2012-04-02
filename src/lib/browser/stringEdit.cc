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


#include <anatomist/browser/stringEdit.h>
#include <aims/qtcompat/qkeyevent.h>
#include <qlayout.h>

using namespace std;
#if QT_VERSION >= 0x040000
namespace Qt {}
using namespace Qt;
#endif

void QCancelLineEdit:: keyPressEvent( QKeyEvent* kev )
{
  if( kev->key() == Key_Escape )
    {
      kev->accept();
      emit cancel();
    }
  else
    {
      QLineEdit::keyPressEvent( kev );
    }
}




QStringEdit::QStringEdit( const string & text, int x, int y, int w, int h, 
			  QWidget* parent, const char* name, WFlags f )
  : QDialog( parent, f )
{
  if( name ){
    setCaption( name );
    setObjectName( name );
  }
  setModal(true);
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


QStringEdit::~QStringEdit()
{
}
