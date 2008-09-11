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


//--- header files ------------------------------------------------------------

#include <anatomist/processor/Registry.h>
#include <anatomist/processor/context.h>
#include <graph/tree/tree.h>

using namespace anatomist;
using namespace carto;
using namespace std;

//--- global variables --------------------------------------------------------

// moved to application/statics.cc
//Registry* Registry::_instance = 0;


//--- methods -----------------------------------------------------------------

Registry::Registry()
{
  _syntax[ "EXECUTE" ];
  _syntax[ "UNDO" ];
  _syntax[ "REDO" ];
}


Registry*
Registry::instance()
{
  //cout << "Registry::instance : " << _instance << endl;
  if (_instance == 0)
    {
      _instance = new Registry;
    }
  return _instance;
}


bool
Registry::add( const string & command, Command::Reader function, 
	       const SyntaxSet & syntax )
{
  _command[ command ] = function;
  SyntaxSet::const_iterator	is, es = syntax.end();
  for( is=syntax.begin(); is!=es; ++is )
    _syntax.insert( *is );
  return( true );
}


Command* Registry::create( const Tree & comm, CommandContext* context ) const
{
  map<string, Command::Reader>::const_iterator
    ic = _command.find( comm.getSyntax() );
  if( ic == _command.end() )
    return( 0 );
  return( (*ic).second( comm, context ) );
}
