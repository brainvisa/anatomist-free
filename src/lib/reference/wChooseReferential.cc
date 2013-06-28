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


#include <anatomist/reference/wChooseReferential.h>

#include <anatomist/reference/Referential.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/processor/Processor.h>
#include <anatomist/commands/cAssignReferential.h>
#include <anatomist/window/colorstyle.h>

#include <aims/qtcompat/qvbuttongroup.h>
#include <qpushbutton.h>
#include <qlayout.h>


using namespace anatomist;
using namespace std;


ChooseReferentialWindow::ChooseReferentialWindow( const set<AObject*> &objL,
                                                  const char *name, 
                                                  Qt::WFlags f )
  : QDialog( 0, f ), _objL(objL), 
    _chosenref( 0 )
{
  drawContents( name );
}


ChooseReferentialWindow::ChooseReferentialWindow( const set<AWindow *> &winL,
                                                  const char *name, 
                                                  Qt::WFlags f  )
  : QDialog( 0, f ), _winL(winL)
{ 
  drawContents( name );
}


ChooseReferentialWindow::ChooseReferentialWindow( const set<AWindow *> &winL, 
                                                  const set<AObject*> &objL, 
                                                  const char *name, 
                                                  Qt::WFlags f  )
  : QDialog( 0, f ), 
    _winL(winL), _objL(objL)
{
  drawContents( name );
}


ChooseReferentialWindow::~ChooseReferentialWindow()
{
}


void ChooseReferentialWindow::drawContents( const char *name )
{
  setModal( true );
  setWindowTitle( name );
  setObjectName( name );
  QVBoxLayout	*lay = new QVBoxLayout( this );
  lay->setMargin( 10 );
  lay->setSpacing( 5 );
  QGroupBox *grp = new QGroupBox( tr( "Referential:" ), this );
  QVBoxLayout *glay = new QVBoxLayout( grp );
  lay->addWidget( grp );
  QButtonGroup  *bg = new QButtonGroup( grp );
  int id = 0;

  QPushButton	*but = new QPushButton( tr( "None" ), grp );
  glay->addWidget( but );
  bg->addButton( but, id++ );
  setQtColorStyle( but );
  but = new QPushButton( tr( "New" ), grp );
  glay->addWidget( but );
  bg->addButton( but, id++ );
  setQtColorStyle( but );

  set<Referential *>			refs = theAnatomist->getReferentials();
  set<Referential *>::const_iterator	ir, fr=refs.end();
  string                                refname;
  QString                               qrefname;
  bool hidden;

  for( ir=refs.begin(); ir!=fr; ++ir )
  {
    if( !(*ir)->header().getProperty( "hidden", hidden )
        || !hidden )
    {
      if( (*ir)->header().getProperty( "name", refname ) )
        qrefname = refname.c_str();
      else
        qrefname = tr( "Existing one" );
      but = new QPushButton( qrefname, grp );
      glay->addWidget( but );
      bg->addButton( but, id++ );
      setQtColorStyle( but );
      but->setPalette( QPalette( QColor( (*ir)->Color().red(),
                                        (*ir)->Color().green(),
                                        (*ir)->Color().blue() ) ) );
    }
  }

  connect( bg, SIGNAL( buttonClicked( int ) ), this,
            SLOT( chooseRef( int ) ) );
}


void ChooseReferentialWindow::update( const Observable*, void* arg )
{
  if (arg == 0)
    {
      cout << "called obsolete ChooseReferentialWindow::update( obs, NULL )\n";
      delete this;
    }
}


void ChooseReferentialWindow::unregisterObservable( Observable* o )
{
  Observer::unregisterObservable( o );
  delete this;
}


void ChooseReferentialWindow::chooseRef( int num )
{
  Referential	*ref = 0;
  int		id = 0;

  if( num == 1 )	// new
  {
    id = -1;
  }
  else if( num > 1 )	// old
  {
    set<Referential *>		refs = theAnatomist->getReferentials();
    set<Referential *>::const_iterator	ir, er = refs.end();
    int				i;
    bool hidden;

    for( i=1, ir=refs.begin(); ir!=er; ++ir )
    {
      if( !(*ir)->header().getProperty( "hidden", hidden )
          || !hidden )
      {
        ++i;
        if( i == num )
          break;
      }
    }
    ref = *ir;
  }	// else: none

  if( !_objL.empty() || !_winL.empty() || id < 0 )
  {
    if( !ref )
      ref = theAnatomist->centralReferential();
    AssignReferentialCommand	*com 
      = new AssignReferentialCommand( ref, _objL, _winL, id );
    theProcessor->execute( com );
    ref = com->ref();
  }
  _chosenref = ref;

  accept();
  close();
}


Referential* ChooseReferentialWindow::selectedReferential() const
{
  return _chosenref;
}


