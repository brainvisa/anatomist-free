/* Copyright (c) 1995-2006 CEA
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


#ifndef ANA_COMMANDS_COBJECTINFO_H
#define ANA_COMMANDS_COBJECTINFO_H


//--- header files ------------------------------------------------------------

#include <anatomist/processor/Command.h>
#include <cartobase/object/object.h>
#include <vector>
#include <set>

namespace anatomist
{

  class AWindow;
  class AObject;


  //--- class declarations ---------------------------------------------------

  /**	Writes information about Anatomist state in a file or pipe
   */
  class ObjectInfoCommand : public RegularCommand, public SerializingCommand
  {
  public:
    ObjectInfoCommand( const std::string & filename, CommandContext* context, 
		       const std::vector<int> & objectIDs, bool nameChildren,
		       bool nameref = false, 
                       const std::string & requestid = "",
                       const std::set<std::string> & objfilenames
                           = std::set<std::string>() );
    virtual ~ObjectInfoCommand();

    virtual std::string name() const { return( "ObjectInfo" ); }
    virtual void write( Tree & com, Serializer* ser ) const;
    carto::Object result();

  protected:
    virtual void doit();

  private:
    std::string		   _filename;
    std::vector<int>	   _objects;
    bool		   _nameChildren;
    bool		   _nameref;
    std::string		   _requestid;
    carto::Object	   _result;
    std::set<std::string>  _objfilenames;

    friend class StdModule;
    static Command* read( const Tree & com, CommandContext* context );
    static bool initSyntax();
  };

}


#endif



