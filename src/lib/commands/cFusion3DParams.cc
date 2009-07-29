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

#include <anatomist/commands/cFusion3DParams.h>
#include <anatomist/mobject/Fusion3D.h>
#include <anatomist/processor/Registry.h>
#include <anatomist/processor/Serializer.h>
#include <anatomist/processor/unserializer.h>
#include <anatomist/processor/context.h>
#include <anatomist/application/Anatomist.h>
#include <cartobase/object/syntax.h>
#include <graph/tree/tree.h>

using namespace anatomist;
using namespace carto;
using namespace std;


Fusion3DParamsCommand::Fusion3DParamsCommand( Fusion3D* obj, int method, 
					      int submethod, float depth, 
                                              float step )
  : RegularCommand(), _object( obj ), _method( method ), 
    _submethod( submethod ), _depth( depth ), _step( step )
{
}


Fusion3DParamsCommand::~Fusion3DParamsCommand()
{
}


bool Fusion3DParamsCommand::initSyntax()
{
  SyntaxSet	ss;
  Syntax	& s = ss[ "Fusion3DParams" ];
  
  s[ "object"    ] = Semantic( "int", true );
  s[ "method"    ] = Semantic( "string", false );
  s[ "submethod" ] = Semantic( "string", false );
  s[ "depth"     ] = Semantic( "float", false );
  s[ "step"      ] = Semantic( "float", false );
  Registry::instance()->add( "Fusion3DParams", &read, ss );
  return( true );
}


void Fusion3DParamsCommand::doit()
{
  // cout << "Fusion3DParamsCommand::doit\n";
  if( !theAnatomist->hasObject( _object ) )
    return;
  GLComponent	*gc = _object->glAPI();
  if( !gc )
    return;

  if( _method >= 0 )
    _object->setMethod( (Fusion3D::Method) _method );
  if( _submethod >= 0 )
    _object->setSubMethod( (Fusion3D::SubMethod) _submethod );
  if( _depth > 0 )
    _object->setDepth( _depth );
  if( _step > 0 )
    _object->setStep( _step );

  _object->notifyObservers( this );
}


Command* Fusion3DParamsCommand::read( const Tree & com, 
				      CommandContext* context )
{
  int			id;
  void			*ptr = 0;
  AObject		*obj;
  Fusion3D		*f;
  string		smethod, ssubmethod;
  int			method = -1, submethod = -1;
  float			depth = -1, step = -1;

  com.getProperty( "object", id );

  ptr = context->unserial->pointer( id, "AObject" );
  if( !ptr )
    {
      cerr << "object id " << id << " not found\n";
      return( 0 );
    }

  obj = (AObject *) ptr;
  f = dynamic_cast<Fusion3D *>( obj );
  if( !f )
    {
      cerr << "Fusion3DParams : object " << obj->name() 
	   << " isn't a Fusion3D !\n";
      return( 0 );
    }

  com.getProperty( "method", smethod );
  com.getProperty( "submethod", ssubmethod );
  com.getProperty( "depth", depth );
  com.getProperty( "step", step );

  if( !smethod.empty() )
    {
      static map<string, Fusion3D::Method>	methods;
      if( methods.empty() )
        {
          methods[ "point" ] = Fusion3D::POINT_TO_POINT;
          methods[ "point_offset_internal" ] = Fusion3D::INSIDE_POINT_TO_POINT;
          methods[ "point_offset_external" ] 
            = Fusion3D::OUTSIDE_POINT_TO_POINT;
          methods[ "line" ] = Fusion3D::LINE_TO_POINT;
          methods[ "line_internal" ] = Fusion3D::INSIDE_LINE_TO_POINT;
          methods[ "line_external" ] = Fusion3D::OUTSIDE_LINE_TO_POINT;
          methods[ "sphere" ] = Fusion3D::SPHERE_TO_POINT;
        };
      map<string, Fusion3D::Method>::const_iterator 
        i = methods.find( smethod );
      if( i != methods.end() )
        method = i->second;
    }

  if( !ssubmethod.empty() )
    {
      static map<string, Fusion3D::SubMethod>	submethods;
      if( submethods.empty() )
        {
          submethods[ "max" ] = Fusion3D::MAX;
          submethods[ "min" ] = Fusion3D::MIN;
          submethods[ "mean" ] = Fusion3D::MEAN;
          submethods[ "mean_corrected" ] = Fusion3D::CORRECTED_MEAN;
          submethods[ "mean_enhanced" ] = Fusion3D::ENHANCED_MEAN;
        };
      map<string, Fusion3D::SubMethod>::const_iterator 
        i = submethods.find( ssubmethod );
      if( i != submethods.end() )
        submethod = i->second;
    }

  return( new Fusion3DParamsCommand( f, method, submethod, depth, step ) );
}


void Fusion3DParamsCommand::write( Tree & com, Serializer* ser ) const
{
  Tree	*t = new Tree( true, name() );
  int	id;

  id = ser->serialize( _object );
  t->setProperty( "object", id );
  if( _method >= 0 && _method < 7 )
    {
      static const string smethod[] = 
        {
          "point", "point_offset_internal", "point_offset_external", 
          "line", "line_internal", "line_external", "sphere", 
        };
      t->setProperty( "method", smethod[ _method ] );
    }

  if( _submethod >= 0 && _submethod < 5 )
    {
      static const string ssubmethod[] = 
        {
          "max", "min", "mean", "mean_corrected", "mean_enhanced", 
        };
      t->setProperty( "submethod", ssubmethod[ _submethod ] );
    }

  if( _depth > 0 )
    t->setProperty( "depth", _depth );
  if( _step > 0 )
    t->setProperty( "step", _step );

  com.insert( t );
  // cout << "Fusion3DParamsCommand::write done\n";
}


