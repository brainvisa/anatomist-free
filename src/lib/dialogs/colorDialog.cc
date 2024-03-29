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

#include <anatomist/dialogs/colorDialog.h>
#include <anatomist/dialogs/colorWidget.h>
#include <qlayout.h>
#include <qpushbutton.h>


QAColorDialog	*QAColorDialog::_dialog = 0;


QAColorDialog::QAColorDialog( QColor init, QWidget * parent, 
			      const char * name, bool modal, bool allowAlpha, 
			      bool allowNeutralAlpha, int initalpha, 
			      bool neutral )
  : QDialog( parent )
{
  setWindowTitle( name );
  setObjectName(name);
  setModal(modal);
  QVBoxLayout	*mainlay = new QVBoxLayout( this );
  mainlay->setContentsMargins( 5, 5, 5, 5 );
  mainlay->setSpacing( 10 );

  _widget = new QAColorWidget( init, this, 0, Qt::WindowFlags(), allowAlpha,
                               allowNeutralAlpha, initalpha, neutral );

  QWidget *butts = new QWidget( this );
  QHBoxLayout *hlay = new QHBoxLayout( butts );
  butts->setLayout( hlay );
  hlay->setSpacing( 5 );
  hlay->setContentsMargins( 0, 0, 0, 0 );
  QPushButton	*okb = new QPushButton( tr( "OK" ), butts );
  hlay->addWidget( okb );
  QPushButton	*ccb = new QPushButton( tr( "Cancel" ), butts );
  hlay->addWidget( ccb );
  okb->setDefault( true );
  okb->setFixedHeight( okb->sizeHint().height() );
  ccb->setFixedHeight( okb->sizeHint().height() );
  okb->setMinimumWidth( okb->sizeHint().width() );
  ccb->setMinimumWidth( ccb->sizeHint().width() );

  mainlay->addWidget( _widget, 0, Qt::AlignCenter );
  mainlay->addWidget( butts, 0, Qt::AlignCenter );

  connect( okb, SIGNAL( clicked() ), this, SLOT( accept() ) );
  connect( ccb, SIGNAL( clicked() ), this, SLOT( reject() ) );
  connect( _widget, SIGNAL( colorChanged() ), this, SIGNAL( colorChanged() ) );
}


QAColorDialog::~QAColorDialog()
{
  if( _dialog == this )
    _dialog = 0;
}


QColor QAColorDialog::getColor( QColor init, QWidget* parent, 
                                const char* name, int* alpha, bool* neutralph )
{
  if( !_dialog )
  {
    _dialog = new QAColorDialog( init, parent, name, true, alpha, neutralph, 
                                 alpha ? *alpha : 255, 
                                 neutralph ? *neutralph : false );
  }
  else
  {
    _dialog->setWindowTitle( name );
    _dialog->relook( init, alpha ? *alpha : 255, alpha, 
                      neutralph ? *neutralph : false, neutralph );
  }

  if( _dialog->exec() )
  {
    if( alpha )
      *alpha = _dialog->alpha();
    if( neutralph )
      *neutralph = _dialog->neutralAlpha();
    return( _dialog->color() );
  }
  else
  {
    QColor	col;
    return( col );
  }
}


void QAColorDialog::relook( const QColor & col, int alpha, bool allowalpha, 
			    bool neutralpha, bool allownalph )
{
  _widget->relook( col, alpha, allowalpha, neutralpha, allownalph );
}


QColor QAColorDialog::color() const
{
  return( _widget->color() );
}


int QAColorDialog::alpha() const
{
  return( _widget->alpha() );
}


bool QAColorDialog::neutralAlpha() const
{
  return( _widget->neutralAlpha() );
}
