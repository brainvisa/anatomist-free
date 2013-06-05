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


#ifndef ANATOMIST_COMMANDS_CSETMATERIAL_H
#define ANATOMIST_COMMANDS_CSETMATERIAL_H


#include <anatomist/processor/Command.h>
#include <set>
#include <vector>


namespace anatomist
{

  class AObject;


  ///	Apply a material colors to objects
  class SetMaterialCommand : public RegularCommand
  {
  public:
    SetMaterialCommand( const std::set<AObject *> & obj, float* ambient, 
			float* diffuse, float* emission, float* specular, 
			float shininess = -1, bool refresh = true, 
                        int lighting = -2, int smoothshading = -2, 
                        int polyfiltering = -2, int zbuffer = -2, 
                        int faceculling = -2, 
                        const std::string & polymode = "",
                        int frontface = -1, float linewidth=-1,
                        const std::vector<float> &
                        unlitcolor=std::vector<float>(), int ghost=-1 );
    virtual ~SetMaterialCommand();

    virtual std::string name() const { return( "SetMaterial" ); }
    virtual void write( Tree & com, Serializer* ser ) const;

  protected:
    virtual void doit();

  private:
    std::set<AObject *>	_obj;
    float		_ambient[4];
    float		_diffuse[4];
    float		_emission[4];
    float		_specular[4];
    float		_shininess;
    bool		_refresh;
    int			_lighting;
    int			_smoothshading;
    int			_polygonfiltering;
    int			_zbuffer;
    int			_faceculling;
    std::string		_polygonmode;
    int                 _frontface;
    float               _linewidth;
    std::vector<float>  _unlitcolor;
    int                 _ghost;

    friend class StdModule;
    static Command* read( const Tree & com, CommandContext* context );
    static bool initSyntax();
  };

}


#endif
