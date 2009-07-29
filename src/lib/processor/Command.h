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


#ifndef ANA_PROCESSOR_COMMAND_H
#define ANA_PROCESSOR_COMMAND_H


//--- header files ------------------------------------------------------------

#include <map>
#include <string>

class Tree;


//--- class declarations ------------------------------------------------------


namespace anatomist
{
  class Serializer;
  class Unserializer;
  class CommandReader;
  struct CommandContext;

  /**	The abstract base class for commands
   */
  class Command
  {
  public:
    ///	Destructor does nothing
    virtual ~Command();

    ///	Execute the command - may use {\tt doit()}
    virtual void execute();
    ///	Undo the command - may use {\tt undoit()}
    virtual void undo();
    ///	Redo the command - may use {\tt doit()}
    virtual void redo();

    typedef Command* (*Reader)( const Tree &, CommandContext * );

    /// Return the unique name of a class of commands
    virtual std::string name() const = 0;

    /// Print the guts of a command into a Tree given as parent
    virtual void write( Tree & com, Serializer *ser ) const;

  protected:
    /** The programmer cannot call the constructor of an
	abstract base class
    */
    Command();

    /// Do a command
    virtual void doit() = 0;
    /// Undo a command
    virtual void undoit();

  private:
    /// Copy constructor
    Command(const Command&);
    /// Assignment operator
    Command& operator=(const Command&);
  };


  //--- class declarations ----------------------------------------------------

  /**	The abstract base class for usual commands
   */
  class RegularCommand : public Command
  {
  public:
    /// Destructor does nothing
    virtual ~RegularCommand();

  protected:
    /**     The programmer cannot call the constructor of an
	    abstract base class
    */
    RegularCommand();
  };


  //--- class declarations ----------------------------------------------------

  /**	The abstract base class used for commands that have to read/write 
	pointers. \\
	Commands that have to create pointers and write identifiers for them 
	or match them with a given (read) identifier, must inherit from 
	SerializingCommand and also from its base command type 
	(RegularCommand, MppCommand...). SerializingCommand is not a real 
	command in itself and cannot be used alone.
  */
  class SerializingCommand
  {
  public:
    virtual ~SerializingCommand();
    CommandContext *context() { return _context; }
    const CommandContext *context() const { return _context; }

  protected:
    SerializingCommand( CommandContext * );

    CommandContext *_context;
  };


  //--- class declarations ----------------------------------------------------

  /**	The abstract base class for commands that display a watch cursor
   */
  class WaitCommand : public Command
  {
  public:
    /// Destructor does nothing
    virtual ~WaitCommand();

    /// Execute the command - may use {\tt doit()}
    virtual void execute();
    /// Undo the command - may use {\tt undoit()}
    virtual void undo();
    /// Redo the command - may use {\tt doit()}
    virtual void redo();

  protected:
    /**     The programmer cannot call the constructor of an
	    abstract base class
    */
    WaitCommand();
  };

}


#endif
