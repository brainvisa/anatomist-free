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


#ifndef ANA_PROCESSOR_READER_H
#define ANA_PROCESSOR_READER_H


//--- header files ------------------------------------------------------------

#include <anatomist/processor/context.h>
#include <anatomist/processor/pipeReaderP.h>
#include <cartobase/smart/mutexrcptr.h>
#include <fstream>
#include <string>

namespace anatomist
{
  class Unserializer;


  //--- class declarations ----------------------------------------------------

  /**
   *	CommandReader replays history files.
   */
  class CommandReader
  {
  public:
    /**
     *	Create a CommandReader to read commands from
     *	@param filename name of the file to read commands from
     */
    CommandReader( const std::string& filename, 
		   carto::MutexRcPtr<CommandContext> & context );
    ///	Creates a command reader not bound to a file descriptor
    CommandReader( carto::MutexRcPtr<CommandContext> & context );

    /**
     *	Create a CommandReader to read commands from
     *	@param filename name of the file to read commands from
     */
    virtual ~CommandReader();

    ///	Attach to an open stream
    void attach( std::istream & is );
    ///	Detach stream (destroys it if owned)
    void close();
    ///	Reader is asked to be closed (by the command just being read)
    bool askedToClose() const { return( _askedToClose ); }
    ///	Reader is asked to remove the named pipe file after closing
    bool askedToRemovePipeFile() const { return( _removePipe ); }

    /**@name	Command replay*/
    //@{

    /**
     *	Replay the logged commands
     */
    void read();

    ///	Replay next command
    void readOne();

    //@}

  private:
    ///	Stream to read from
    std::istream			*_history;
    bool				_ownStream;
    carto::MutexRcPtr<CommandContext>	_context;
    bool				_askedToClose;
    bool				_removePipe;
  };

}

#endif
