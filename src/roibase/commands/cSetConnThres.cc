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


#include <anatomist/processor/Processor.h>
#include <anatomist/action/levelsetaction.h>
#include <anatomist/commands/cSetConnThres.h>
#include <anatomist/commands/cSetControl.h>
#include <anatomist/object/Object.h>
#include <anatomist/controler/view.h>
#include <anatomist/window/Window.h>
#include <anatomist/window3D/window3D.h>
#include <anatomist/processor/Registry.h>
#include <anatomist/processor/unserializer.h>
#include <anatomist/processor/Serializer.h>
#include <anatomist/processor/context.h>
#include <anatomist/application/roibasemodule.h>
#include <anatomist/graph/Graph.h>
#include <anatomist/graph/GraphObject.h>
#include <cartobase/object/syntax.h>
#include <graph/tree/tree.h>

using namespace anatomist;
using namespace carto;
using namespace std;


SetConnThresholdCommand::SetConnThresholdCommand( AObject* image, AWindow* win, bool assignLowThreshold, 
						  float lowThres, bool assignHighThreshold, float highThres,
						  CommandContext* context )
  : RegularCommand(), SerializingCommand( context ), _image(image), _win(win), _assignLowThreshold(assignLowThreshold), _lowThres(lowThres), 
  _assignHighThreshold(assignHighThreshold), _highThres(highThres)
{
}


SetConnThresholdCommand::~SetConnThresholdCommand()
{
}


bool SetConnThresholdCommand::initSyntax()
{
  SyntaxSet	ss;
  Syntax	& s = ss[ "SetConnThreshold" ];
  
  s[ "image"      ].type = "int";
  s[ "image"      ].needed = true;
  s[ "window"      ].type = "int";
  s[ "window"      ].needed = true;
  s[ "assignLowThreshold"     ].type = "int";
  s[ "assignLowThreshold"     ].needed = false;
  s[ "lowThres"     ].type = "float";
  s[ "lowThres"     ].needed = false;
  s[ "assignHighThreshold" ].type = "int";
  s[ "assignHighThreshold" ].needed = false;
  s[ "highThres" ].type = "float";
  s[ "highThres" ].needed = false;
  Registry::instance()->add( "SetConnThreshold", &read, ss );
  return( true );
}


void SetConnThresholdCommand::doit()
{
  cerr << "SetConnThresholdCommand::doit\n";
  
  set<AWindow *> setWin ;
  setWin.insert( _win ) ;
  
  cerr << "Setting control to ROI" << endl ;
  Command       *cmd = new SetControlCommand( setWin, "RoiControl" ) ;
  theProcessor->execute( cmd ) ;

  cerr << "Setting control to ROI" << endl ;
  AWindow3D * win3d = dynamic_cast<AWindow3D*>( _win ) ;
  if( win3d == 0 ){
    cerr << "Bad window type" << endl ;
    return ;	
  }
  
  cerr << "non null win ptr" << endl ;
  
  
  ControlSwitch * cs = win3d->view()->controlSwitch() ;
  cs->switchToolBoxVisible() ;
  cerr << "tool box visible" << endl ;
  ToolBox * tb = cs->toolBox() ;
  
  anatomist::Action* action = cs->getAction( QT_TRANSLATE_NOOP( "ControlSwitch", "ConnectivityThresholdAction" ) ) ;
  if(!action)
    cerr << "null action ptr" << endl ;
  else
    cerr << "got action : " << action->name() << endl ;
  

  if( tb )
    tb->showPage( action->name() ) ;
  
  cerr << "showPage" << endl ;
  
  RoiLevelSetAction * lsa = dynamic_cast<RoiLevelSetAction *>(action) ;
  lsa->activateLevelSet() ;
  lsa->setDimensionModeTo3D() ;
  lsa->setMixMethod( "LINEAR" ) ;
  lsa->setMixFactor( 50 ) ;

  cerr << "lsa set" << endl ;

  if( _assignLowThreshold )
     lsa->lowLevelChanged( _lowThres ) ;
  if( _assignLowThreshold )
     lsa->lowLevelChanged( _highThres ) ;
}


Command* SetConnThresholdCommand::read( const Tree & com, 
					 CommandContext* context )
{
  AObject *             image = 0 ;
  int                   imageId ;
  AWindow *             win = 0 ;
  int                   winId ;
  bool                  assignLowThreshold ;
  float                 lowThres ;
  bool                  assignHighThreshold ;
  float                 highThres ;
  
/*  int			id;
  set<AObject *>	obj;
  set<AWindow *>	win;
  vector<int>		objId;
  vector<int>		winId;
  unsigned		i, n;*/
  void			*ptr = 0;

  com.getProperty( "image", imageId );
  com.getProperty( "window", winId );
  
  com.getProperty( "assignLowThreshold", assignLowThreshold );
  com.getProperty( "lowThres", lowThres );
  com.getProperty( "assignHighThreshold", assignHighThreshold );
  com.getProperty( "highThres", highThres );

  ptr = context->unserial->pointer( imageId, "AObject" );
  if( ptr )
    image = (AObject *) ptr ;
  else
    cerr << "object id " << imageId << " not found\n";
    
  ptr = context->unserial->pointer( winId, "AWindow" );
  if( ptr )
    win = (AWindow *) ptr ;
  else
    cerr << "window id " << winId << " not found\n";
    
  return new SetConnThresholdCommand( image, win, assignLowThreshold, lowThres, assignHighThreshold, highThres, context );
}


void SetConnThresholdCommand::write( Tree & com, Serializer* ser ) const
{
  int imageId = ser->serialize( _image ) ;
  int winId = ser->serialize( _win ) ;
  
  Tree		*t = new Tree( true, name() );
  t->setProperty( "image", imageId );
  t->setProperty( "window", winId );
  t->setProperty( "assignLowThreshold", _assignLowThreshold );
  t->setProperty( "lowThres", _lowThres );
  t->setProperty( "assignHighThreshold", _assignHighThreshold );
  t->setProperty( "highThres", _highThres );
  
  com.insert( t );
}
