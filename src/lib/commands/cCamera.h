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


#ifndef ANATOMIST_COMMANDS_CCAMERA_H
#define ANATOMIST_COMMANDS_CCAMERA_H

#include <anatomist/processor/Command.h>
#include <aims/resampling/quaternion.h>
#include <aims/vector/vector.h>
#include <set>

namespace anatomist
{
  class AWindow;

  class CameraCommand : public RegularCommand
  {
  public:
    CameraCommand( const std::set<AWindow *> & win, 
		   const Point3df *observerpos = 0, 
		   const aims::Quaternion* viewq = 0, float zoom = 0,
		   const aims::Quaternion* sliceq = 0, bool forcedraw = false, 
		   const Point4df *curpos = 0, const Point3df* bbmin = 0, 
                   const Point3df* bbmax = 0 );
    virtual ~CameraCommand();

    virtual std::string name() const { return( "Camera" ); }
    virtual void write( Tree & com, Serializer* ser ) const;

  protected:
    virtual void doit();

  private:
    std::set<AWindow *>	_win;
    bool		_obspos;
    Point3df		_observerpos;
    bool		_curpos;
    Point4df		_cursorpos;
    bool		_viewq;
    aims::Quaternion	_viewquat;
    bool		_sliceq;
    aims::Quaternion	_slicequat;
    float		_zoom;
    bool		_forcedraw;
    Point3df		_bbmin;
    bool		_bbminset;
    Point3df		_bbmax;
    bool		_bbmaxset;

    friend class StdModule;
    static Command* read( const Tree & com, CommandContext* context );
    static bool initSyntax();
  };

}

#endif

