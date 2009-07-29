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


#ifndef ANA_COMMANDS_CSETOBJECTPALETTE_H
#define ANA_COMMANDS_CSETOBJECTPALETTE_H

#include <anatomist/processor/Command.h>
#include <set>


namespace anatomist
{

  class AObject;


  /**	Set an AObjectPalette to objects
   */
  class SetObjectPaletteCommand : public RegularCommand
  {
  public:
    SetObjectPaletteCommand( const std::set<AObject *> &, 
			     const std::string & palname1,
			     bool min1flg = false, float min1 = 0, 
			     bool max1flg = false, float max1 = 1, 
			     const std::string & palname2 = "", 
			     bool min2flg = false, 
			     float min2 = 0, bool max2flg = false, 
			     float max2 = 1, 
			     const std::string & mixMethod = "", 
			     bool mixFacflg = false, 
			     float linMixFactor = 0.5, 
			     const std::string & pal1Dmapping = "",
                             bool absmode = false );
    virtual ~SetObjectPaletteCommand();

    virtual std::string name() const { return( "SetObjectPalette" ); }
    virtual void write( Tree & com, Serializer* ser ) const;

  protected:
    virtual void doit();

  private:
    std::set<AObject *>	_objL;
    std::string		_pal1;
    std::string		_pal2;
    std::string		_pal1Dmapping ;
    float		_min1;
    float		_max1;
    float		_min2;
    float		_max2;
    std::string		_mixMethod;
    float		_linMixFactor;
    bool		_min1flg;
    bool		_max1flg;
    bool		_min2flg;
    bool		_max2flg;
    bool		_mixFacFlg;
    bool                _absmode;

    friend class StdModule;
    static Command* read( const Tree & com, CommandContext* context );
    static bool initSyntax();
  };

}


#endif
