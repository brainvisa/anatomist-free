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


#ifndef ANATOMIST_COMMANDS_CSLICEPARAMS_H
#define ANATOMIST_COMMANDS_CSLICEPARAMS_H


#include <anatomist/processor/Command.h>
#include <aims/resampling/quaternion.h>
#include <set>


namespace anatomist
{
  class AObject;

  class SliceParamsCommand : public RegularCommand
  {
  public:
    SliceParamsCommand( const std::set<AObject *> & obj, 
                        const Point3df *pos = 0,
                        const aims::Quaternion *quat = 0,
                        const Point4df *plane = 0 );
    virtual ~SliceParamsCommand();

    virtual std::string name() const { return( "SliceParams" ); }
    virtual void write( Tree & com, Serializer* ser ) const;

  protected:
    virtual void doit();

  private:
    std::set<AObject *>	_obj;
    bool		_setpos;
    Point3df		_position;
    bool		_setquat;
    aims::Quaternion	_quaternion;
    bool		_setplane;
    Point4df		_plane;

    friend class StdModule;
    static Command* read( const Tree & com, CommandContext* context );
    static bool initSyntax();
  };

}


#endif

