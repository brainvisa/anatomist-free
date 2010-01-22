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


#ifndef ANA_PROCESSOR_PROCESSOR_H
#define ANA_PROCESSOR_PROCESSOR_H


//--- header files ------------------------------------------------------------

#include <anatomist/config/anatomist_config.h>
#include <anatomist/observer/Observable.h>

#include <queue>
#include <stack>


namespace anatomist
{
  class Command;
  class CommandContext;


  //--- class declarations ----------------------------------------------------

  /**
   *	The command processor receives commands to manage and execute. \\
   *	Commands can be later undone and redone. From the moment a command
   *	is pushed to the processor, the processor becomes the owner of the
   *	command and manages its lifecycle. \\
   *	the Command Processor has no knowledge of the concrete subclasses of
   *	Command and accesses them through the standard Command interface.
   *	@see Command
   */
  class Processor : public Observable
  {
  public:
    Processor();
    virtual ~Processor();

    /**
     *	Give a command to execute
     *	@param c command to execute
     */
    void execute(Command* c);
    /** build and execute a command
        \param cname is the command type (syntactic attribute)
        \param params is a string formatted as a python dictionary
        \param cc IO and ID namespace for command execution
        \return the executed command
    */
    Command* execute( const std::string & cname, 
                      const std::string & params = "", 
                      CommandContext* cc = 0 );
    ///	Undo last done command
    bool undo();
    ///	Redo last undone command
    bool redo();
    void allowExecWhileIdle( bool );
    bool execWhileIdle() const;
    bool idle() const;

    /**@name	Helper structures*/
    //@{

    class Memo
    {
    public:
      enum Type { EXECUTE, UNDO, REDO };
      Memo(Type t, Command* c);
      Type type() const;
      Command* command() const;

    private:
      Type     _type;
      Command* _command;
    };

    //@}

  private:
    /**@name        Disable copy*/
    //@{

    /// Copy constructor
    Processor(const Processor&);

    /// Assignment operator
    Processor& operator=(const Processor&);

    //@}

    /**@name	Data*/
    //@{

    /// Store here incoming commands
    std::queue<Command*> _todo;

    /// Store here done commands
    std::stack<Command*> _done;

    /// Store here undone commands
    std::stack<Command*> _undone;

    /// Is the processor idle or is it already executing?
    bool _idle;
    bool _allowExecWhileIdle;

    //@}
  };


  //--- global variables ------------------------------------------------------

  ///	Pointer to the command processor of the application
  extern Processor* theProcessor;


  //--- inline methods --------------------------------------------------------

  inline
  Processor::Memo::Memo(Type t, Command* c)
    : _type(t), _command(c)
  {
  }


  inline
  Processor::Memo::Type
  Processor::Memo::type() const
  {
    return _type;
  }


  inline
  Command*
  Processor::Memo::command() const
  {
    return _command;
  }

}

#endif
