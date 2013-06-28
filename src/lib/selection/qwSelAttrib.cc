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


#include <anatomist/selection/qwSelAttrib.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qcombobox.h>

using namespace std;


set<string>	QSelAttrib::_attribList;
set<string>	QSelAttrib::_valueList;
string		QSelAttrib::_lastAttrib;
string		QSelAttrib::_lastValue;


QSelAttrib::QSelAttrib( QWidget* parent, const char* name ) 
  : QDialog( parent )
{
  setWindowTitle( tr( "AnaQt attribute criterion" ) );
  setObjectName(name);
  setModal(true);

  QGridLayout	*lay = new QGridLayout( this );
  lay->setMargin( 5 );
  lay->setSpacing( 5 );
  QLabel	*lab1 = new QLabel( tr( "Attribute :" ), this );
  lab1->setObjectName( "attr" );
  QLabel	*lab2 = new QLabel( tr( "Value :" ), this );
  lab2->setObjectName( "value" );
  QPushButton	*okb = new QPushButton( tr( "OK" ), this );
  QPushButton	*ccb = new QPushButton( tr( "Cancel" ), this );
  _attrib = new QComboBox( this );
  _attrib->setObjectName( "comboAtt" );
  _attrib->setEditable( true );
  _value = new QComboBox( this );
  _value->setObjectName( "combVal" );
  _value->setEditable( true );

  okb->setDefault( true );

  set<string>::const_iterator	ia, fa=_attribList.end();
  set<string>::const_iterator	iv, fv=_valueList.end();

  for( ia=_attribList.begin(); ia!=fa; ++ia )
      _attrib->addItem( (*ia).c_str() );
  for( iv=_valueList.begin(); iv!=fv; ++iv )
    _value->addItem( (*iv).c_str() );
  _attrib->setEditText( _lastAttrib.c_str() );
  _value->setEditText( _lastValue.c_str() );

  lab1->setFixedHeight( lab1->sizeHint().height() );
  lab2->setFixedHeight( lab2->sizeHint().height() );
  _attrib->setFixedHeight( _attrib->sizeHint().height() );
  _value->setFixedHeight( _value->sizeHint().height() );
  int	mw = _attrib->sizeHint().width();
  if( _value->sizeHint().width() > mw )
    mw = _value->sizeHint().width();
  _attrib->setMinimumWidth( mw );
  _value->setMinimumWidth( mw );
  okb->setFixedHeight( okb->sizeHint().height() );
  ccb->setFixedHeight( ccb->sizeHint().height() );

  lay->addWidget( lab1, 0, 0 );
  lay->addWidget( lab2, 0, 1 );
  lay->addWidget( _attrib, 1, 0 );
  lay->addWidget( _value, 1, 1 );
  lay->addWidget( okb, 2, 0 );
  lay->addWidget( ccb, 2, 1 );

  connect( okb, SIGNAL( clicked() ), this, SLOT( accept() ) );
  connect( ccb, SIGNAL( clicked() ), this, SLOT( reject() ) );
}


QSelAttrib::~QSelAttrib()
{
}


string QSelAttrib::attribute() const
{
  return( _attrib->currentText().toStdString() );
}


string QSelAttrib::value() const
{
  return( _value->currentText().toStdString() );
}


void QSelAttrib::accept()
{
  _lastAttrib = _attrib->currentText().toStdString();
  _lastValue = _value->currentText().toStdString();
  _attribList.insert( _lastAttrib );
  _valueList.insert( _lastValue );
  QDialog::accept();
}
