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


#include <anatomist/commands/cPaintParams.h>
#include <anatomist/processor/Registry.h>
/*#include <anatomist/processor/unserializer.h>
#include <anatomist/processor/Serializer.h>*/
// #include <anatomist/processor/context.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/window/controlledWindow.h>
#include <anatomist/application/roibasemodule.h>
#include <anatomist/controler/controlswitch.h>
#include <anatomist/action/paintaction.h>
#include <cartobase/object/syntax.h>
#include <graph/tree/tree.h>

using namespace anatomist;
using namespace carto;
using namespace std;


bool PaintParamsCommand::_helper( initSyntax() );


PaintParamsCommand::PaintParamsCommand( Object params )
  : RegularCommand(), _params( params )
{
}


PaintParamsCommand::~PaintParamsCommand()
{
}


bool PaintParamsCommand::initSyntax()
{
  SyntaxSet     ss;
  Syntax        & s = ss[ "PaintParams" ];

  s[ "brush_type"           ] = Semantic( "string", false );
  s[ "brush_size"           ] = Semantic( "float", false );
  s[ "line_mode"            ] = Semantic( "int", false );
  s[ "replace_mode"         ] = Semantic( "int", false );
  s[ "follow_linked_cursor" ] = Semantic( "int", false );
  s[ "millimeter_mode"      ] = Semantic( "int", false );
  s[ "region_transparency"  ] = Semantic( "float", false );
  Registry::instance()->add( "PaintParams", &read, ss );
  return true;
}


void
PaintParamsCommand::doit()
{
  PaintAction *pa = 0;
  /* Retreive a working instance of PaintAction.
     We have to search it through existing windows, since they actually belong to the
     control system. It's a bit painful...
   */
  set<AWindow *> wl = theAnatomist->getWindows();
  set<AWindow *>::iterator iw, ew = wl.end();
  for( iw=wl.begin(); iw!=ew; ++iw )
  {
    ControlledWindow *cw = dynamic_cast<ControlledWindow *>( *iw );
    if( cw )
    {
      View *view = cw->view();
      if( view )
      {
        ControlSwitch *cs = view->controlSwitch();
        pa = dynamic_cast<PaintAction *>( cs->getAction( "PaintAction" ) );
        if( pa )
          break;
      }
    }
  }
  if( !pa )
    return;

  try
  {
    string btype = _params->getProperty( "brush_type" )->getString();
    if( btype == "point" )
      pa->brushToPoint();
    else if( btype == "square" )
      pa->brushToSquare();
    else if( btype == "disk" )
      pa->brushToDisk();
    else if( btype == "sphere" || btype == "ball" )
      pa->brushToBall();
  }
  catch( ... )
  {
  }
  try
  {
    float bsize = (float) _params->getProperty( "brush_size" )->getScalar();
    pa->setSize( bsize );
  }
  catch( ... )
  {
  }
  try
  {
    bool lmode = (bool) _params->getProperty( "line_mode" )->getScalar();
    if( lmode )
      pa->lineOn();
    else
      pa->lineOff();
  }
  catch( ... )
  {
  }
  try
  {
    bool rmode = (bool) _params->getProperty( "replace_mode" )->getScalar();
    if( rmode )
      pa->replaceOn();
    else
      pa->replaceOff();
  }
  catch( ... )
  {
  }
  try
  {
    bool follow
      = (bool) _params->getProperty( "follow_linked_cursor" )->getScalar();
    if( follow )
      pa->followingLinkedCursorOn();
    else
      pa->followingLinkedCursorOff();
  }
  catch( ... )
  {
  }
  try
  {
    bool mmmode
      = (bool) _params->getProperty( "millimeter_mode" )->getScalar();
    if( mmmode )
      pa->brushToMm();
    else
      pa->brushToVoxel();
  }
  catch( ... )
  {
  }
  try
  {
	  float rtransparency = (float)_params->getProperty( "region_transparency" )->getScalar();
      pa->changeRegionTransparency( rtransparency );
  }
  catch( ... )
  {
  }
}


Command* PaintParamsCommand::read( const Tree & com, CommandContext* context )
{
  Object params = Object::value( carto::Dictionary() );
  int           iobj, rid, bkid, wb = 1, nodup = false;
  AGraph        *gr;
  void          *ptr;
  string        name, syntax;

  params->copyProperties( Object::reference( const_cast<Tree &>(
    com ).getValue()  ) );

  return new PaintParamsCommand( params );
}


void PaintParamsCommand::write( Tree & com, Serializer* ) const
{
  Tree  *t = new Tree( true, name() );
  int   obj;

  t->copyProperties( _params );

  com.insert( t );
}
