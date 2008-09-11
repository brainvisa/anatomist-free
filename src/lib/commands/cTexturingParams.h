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


#ifndef ANATOMIST_COMMANDS_CTEXTURINGPARAMS_H
#define ANATOMIST_COMMANDS_CTEXTURINGPARAMS_H


#include <anatomist/processor/Command.h>
#include <set>
#include <vector>


namespace anatomist
{
  class AObject;

  class TexturingParamsCommand : public RegularCommand
  {
  public:
    TexturingParamsCommand( const std::set<AObject *> & obj, unsigned tex = 0, 
                            int mode = -1, int filter = -1, int gen = -1, 
                            float rate = -1, int rgbint = -1, 
                            const float* genparams1 = 0, 
                            const float* genparams2 = 0, 
                            const float* genparams3 = 0);
    virtual ~TexturingParamsCommand();

    virtual std::string name() const { return( "TexturingParams" ); }
    virtual void write( Tree & com, Serializer* ser ) const;

  protected:
    virtual void doit();

  private:
    std::set<AObject *>	_objects;
    unsigned		_tex;
    int			_mode;
    int			_filter;
    int			_gen;
    float		_rate;
    int			_rgbinter;
    std::vector<float>	_genparams_1;
    std::vector<float>	_genparams_2;
    std::vector<float>	_genparams_3;

    friend class StdModule;
    static Command* read( const Tree & com, CommandContext* context );
    static bool initSyntax();
  };

}


#endif

