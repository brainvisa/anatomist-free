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


#ifndef ANA_PROCESSOR_REGISTRY_H
#define ANA_PROCESSOR_REGISTRY_H


//--- header files ------------------------------------------------------------

#include <anatomist/processor/Command.h>
#include <cartobase/object/syntax.h>


class Tree;


namespace anatomist
{
  struct CommandContext;

  //--- class declarations ----------------------------------------------------

  /**
   *	Registry links all Command concrete subclasses with a function
   *	that will read the guts of the command and create an instance of the
   *	subclass.
   *
   *	Because there has to be only one registry, Registry implements
   *	the Singleton pattern:
   *	- Erich Gamma, Richard Helm, Ralph Johnson, John M. Vlissides,
   *	\URL[Design patterns]{http://www.awl.com/cseng/titles/0-201-63361-2/},
   *	pp. 127-134.
   *	Addison Wesley, 1995.
   */

  class Registry
  {
  public:
    /**
     *	Access the unique instance of Registry
     *	@return always the same unique instance of Registry
     */
    static Registry* instance();
    
    ~Registry();

    const carto::SyntaxSet & syntax() const { return( _syntax ); }

    /**
     *	Register a \Ref{Command} subclass and link with a function
     *	will read the guts from a stream and create an instance of it
     *	@param command name under which the class will be registered
     */
    bool add( const std::string & command, Command::Reader function, 
	      const carto::SyntaxSet & syntax );

    ///	Creates a command with given characteristics provided as a Tree
    Command* create( const Tree & command, CommandContext* context ) const;

  protected:
    ///	Singleton constructor cannot be accessed by the programmer
    Registry();

  private:
    /**@name        Disable copy*/
    //@{

    /// Copy constructor
    Registry(const Registry&);

    /// Assignment operator
    Registry& operator=(const Registry&);

    //@}

    ///	The unique instance of this class
    static Registry* 				_instance;

    std::map<std::string, Command::Reader>	_command;
    carto::SyntaxSet				_syntax;
  };

}


#endif
