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


//--- header files ------------------------------------------------------------

#include <anatomist/processor/Command.h>
#include <anatomist/processor/Registry.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/processor/context.h>
#include <graph/tree/tree.h>
#include <stdlib.h>


using namespace anatomist;
using namespace std;


//--- methods -----------------------------------------------------------------

Command::Command()
{
}


Command::~Command()
{
}


void
Command::execute()
{
	doit();
}


void
Command::undo()
{
	undoit();
}


void
Command::redo()
{
	doit();
}


void
Command::undoit()
{
	cerr << "NOT IMPLEMENTED (" << __FILE__ << ":" << __LINE__ << ": "
	     << "undoit())" << endl;
	abort();
}


void
Command::write( Tree & com, Serializer * ) const
{
  Tree	*tr = new Tree( true, name() );
  com.insert( tr );
}


//--- methods -----------------------------------------------------------------

RegularCommand::RegularCommand() : Command()
{
}


RegularCommand::~RegularCommand()
{
}


//--- methods -----------------------------------------------------------------

SerializingCommand::SerializingCommand( CommandContext *context )
  : _context ( context ? context : &CommandContext::defaultContext() )
{
}


SerializingCommand::~SerializingCommand()
{
}


//--- methods -----------------------------------------------------------------

WaitCommand::WaitCommand() : Command()
{
}


WaitCommand::~WaitCommand()
{
}


void
WaitCommand::execute()
{
  theAnatomist->setCursor( Anatomist::Working );
  try
    {
      doit();
    }
  catch( ... )
    {
      theAnatomist->setCursor( Anatomist::Normal );
      throw;
    }
  theAnatomist->setCursor( Anatomist::Normal );
}


void
WaitCommand::undo()
{
	theAnatomist->setCursor( Anatomist::Working );
	undoit();
	theAnatomist->setCursor( Anatomist::Normal );
}


void
WaitCommand::redo()
{
	theAnatomist->setCursor( Anatomist::Working );
	doit();
	theAnatomist->setCursor( Anatomist::Normal );
}
