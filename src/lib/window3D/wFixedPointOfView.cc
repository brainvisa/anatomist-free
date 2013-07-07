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


#include <anatomist/window3D/wFixedPointOfView.h>
#include <anatomist/window3D/window3D.h>
#include <qlayout.h>
#include <aims/qtcompat/qvbuttongroup.h>
#include <qpushbutton.h>
#include <iostream>

using namespace anatomist;
using namespace std;


struct FixedPointOfViewWindow_PrivateData
{
  FixedPointOfViewWindow_PrivateData();

  float	straight[6][4];
  float	edge[12][4];
  float	corner[8][4];
  bool	destroying;
};


FixedPointOfViewWindow_PrivateData::FixedPointOfViewWindow_PrivateData() 
  : destroying( false )
{
  float	c = 1. / sqrt( 2. );

  straight[0][0] = c;
  straight[0][1] = 0;
  straight[0][2] = 0;
  straight[0][3] = c;

  straight[1][0] = 0;
  straight[1][1] = c;
  straight[1][2] = c;
  straight[1][3] = 0;

  straight[2][0] = 0.5;
  straight[2][1] = 0.5;
  straight[2][2] = 0.5;
  straight[2][3] = 0.5;

  straight[3][0] = 0.5;
  straight[3][1] = -0.5;
  straight[3][2] = -0.5;
  straight[3][3] = 0.5;

  straight[4][0] = 0;
  straight[4][1] = 0;
  straight[4][2] = 0;
  straight[4][3] = 1;

  straight[5][0] = 1;
  straight[5][1] = 0;
  straight[5][2] = 0;
  straight[5][3] = 0;

  edge[0][0] = -0.426086;
  edge[0][1] = 0;
  edge[0][2] = 0;
  edge[0][3] = -0.904879;

  edge[1][0] = -0.902674;
  edge[1][1] = 0;
  edge[1][2] = 0;
  edge[1][3] = -0.430661;

  edge[2][0] = -0.663981;
  edge[2][1] = -0.240523;
  edge[2][2] = -0.238736;
  edge[2][3] = -0.666549;

  edge[3][0] = -0.664493;
  edge[3][1] = 0.235447;
  edge[3][2] = 0.241882;
  edge[3][3] = -0.666717;

  edge[4][0] = 0;
  edge[4][1] = 0.428861;
  edge[4][2] = 0.902998;
  edge[4][3] = 0;

  edge[5][0] = 0;
  edge[5][1] = -0.902638;
  edge[5][2] = -0.432589;
  edge[5][3] = 0;

  edge[6][0] = -0.244157;
  edge[6][1] = -0.664259;
  edge[6][2] = -0.664403;
  edge[6][3] = -0.240247;

  edge[7][0] = 0.236937;
  edge[7][1] = -0.666506;
  edge[7][2] = -0.665614;
  edge[7][3] = 0.237873;

  edge[8][0] = -0.272457;
  edge[8][1] = -0.271965;
  edge[8][2] = -0.65204;
  edge[8][3] = -0.653181;

  edge[9][0] = -0.654997;
  edge[9][1] = -0.652452;
  edge[9][2] = -0.267328;
  edge[9][3] = -0.2717;

  edge[10][0] = -0.272103;
  edge[10][1] = 0.270793;
  edge[10][2] = 0.651146;
  edge[10][3] = -0.654707;

  edge[11][0] = 0.653748;
  edge[11][1] = -0.65496;
  edge[11][2] = -0.26623;
  edge[11][3] = 0.269745;

  corner[0][0] = -0.423109;
  corner[0][1] = -0.178352;
  corner[0][2] = -0.335986;
  corner[0][3] = -0.822358;

  corner[1][0] = -0.423109;
  corner[1][1] = 0.178352;
  corner[1][2] = 0.335986;
  corner[1][3] = -0.822358;

  corner[2][0] = -0.817231;
  corner[2][1] = -0.347051;
  corner[2][2] = -0.174582;
  corner[2][3] = -0.425695;

  corner[3][0] = -0.821242;
  corner[3][1] = 0.339998;
  corner[3][2] = 0.173608;
  corner[3][3] = -0.424047;

  corner[4][0] = 0.176549;
  corner[4][1] = 0.425147;
  corner[4][2] = 0.821291;
  corner[4][3] = 0.336996;

  corner[5][0] = 0.151969;
  corner[5][1] = -0.453031;
  corner[5][2] = -0.828889;
  corner[5][3] = 0.29087;

  corner[6][0] = -0.332481;
  corner[6][1] = -0.822903;
  corner[6][2] = -0.424042;
  corner[6][3] = -0.180186;

  corner[7][0] = -0.299189;
  corner[7][1] = 0.824732;
  corner[7][2] = 0.450861;
  corner[7][3] = -0.16442;
}


FixedPointOfViewWindow::FixedPointOfViewWindow( AWindow3D* win, 
						QWidget* parent, 
						const char* name )
  : QWidget( parent ), Observer(), _window( win ),
    _pdat( new FixedPointOfViewWindow_PrivateData )
{
  //_window->addObserver( this );
  setWindowTitle( tr( "Standard point of view" ) );
  setObjectName(name);
  QHBoxLayout	*lay = new QHBoxLayout( this );
  lay->setMargin( 10 );
  lay->setSpacing( 10 );

  QGroupBox *straight = new QGroupBox( tr( "Straight view :" ), this );
  QVBoxLayout *vlay = new QVBoxLayout( straight );
  straight->setLayout( vlay );
  QButtonGroup  *bg = new QButtonGroup( straight );
  int id = 0;

  QPushButton *but = new QPushButton( tr( "Front" ), straight );
  bg->addButton( but, id++ );
  vlay->addWidget( but );

  but = new QPushButton( tr( "Back" ), straight );
  bg->addButton( but, id++ );
  vlay->addWidget( but );

  but = new QPushButton( tr( "Left" ), straight );
  bg->addButton( but, id++ );
  vlay->addWidget( but );

  but = new QPushButton( tr( "Right" ), straight );
  bg->addButton( but, id++ );
  vlay->addWidget( but );

  but = new QPushButton( tr( "Top" ), straight );
  bg->addButton( but, id++ );
  vlay->addWidget( but );

  but = new QPushButton( tr( "Bottom" ), straight );
  bg->addButton( but, id++ );
  vlay->addWidget( but );
  vlay->addItem( new QSpacerItem( 1, 1, QSizePolicy::Expanding,
                                  QSizePolicy::Expanding ) );
  connect( bg, SIGNAL( buttonClicked( int ) ), this,
           SLOT( straightPOV( int ) ) );

  QGroupBox  *edge = new QGroupBox( tr( "Edge view :" ), this );
  vlay = new QVBoxLayout( edge );
  edge->setLayout( vlay );
  bg = new QButtonGroup( edge );
  id = 0;

  but = new QPushButton( tr( "Front top" ), edge );
  bg->addButton( but, id++ );
  vlay->addWidget( but );

  but = new QPushButton( tr( "Front bottom" ), edge );
  bg->addButton( but, id++ );
  vlay->addWidget( but );

  but = new QPushButton( tr( "Front left" ), edge );
  bg->addButton( but, id++ );
  vlay->addWidget( but );

  but = new QPushButton( tr( "Front right" ), edge );
  bg->addButton( but, id++ );
  vlay->addWidget( but );

  but = new QPushButton( tr( "Back top" ), edge );
  bg->addButton( but, id++ );
  vlay->addWidget( but );

  but = new QPushButton( tr( "Back bottom" ), edge );
  bg->addButton( but, id++ );
  vlay->addWidget( but );

  but = new QPushButton( tr( "Back left" ), edge );
  bg->addButton( but, id++ );
  vlay->addWidget( but );

  but = new QPushButton( tr( "Back right" ), edge );
  bg->addButton( but, id++ );
  vlay->addWidget( but );

  but = new QPushButton( tr( "Left top" ), edge );
  bg->addButton( but, id++ );
  vlay->addWidget( but );

  but = new QPushButton( tr( "Left bottom" ), edge );
  bg->addButton( but, id++ );
  vlay->addWidget( but );

  but = new QPushButton( tr( "Right top" ), edge );
  bg->addButton( but, id++ );
  vlay->addWidget( but );

  but = new QPushButton( tr( "Right bottom" ), edge );
  bg->addButton( but, id++ );
  vlay->addWidget( but );
  vlay->addItem( new QSpacerItem( 1, 1, QSizePolicy::Expanding,
                                  QSizePolicy::Expanding ) );

  connect( bg, SIGNAL( buttonClicked( int ) ), this,
           SLOT( edgePOV( int ) ) );

  QGroupBox *corner = new QGroupBox( tr( "Corner view :" ), this );
  vlay = new QVBoxLayout( corner );
  corner->setLayout( vlay );
  bg = new QButtonGroup( corner );
  id = 0;

  but = new QPushButton( tr( "Front top left" ), corner );
  bg->addButton( but, id++ );
  vlay->addWidget( but );

  but = new QPushButton( tr( "Front top right" ), corner );
  bg->addButton( but, id++ );
  vlay->addWidget( but );

  but = new QPushButton( tr( "Front bottom left" ), corner );
  bg->addButton( but, id++ );
  vlay->addWidget( but );

  but = new QPushButton( tr( "Front bottom right" ), corner );
  bg->addButton( but, id++ );
  vlay->addWidget( but );

  but = new QPushButton( tr( "Back top left" ), corner );
  bg->addButton( but, id++ );
  vlay->addWidget( but );

  but = new QPushButton( tr( "Back top right" ), corner );
  bg->addButton( but, id++ );
  vlay->addWidget( but );

  but = new QPushButton( tr( "Back bottom left" ), corner );
  bg->addButton( but, id++ );
  vlay->addWidget( but );

  but = new QPushButton( tr( "Back bottom right" ), corner );
  bg->addButton( but, id++ );
  vlay->addWidget( but );
  vlay->addItem( new QSpacerItem( 1, 1, QSizePolicy::Expanding,
                                  QSizePolicy::Expanding ) );

  connect( bg, SIGNAL( buttonClicked( int ) ), this,
           SLOT( cornerPOV( int ) ) );

  lay->addWidget( straight );
  lay->addWidget( edge );
  lay->addWidget( corner );
}


FixedPointOfViewWindow::~FixedPointOfViewWindow()
{
  _pdat->destroying = true;
  _window->povWinDestroyed();
  cleanupObserver();
  //_window->deleteObserver( this );
  delete _pdat;
}


void FixedPointOfViewWindow::update( const Observable*, void* arg )
{
  if( arg == 0 )
    {
      cout << "called obsolete FixedPointOfViewWindow::update( obs, NULL )\n";
      delete this;
    }
}


void FixedPointOfViewWindow::unregisterObservable( Observable* o )
{
  Observer::unregisterObservable( o );
  if( !_pdat->destroying )
    delete this;
}


void FixedPointOfViewWindow::straightPOV( int num )
{
  _window->setViewPoint( _pdat->straight[num], 1. );
}


void FixedPointOfViewWindow::edgePOV( int num )
{
  _window->setViewPoint( _pdat->edge[num], 1. );
}


void FixedPointOfViewWindow::cornerPOV( int num )
{
  _window->setViewPoint( _pdat->corner[num], 1. );
}
