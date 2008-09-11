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

#include <anatomist/application/roibasemodule.h>
#include <anatomist/control/roimanagementcontrol.h>
#include <anatomist/action/roimanagementaction.h>
#include <anatomist/controler/actionpool.h>
#include <anatomist/controler/control_d.h>
#include <anatomist/window3D/control3D.h>
#include <anatomist/window3D/trackball.h>
#include <anatomist/misc/error.h>
#include <qlabel.h>
#include <qobject.h>

using namespace anatomist ;


Control *
RoiManagementControl::creator( ) 
{
  RoiManagementControl * pc = new RoiManagementControl() ;
  
  return ( pc ) ;
}


RoiManagementControl::RoiManagementControl( ) : Control( 101, "RoiManagementControl" )
{
  
}

RoiManagementControl::RoiManagementControl( const RoiManagementControl& c) : Control(c)
{
  
}

RoiManagementControl::~RoiManagementControl() 
{

}

void 
RoiManagementControl::eventAutoSubscription( ActionPool * actionPool )
{
  mousePressButtonEventSubscribe
    ( Qt::RightButton, Qt::NoButton, 
      MouseActionLinkOf<MenuAction>( actionPool->action( "MenuAction" ), 
				     &MenuAction::execMenu ) );
  

  // general window shortcuts

  keyPressEventSubscribe( Qt::Key_W, Qt::ControlButton, 
			  KeyActionLinkOf<WindowActions>
			  ( actionPool->action( "WindowActions" ), 
			    &WindowActions::close ) );
  keyPressEventSubscribe( Qt::Key_F9, Qt::NoButton, 
			  KeyActionLinkOf<WindowActions>
			  ( actionPool->action( "WindowActions" ), 
			    &WindowActions::toggleFullScreen ) );
  keyPressEventSubscribe( Qt::Key_F10, Qt::NoButton, 
			  KeyActionLinkOf<WindowActions>
			  ( actionPool->action( "WindowActions" ), 
			    &WindowActions::toggleShowTools ) );

}

