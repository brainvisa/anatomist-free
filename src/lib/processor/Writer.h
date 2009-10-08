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


#ifndef ANA_PROCESSOR_WRITER_H
#define ANA_PROCESSOR_WRITER_H


//--- header files ------------------------------------------------------------

#include <anatomist/processor/Serializer.h>
#include <anatomist/processor/Processor.h>
#include <anatomist/observer/Observer.h>
#include <fstream>
#include <string>


namespace anatomist
{
  //--- forward declarations --------------------------------------------------

  class Command;


  //--- class declarations ----------------------------------------------------

  /**
   *	CommandWriter is a helper class logging commands into a history file
   */

  class CommandWriter : public Observer, public Serializer
  {
  public:
    /**@name	Constructors and Destructor*/
    //@{

    /**
     *	Create a CommandWriter
     */
    CommandWriter();

    /**
     *	Create a CommandWriter to log commands
     *	@param filename name of the file to log commands into
     */
    CommandWriter(const std::string& filename);

    /// does nothing
    virtual ~CommandWriter();

    //@}

    /**@name	File*/
    //@{

    /**
     *	Attach to a file
     *	@param filename name of the file to log commands into
     */
    void open(const std::string& filename);

    /**
     *	Create a CommandWriter to log commands
     *	@param filename name of the file to log commands into
     */
    int operator!() const;

    //@}

    /**@name	Command logging*/
    //@{

    /**
     *	Log a command into the stream associated with the CommandWriter
     *	@param observable observed Processor
     *	@param arg cast back to a Processor::Memo to retrieve the
     *	command to log
     */
    virtual void update(const Observable* observable, void* arg);

    //@}

  private:
    /**@name	Command logging*/
    //@{

    /**
     *	Log a command into the stream associated with the CommandWriter
     *	@param type EXECUTE, UNDO or REDO
     *	@param command the one to log
     */
    void write(Processor::Memo::Type type, Command* command);

    //@}

    /**@name        Disable copy*/
    //@{

    ///	Copy constructor
    CommandWriter(const CommandWriter&);

    ///	Assignment operator
    CommandWriter& operator=(const CommandWriter&);

    //@}

    /**@name	Data*/
    //@{

    //	Stream to log into
    std::ofstream _stream;

    //@}
  };

}


#endif
