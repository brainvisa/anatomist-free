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

#include <anatomist/commands/cClosePipe.h>
#include <anatomist/processor/Registry.h>
#include <cartobase/object/syntax.h>
#include <graph/tree/tree.h>

using namespace anatomist;
using namespace carto;
using namespace std;


ClosePipeCommand::ClosePipeCommand( bool removePipe )
  : RegularCommand(), _removeFile( removePipe )
{
}


ClosePipeCommand::~ClosePipeCommand()
{
}


bool ClosePipeCommand::initSyntax()
{
  SyntaxSet	ss;
  Syntax	& s = ss[ "ClosePipe" ];
  s[ "remove_file" ].type = "int";
  s[ "remove_file" ].needed = false;
  Registry::instance()->add( "ClosePipe", &read, ss );
  return( true );
}


void ClosePipeCommand::doit()
{
  //cout << "ClosePipeCommand::doit\n";
}


Command* ClosePipeCommand::read( const Tree & /*com*/, CommandContext* )
{
  //int	rmv = 0;
  //com.getProperty( "remove_file", rmv );
  return( new ClosePipeCommand /*( rmv )*/ );
}


void ClosePipeCommand::write( Tree & com, Serializer* /*ser*/ ) const
{
  Tree	*t = new Tree( true, name() );

  if( _removeFile )
    t->setProperty( "remove_file", (int) 1 );
  com.insert( t );
  //cout << "ClosePipeCommand::write done\n";
}
