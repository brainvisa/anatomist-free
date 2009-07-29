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


#ifndef ANA_COMMANDS_CGETINFO_H
#define ANA_COMMANDS_CGETINFO_H


//--- header files ------------------------------------------------------------

#include <anatomist/processor/Command.h>
#include <anatomist/processor/context.h>
#include <cartobase/object/object.h>

namespace anatomist
{

  class AWindow;
  class AObject;
  class Referential;


  //--- class declarations ---------------------------------------------------

  /**	Writes information about Anatomist state in a file or pipe
   */
  class GetInfoCommand : public RegularCommand, public SerializingCommand
  {
  public:
    GetInfoCommand( const std::string & filename, CommandContext* context 
		    = &CommandContext::defaultContext(), 
		    bool objects = false, bool windows = false, 
		    bool refs = false, bool trans = false, 
		    bool palettes = false, const std::string & nameobj = "", 
		    bool namewin = false, bool selects = false, 
		    bool link = false, Referential* linkref = 0, 
                    bool nameref = false, bool nametrans = false,
                    const std::string & requestid = "", bool version = false,
                    bool listcommands = false, bool aimsinfo = false,
                    bool modinfo = false
                  );
    virtual ~GetInfoCommand();

    virtual std::string name() const { return( "GetInfo" ); }
    virtual void write( Tree & com, Serializer* ser ) const;
    carto::Object result();

  protected:
    virtual void doit();

  private:
    std::string		_filename;
    bool		_objects;
    bool		_windows;
    bool		_refs;
    bool		_trans;
    bool		_palettes;
    std::string		_nameobj;
    bool		_namewin;
    bool		_nameref;
    bool		_nametrans;
    bool		_selects;
    bool		_link;
    Referential		*_linkref;
    std::string		_requestid;
    carto::Object	_result;
    bool                _version;
    bool                _listcommands;
    bool                _aimsinfo;
    bool                _modinfo;

    friend class StdModule;
    static Command* read( const Tree & com, CommandContext* context );
    static bool initSyntax();
  };

}


#endif


