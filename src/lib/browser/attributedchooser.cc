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

#include <anatomist/browser/attributedchooser.h>
#include <qlayout.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <cartobase/object/attributed.h>

using namespace carto;
using namespace std;
#if QT_VERSION >= 0x040000
namespace Qt {}
using namespace Qt;
#endif

AttributedChooser::AttributedChooser( const GenericObject & ao, 
                                      const SyntaxSet & ss, bool newonly, 
                                      QWidget* parent, const char* name, 
                                      Qt::WFlags f )
  : QDialog( parent, f ), _newonly( newonly ), _ao( &ao ), 
    _syntax( &ss )
{
  setCaption( name );
  setObjectName(name);
  setModal(true);
  QGridLayout	*lay1 = new QGridLayout( this, 3, 2, 10, -1, "lay1" );
  _nameBox = new QComboBox( true, this, "name" );
  _typeBox = new QComboBox( true, this, "type" );
  QLabel	*l1 = new QLabel( tr( "Name :" ), this );
  QLabel	*l2 = new QLabel( tr( "Type :" ), this );
  QPushButton	*okb = new QPushButton( tr( "OK" ), this, "okb" );
  QPushButton	*ccb = new QPushButton( tr( "Cancel" ), this, "ccb" );

  _nameBox->setMinimumWidth( 150 );
  _nameBox->setFixedHeight( _nameBox->sizeHint().height() );
  _typeBox->setMinimumWidth( 150 );
  _typeBox->setFixedHeight( _typeBox->sizeHint().height() );
  l1->setMinimumWidth( l1->sizeHint().width() );
  l1->setFixedHeight( l1->sizeHint().height() );
  l2->setMinimumWidth( l2->sizeHint().width() );
  l2->setFixedHeight( l2->sizeHint().height() );
  okb->setFixedSize( okb->sizeHint() );
  ccb->setFixedSize( ccb->sizeHint() );

  lay1->addWidget( l1, 0, 0 );
  lay1->addWidget( l2, 0, 1 );
  lay1->addWidget( _nameBox, 1, 0 );
  lay1->addWidget( _typeBox, 1, 1 );
  lay1->addWidget( okb, 2, 0, AlignCenter );
  lay1->addWidget( ccb, 2, 1, AlignCenter );

  fillNames();

  connect( _nameBox, SIGNAL( activated( const QString & ) ), this, 
	   SLOT( updateTypes( const QString & ) ) );
  connect( okb, SIGNAL( clicked() ), this, SLOT( accept() ) );
  connect( ccb, SIGNAL( clicked() ), this, SLOT( reject() ) );
}


AttributedChooser::~AttributedChooser()
{
}


void AttributedChooser::fillNames()
{
  _nameBox->clear();

  const SyntaxedInterface	*si = _ao->getInterface<SyntaxedInterface>();
  string		snt;
  if( si && si->hasSyntax() )
    snt = si->getSyntax();
  else
    snt = _ao->type();

  SyntaxSet::const_iterator	iss = _syntax->find( snt );

  if( iss == _syntax->end() )
    return;

  SemanticSet::const_iterator	is, fs = (*iss).second.end();

  for( is=(*iss).second.begin(); is!=fs; ++is )
    if( !_newonly || !_ao->hasProperty( (*is).first.c_str() ) )
      _nameBox->insertItem( (*is).first.c_str() );

  if( _nameBox->count() > 0 )
    _nameBox->setCurrentItem( 0 );

  fillTypes();
}


void AttributedChooser::fillTypes()
{
  _typeBox->clear();

  if( _nameBox->count() == 0 )
    return;

  const SyntaxedInterface	*si = _ao->getInterface<SyntaxedInterface>();
  string		snt;
  if( si && si->hasSyntax() )
    snt = si->getSyntax();
  else
    snt = _ao->type();
  SyntaxSet::const_iterator	iss = _syntax->find( snt );

  if( iss == _syntax->end() )
    return;

  SemanticSet::const_iterator 
    is = (*iss).second.find( _nameBox->currentText().utf8().data() );

  if( is != (*iss).second.end() )
    {
      _typeBox->insertItem( (*is).second.type.c_str() );
      return;
    }

  // name not recognized: enable all types

  set<string>			sstr;
  SyntaxSet::const_iterator	fss = _syntax->end();
  SemanticSet::const_iterator	fs;
  set<string>::const_iterator	ist, fst;

  for( iss=_syntax->begin(); iss!=fss; ++iss )
    for( is=(*iss).second.begin(), fs=(*iss).second.end(); is!=fs; ++is )
      sstr.insert( (*is).second.type );

  for( ist = sstr.begin(), fst=sstr.end(); ist!=fst; ++ist )
    _typeBox->insertItem( (*ist).c_str() );
}


void AttributedChooser::updateTypes( const QString & )
{
  fillTypes();
}


void AttributedChooser::accept()
{
  QString	text = _nameBox->currentText();

  if( text.isEmpty() )
    return;
  text = _typeBox->currentText();
  if( text.isEmpty() )
    return;

  QDialog::accept();
}


string AttributedChooser::attName() const
{
  return( _nameBox->currentText().utf8().data() );
}


string AttributedChooser::attType() const
{
  return( _typeBox->currentText().utf8().data() );
}
