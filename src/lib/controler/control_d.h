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

#ifndef ANATOMIST_CONTROLER_CONTROL_D_H
#define ANATOMIST_CONTROLER_CONTROL_D_H

#include "anatomist/controler/control.h"
#include "anatomist/controler/actionpool.h"
#include <cartobase/exception/assert.h>
#include <typeinfo>

namespace anatomist
{

template<typename T>
Control::KeyActionLinkOf<T>::KeyActionLinkOf() : actionInstance(NULL), actionCallback(NULL) {}
 
 
template<typename T>
Control::KeyActionLinkOf<T>::KeyActionLinkOf( Action * action, Callback actionCb) : 
  actionCallback(actionCb) 
{
  actionInstance = dynamic_cast<T *>(action) ;
  ASSERT( actionInstance != 0 ) ;
}


template<typename T>
void
Control::KeyActionLinkOf<T>::execute() 
{
  (actionInstance->*(actionCallback))();
}


template<typename T>
Control::KeyActionLink*
Control::KeyActionLinkOf<T>::clone() const 
{
  return new KeyActionLinkOf<T>(actionInstance, actionCallback);
}


template<typename T>
Control::MouseActionLinkOf<T>::MouseActionLinkOf() : actionInstance(NULL), actionCallback(NULL) {}


template<typename T>
Control::MouseActionLinkOf<T>::MouseActionLinkOf( Action * action, Callback actionCb) : 
  actionCallback(actionCb) 
{
  actionInstance = dynamic_cast<T *>(action) ;
  ASSERT( actionInstance );
}


template<typename T>
Action*
Control::MouseActionLinkOf<T>::action()
{
  return actionInstance;
}


template<typename T>
void
Control::MouseActionLinkOf<T>::execute(  int x, int y, int globalX, int globalY ) 
{
  (actionInstance->*(actionCallback))( x, y, globalX, globalY );
}


template<typename T>
Control::MouseActionLink*
Control::MouseActionLinkOf<T>::clone() const 
{
  return new MouseActionLinkOf<T>(actionInstance, actionCallback);
}


template<typename T>
Control::WheelActionLinkOf<T>::WheelActionLinkOf() : actionInstance(NULL), actionCallback(NULL) {}


template<typename T>
Control::WheelActionLinkOf<T>::WheelActionLinkOf( Action * action, Callback actionCb) : 
  actionCallback(actionCb) 
{
  actionInstance = dynamic_cast<T *>(action) ;
  ASSERT( actionInstance != 0 ) ;
}


template<typename T>
void
Control::WheelActionLinkOf<T>::execute( int delta, int x, int y, int globalX, int globalY ) 
{
  (actionInstance->*(actionCallback))( delta, x, y, globalX, globalY );
}


template<typename T>
Control::WheelActionLink*
Control::WheelActionLinkOf<T>::clone() const 
{
  return new WheelActionLinkOf<T>(actionInstance, actionCallback);
}


template<typename T>
Control::FocusActionLinkOf<T>::FocusActionLinkOf() : actionInstance(NULL), actionCallback(NULL) {}


template<typename T>
Control::FocusActionLinkOf<T>::FocusActionLinkOf( Action * action, Callback actionCb) : 
  actionCallback(actionCb) 
{
  actionInstance = dynamic_cast<T *>(action) ;
  ASSERT( actionInstance != 0 ) ;
}


template<typename T>
void
Control::FocusActionLinkOf<T>::execute() 
{
  (actionInstance->*(actionCallback))();
}


template<typename T>
Control::FocusActionLink*
Control::FocusActionLinkOf<T>::clone() const 
{
  return new FocusActionLinkOf<T>(actionInstance, actionCallback);
}


template<typename T>
Control::EnterLeaveActionLinkOf<T>::EnterLeaveActionLinkOf() : actionInstance(NULL), actionCallback(NULL) {}

template<typename T>
Control::EnterLeaveActionLinkOf<T>::EnterLeaveActionLinkOf( Action * action, Callback actionCb) : 
  actionCallback(actionCb) 
{
  actionInstance = dynamic_cast<T *>(action) ;
  ASSERT( actionInstance != 0 ) ;
}


template<typename T>
void
Control::EnterLeaveActionLinkOf<T>::execute() 
{
  (actionInstance->*(actionCallback))();
}


template<typename T>
Control::EnterLeaveActionLink*
Control::EnterLeaveActionLinkOf<T>::clone() const 
{
  return new EnterLeaveActionLinkOf<T>(actionInstance, actionCallback);
}


template<typename T>
Control::PaintActionLinkOf<T>::PaintActionLinkOf() : actionInstance(NULL), actionCallback(NULL) {}


template<typename T>
Control::PaintActionLinkOf<T>::PaintActionLinkOf( Action * action, Callback actionCb) : 
  actionCallback(actionCb) 
{
  actionInstance = dynamic_cast<T *>(action) ;
  ASSERT( actionInstance != 0 ) ;
}


template<typename T>
void
Control::PaintActionLinkOf<T>::execute( int xOffset, int yOffset, int height, int width ) 
{
  (actionInstance->*(actionCallback))( xOffset, yOffset, height, width );
}


template<typename T>
Control::PaintActionLink*
Control::PaintActionLinkOf<T>::clone() const 
{
  return new PaintActionLinkOf<T>(actionInstance, actionCallback);
}


template<typename T>
Control::MoveOrDragActionLinkOf<T>::MoveOrDragActionLinkOf() : actionInstance(NULL), actionCallback(NULL) {}


template<typename T>
Control::MoveOrDragActionLinkOf<T>::MoveOrDragActionLinkOf( Action * action, Callback actionCb) : 
  actionCallback(actionCb) 
{
  actionInstance = dynamic_cast<T *>(action) ;
  ASSERT( actionInstance != 0 ) ;
}


template<typename T>
void
Control::MoveOrDragActionLinkOf<T>::execute( int posX, int posY, int oldPosX, int oldPosY ) 
{
  (actionInstance->*(actionCallback))( posX, posY, oldPosX, oldPosY );
}


template<typename T>
Control::MoveOrDragActionLink*
Control::MoveOrDragActionLinkOf<T>::clone() const 
{
  return new MoveOrDragActionLinkOf<T>(actionInstance, actionCallback);
}


template<typename T>
Control::DropActionLinkOf<T>::DropActionLinkOf() : actionInstance(NULL), actionCallback(NULL) {}


template<typename T>
Control::DropActionLinkOf<T>::DropActionLinkOf( Action * action, Callback actionCb) : 
  actionCallback(actionCb) 
{
  actionInstance = dynamic_cast<T *>(action) ;
  ASSERT( actionInstance != 0 ) ;
}


template<typename T>
void
Control::DropActionLinkOf<T>::execute( int posX, int posY, Control::DropAction action ) 
{
  (actionInstance->*(actionCallback))( posX, posY, action );
}


template<typename T>
Control::DropActionLink*
Control::DropActionLinkOf<T>::clone() const 
{
  return new DropActionLinkOf<T>(actionInstance, actionCallback);
}


template<typename T>
Control::ResizeActionLinkOf<T>::ResizeActionLinkOf() : actionInstance(NULL), actionCallback(NULL) {}

template<typename T>
Control::ResizeActionLinkOf<T>::ResizeActionLinkOf( Action * action, Callback actionCb) : 
  actionCallback(actionCb) 
{
  actionInstance = dynamic_cast<T *>(action) ;
  ASSERT( actionInstance != 0 ) ;
}


template<typename T>
void
Control::ResizeActionLinkOf<T>::execute( int height, int width, int oldHeight, int oldWidth ) 
{
  (actionInstance->*(actionCallback))( height, width, oldHeight, oldWidth );
}


template<typename T>
Control::ResizeActionLink*
Control::ResizeActionLinkOf<T>::clone() const 
{
  return new ResizeActionLinkOf<T>(actionInstance, actionCallback);
}


template<typename T>
Control::ShowHideActionLinkOf<T>::ShowHideActionLinkOf() : actionInstance(NULL), actionCallback(NULL) {}

template<typename T>
Control::ShowHideActionLinkOf<T>::ShowHideActionLinkOf( Action * action, Callback actionCb) : 
  actionCallback(actionCb) 
{
  actionInstance = dynamic_cast<T *>(action) ;
  ASSERT( actionInstance != 0 ) ;
}


template<typename T>
void
Control::ShowHideActionLinkOf<T>::execute( bool spontaneous ) 
{
  (actionInstance->*(actionCallback))(spontaneous);
}


template<typename T>
Control::ShowHideActionLink*
Control::ShowHideActionLinkOf<T>::clone() const 
{
  return new ShowHideActionLinkOf<T>(actionInstance, actionCallback);
}


template<typename T>
Control::SelectionChangedActionLinkOf<T>::SelectionChangedActionLinkOf()
  : actionInstance( 0 ), actionCallback( 0 )
{
}


template<typename T>
Control::SelectionChangedActionLinkOf<T>::SelectionChangedActionLinkOf(
  Action * action, Callback actionCb )
  : actionCallback( actionCb )
{
  actionInstance = dynamic_cast<T *>( action );
  ASSERT( actionInstance != 0 );
}


template<typename T>
void
Control::SelectionChangedActionLinkOf<T>::execute()
{
  (actionInstance->*(actionCallback))();
}


template<typename T>
Control::SelectionChangedActionLink*
Control::SelectionChangedActionLinkOf<T>::clone() const
{
  return new SelectionChangedActionLinkOf<T>( actionInstance, actionCallback );
}


}


#endif
