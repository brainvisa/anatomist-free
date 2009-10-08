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


#ifndef ANA_COMMANDS_CLOADOBJECT_H
#define ANA_COMMANDS_CLOADOBJECT_H


//--- header files ------------------------------------------------------------

#include <anatomist/processor/Command.h>
#include <anatomist/processor/context.h>
#include <cartobase/object/object.h>


namespace anatomist
{

  class AObject;


  //--- class declarations ----------------------------------------------------

  /**	Command to load an object from a file
   */

  class LoadObjectCommand : public WaitCommand, 
			    public SerializingCommand
  {
  public:
    LoadObjectCommand( const std::string & filename, int id = -1, 
		       const std::string & objname = "", bool ascursor = false,
		       carto::Object options = carto::none(), 
                       CommandContext* context 
		       = &CommandContext::defaultContext() );
    virtual ~LoadObjectCommand();

    virtual std::string name() const { return( "LoadObject" ); }
    virtual void write( Tree & com, Serializer *ser ) const;
    AObject * loadedObject() { return _obj ; }
    
  protected:
    virtual void doit();

  private:
    friend class StdModule;
    static Command* read( const Tree & com, CommandContext* context );
    static bool initSyntax();

    std::string			_filename;
    int				_id;
    AObject			*_obj;
    std::string			_objectname;
    bool			_ascursor;
    carto::Object		_options;
  };

}


#endif
