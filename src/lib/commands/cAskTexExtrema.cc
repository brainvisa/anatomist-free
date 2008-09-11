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

#include <anatomist/commands/cAskTexExtrema.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/object/Object.h>
#include <anatomist/processor/Registry.h>
#include <anatomist/processor/unserializer.h>
#include <anatomist/processor/Serializer.h>
#include <anatomist/processor/context.h>
#include <anatomist/surface/glcomponent.h>
#include <cartobase/object/syntax.h>
#include <cartobase/object/pythonwriter.h>
#include <graph/tree/tree.h>

using namespace anatomist;
using namespace carto;
using namespace std;


AskTexExtremaCommand::AskTexExtremaCommand( AObject* obj )
  : RegularCommand(), _obj( obj )
{
}


AskTexExtremaCommand::~AskTexExtremaCommand()
{
}


bool AskTexExtremaCommand::initSyntax()
{
  SyntaxSet	ss;
  Syntax	& s = ss[ "AskTexExtrema" ];

  s[ "object" ].type = "int";
  s[ "object" ].needed = true;

  Registry::instance()->add( "AskTexExtrema", &read, ss );
  return( true );
}


void AskTexExtremaCommand::doit()
{
  if( theAnatomist->hasObject( _obj ) )
    {
      GLComponent  *gl = _obj->glAPI();
      ostream       & out = cout;
      Object        ol = Object::value( ObjectVector() );
      ObjectVector    & l = ol->value<ObjectVector>();
      if( gl && gl->glNumTextures() > 0 )
      {
        unsigned    i, n = gl->glNumTextures(), j, m;
        for( i=0; i<n; ++i )
        {
          Object      opl = Object::value( ObjectVector() );
          ObjectVector  & pl = opl->value<ObjectVector>();
          const GLComponent::TexExtrema  & te = gl->glTexExtrema( i );
          for( j=0, m=te.minquant.size(); j<m; ++j )
            pl.push_back( Object::value( te.minquant[j] ) );
          for( j=0, m=te.maxquant.size(); j<m; ++j )
            pl.push_back( Object::value( te.maxquant[j] ) );
          l.push_back( opl );
        }
      }
      out << "AskTexExtrema: ";
      PythonWriter  pw;
      pw.attach( out );
      pw.write( *ol, false, false );
    }
}


Command* AskTexExtremaCommand::read( const Tree & com, 
				     CommandContext* context )
{
  int			id;
  void			*ptr;

  com.getProperty( "object", id );

  ptr = context->unserial->pointer( id, "AObject" );
  if( !ptr )
    {
      cerr << "object id " << id << " not found\n";
      return( 0 );
    }

  return( new AskTexExtremaCommand( (AObject *) ptr ) );
}


void AskTexExtremaCommand::write( Tree & com, Serializer* ser ) const
{
  Tree		*t = new Tree( true, name() );
  vector<int>	cols;

  t->setProperty( "object", ser->serialize( _obj ) );

  com.insert( t );
}
