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

#include <anatomist/processor/Writer.h>
#include <anatomist/processor/Command.h>
#include <anatomist/misc/error.h>
#include <anatomist/processor/Registry.h>
#include <graph/tree/tree.h>
#include <graph/tree/twriter.h>

#include <iostream>
#include <string.h>
#include <assert.h>


using namespace anatomist;
using namespace std;


//--- methods -----------------------------------------------------------------

CommandWriter::CommandWriter()
{
}


CommandWriter::CommandWriter(const string& filename)
{
	open(filename);
}


CommandWriter::~CommandWriter()
{
	theProcessor->deleteObserver(this);
}


void
CommandWriter::open(const string& filename)
{
	_stream.open(filename.c_str());
	if (!_stream)
	{
		AWarning("Unable to open history file.");
	}
	else
	{
		theProcessor->addObserver(this);
	}
}


int
CommandWriter::operator!() const
{
	return !_stream;
}


void
CommandWriter::update(const Observable*, void* arg)
{
	assert(arg != 0);

	Processor::Memo* memo = static_cast<Processor::Memo*>(arg);

	write(memo->type(), memo->command());
}


void
CommandWriter::write(Processor::Memo::Type type, Command* command)
{
  assert(type != Processor::Memo::EXECUTE || command != 0);
  Tree	ctype( true, "EXECUTE" );
  TreeWriter	tw( Registry::instance()->syntax() );

  tw.attach( _stream );

  try
    {
      switch (type)
        {
        case Processor::Memo::EXECUTE:
          command->write( ctype, this );
          break;
        case Processor::Memo::UNDO:
          ctype.setSyntax( "UNDO" );
          break;
        case Processor::Memo::REDO:
          ctype.setSyntax( "REDO" );
          break;
        }
      tw << ctype;
    }
  catch( exception & e )
    {
      cerr << "Exception in command/history writer: " << e.what() 
           << " / command type: " << command->name() << endl;
    }
}
