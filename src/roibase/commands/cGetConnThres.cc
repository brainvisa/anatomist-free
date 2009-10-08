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


#include <anatomist/controler/view.h>
#include <anatomist/action/levelsetaction.h>
#include <anatomist/window3D/window3D.h>
#include <anatomist/processor/Processor.h>
#include <anatomist/commands/cGetConnThres.h>
#include <anatomist/commands/cSetControl.h>
#include <anatomist/processor/Registry.h>
#include <anatomist/processor/unserializer.h>
#include <anatomist/processor/Serializer.h>
#include <anatomist/processor/context.h>
#include <anatomist/application/roibasemodule.h>
#include <anatomist/graph/Graph.h>
#include <anatomist/graph/GraphObject.h>
#include <cartobase/object/syntax.h>
#include <cartobase/object/pythonwriter.h>
#include <graph/tree/tree.h>

using namespace anatomist;
using namespace carto;
using namespace std;


GetConnThresholdCommand::GetConnThresholdCommand( AObject* image, AWindow* win, const string & requestid, CommandContext* context )
  : RegularCommand(), SerializingCommand( context ), _image(image), _win(win), _requestId(requestid)
{
}


GetConnThresholdCommand::~GetConnThresholdCommand()
{
}


bool GetConnThresholdCommand::initSyntax()
{
  SyntaxSet	ss;
  Syntax	& s = ss[ "GetConnThreshold" ];
  
  s[ "image"      ].type = "int";
  s[ "image"      ].needed = true;
  s[ "window"      ].type = "int";
  s[ "window"      ].needed = true;
  s[ "request_id"      ].type = "string";
  s[ "request_id"      ].needed = true;
  Registry::instance()->add( "GetConnThreshold", &read, ss );
  return( true );
}


void GetConnThresholdCommand::doit()
{
  cout << "GetConnThresholdCommand::doit\n";
  
  set<AWindow *> setWin ;
  setWin.insert( _win ) ;
  Command       *cmd = new SetControlCommand( setWin, "RoiControl" ) ;
  theProcessor->execute( cmd ) ;
  
  AWindow3D * win3d = dynamic_cast<AWindow3D*>( _win ) ;
  if( win3d == 0 ){
    cerr << "Bad window type" << endl ;
    return ;	
  }
  
  ControlSwitch * cs = win3d->view()->controlSwitch() ;
  cs->switchToolBoxVisible() ;
  ToolBox * tb = cs->toolBox() ;
  
  anatomist::Action* action = cs->getAction( QT_TRANSLATE_NOOP( "ControlSwitch", "ConnectivityThresholdAction" ) ) ;
  if( tb )
    tb->showPage( action->name() ) ;
  RoiLevelSetAction * lsa = dynamic_cast<RoiLevelSetAction *>(action) ;
  float realMax = lsa->realMax() ;
  float realMin = lsa->realMin() ;
  
  ofstream	f;
  ostream	*filep = &f;
  filep = context()->ostr;
  if( !filep )
    {
      cerr << "GetInfoCommand: no stream to write to\n";
      return;
    }
  
  
  ostream	& file = *filep;
  
  Object	ex( (GenericObject *) new ValueObject<Dictionary> ) ;
  if( !_requestId.empty() )
    ex->setProperty( "request_id", _requestId );
  
  ex->setProperty( "real_min", realMin ) ;
  ex->setProperty( "real_max", realMax ) ;
  
  file << "'GetInfo'" << endl;
  PythonWriter	pw;
  pw.setSingleLineMode( true );
  pw.attach( file );
  pw.write( *ex, false, false );
  file << endl << flush;
}


Command* GetConnThresholdCommand::read( const Tree & com, 
					 CommandContext* context )
{
  AObject *             image = 0 ;
  int                   imageId ;
  AWindow *             win = 0 ;
  int                   winId ;
  string                reqId ;
/*  int			id;
  set<AObject *>	obj;
  set<AWindow *>	win;
  vector<int>		objId;
  vector<int>		winId;
  unsigned		i, n;*/
  void			*ptr = 0;

  com.getProperty( "image", imageId );
  com.getProperty( "window", winId );
  com.getProperty( "request_id", reqId );
  
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
    
  return new GetConnThresholdCommand( image, win, reqId, context );
}


void GetConnThresholdCommand::write( Tree & com, Serializer* ser ) const
{
  int imageId = ser->serialize( _image ) ;
  int winId = ser->serialize( _win ) ;
  
  Tree		*t = new Tree( true, name() );
  t->setProperty( "image", imageId );
  t->setProperty( "window", winId );
  t->setProperty( "request_id", _requestId );
  
  com.insert( t );
}
