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


#ifndef ANATOMIST_COMMANDS_CGRAPHPARAMS_H
#define ANATOMIST_COMMANDS_CGRAPHPARAMS_H


#include <anatomist/processor/Command.h>
#include <vector>


namespace anatomist
{

  class GraphParamsCommand : public RegularCommand
  {
  public:
    GraphParamsCommand( const std::string & dispmode, bool change_usenom, 
			bool usenom, const std::string & labelattrib, 
			bool ch_showttips, bool showttips, 
			bool ch_selcolorinv, bool selcolorinv, 
			const std::vector<int> & selcolor, 
			const std::string & savemode, bool ch_saveonlymod, 
			bool saveonlymod, bool ch_setbasedir, 
			bool setbasedir );
    virtual ~GraphParamsCommand();

    virtual std::string name() const { return( "GraphParams" ); }
    virtual void write( Tree & com, Serializer* ser ) const;

  protected:
    virtual void doit();

  private:
    std::string		_displaymode;
    bool		_ch_usenomenclature;
    bool		_usenomenclature;
    std::string		_labelattrib;
    bool		_ch_showttips;
    bool		_showttips;
    bool		_ch_selcolorinv;
    bool		_selcolorinv;
    std::vector<int>	_selcolor;
    std::string		_savemode;
    bool		_ch_saveonlymodif;
    bool		_saveonlymodif;
    bool		_ch_setbasedir;
    bool		_setbasedir;

    friend class StdModule;
    static Command* read( const Tree & com, CommandContext* context );
    static bool initSyntax();
  };

}

#endif

