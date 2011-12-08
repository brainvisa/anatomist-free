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


#ifndef ANA_COLOR_LIGHT_H
#define ANA_COLOR_LIGHT_H

#include <anatomist/window/glcaps.h>
#include <cartobase/object/object.h>


namespace anatomist
{

  /**    Light used for OpenGL in 3D windows
   */
  class Light
  {
  public:
    Light();
    virtual ~Light();

    GLfloat *Ambient() { return(_ambient); }
    GLfloat Ambient(int i) { return(_ambient[i]); }

    GLfloat *Diffuse() { return(_diffuse); }
    GLfloat Diffuse(int i) { return(_diffuse[i]); }

    GLfloat *Specular() { return(_specular); }
    GLfloat Specular(int i) { return(_specular[i]); }

    GLfloat *Background() { return(_background); }
    GLfloat Background(int i) { return(_background[i]); }

    GLfloat *ModelAmbient() { return(_modelAmbient); }
    GLfloat ModelAmbient(int i) { return(_modelAmbient[i]); }

    GLfloat *Position() { return(_position); }
    GLfloat Position(int i) { return(_position[i]); }

    GLfloat *SpotDirection() { return(_spotDirection); }
    GLfloat SpotDirection(int i) { return(_spotDirection[i]); }

    GLfloat SpotExponent() { return(_spotExponent); }
    GLfloat SpotCutoff() { return(_spotCutoff); }
    GLfloat ConstantAttenuation() { return(_constantAttenuation); }
    GLfloat LinearAttenuation() { return(_linearAttenuation); }
    GLfloat QuadraticAttenuation() { return(_quadraticAttenuation); }
    GLfloat ModelLocalViewer() { return(_modelLocalViewer); }
    GLfloat ModelTwoSide() { return(_modelTwoSide); }

    /// 		Set the light spot attenuation exponent.
    void SetSpotExponent(float val);
    /// 		Set the light spot cutoff angle.
    void SetSpotCutoff(float val);
    /// 		Set the light spot constant attenuation.
    void SetConstantAttenuation(float val);
    /// 		Set the light spot linear attenuation.
    void SetLinearAttenuation(float val);
    /// 		Set the light spot quadratic attenuation.
    void SetQuadraticAttenuation(float val);
    /// 		Set the light local viewer model.
    void SetModelLocalViewer(float val);
    /// 		Set the light two side model.
    void SetModelTwoSide(float val);

    /// 		Set the four position coordinates of the light.
    void SetPosition(float, float, float, float);
    /// 		Set the X position of the spot light.
    void SetPositionX(float val);
    /// 		Set the Y position of the spot light.
    void SetPositionY(float val);
    /// 		Set the Z position of the spot light.
    void SetPositionZ(float val);
    /// 		Set the W position of the spot light.
    void SetPositionW(float val);

    /// 		Set the three spot direction components of the light.
    void SetSpotDirection(float, float, float);
    /// 		Set the X spot light direction.
    void SetSpotDirectionX(float val);
    /// 		Set the Y spot ligth direction.
    void SetSpotDirectionY(float val);
    /// 		Set the Z spot light direction.
    void SetSpotDirectionZ(float val);

    /// 		Set the four ambient model components of the light.
    void SetModelAmbient(float, float, float, float);
    /// 		Set the light ambient model red component.
    void SetModelAmbientR(float val);
    /// 		Set the light ambient model green component.
    void SetModelAmbientG(float val);
    /// 		Set the light ambient model blue component.
    void SetModelAmbientB(float val);
    /// 		Set the light ambient model alpha coefficient.
    void SetModelAmbientA(float val);

    /// 		Set the four ambient components of the light.
    void SetAmbient(float, float, float, float);
    /// 		Set the ambient red component of the light.
    void SetAmbientR(float val);
    /// 		Set the ambient green component of the light.
    void SetAmbientG(float val);
    /// 		Set the ambient blue component of the light.
    void SetAmbientB(float val);
    /// 		Set the ambient alpha coefficient of the light.
    void SetAmbientA(float val);

    /// 		Set the four diffuse components of the light .
    void SetDiffuse(float, float, float, float);
    /// 		Set the diffuse red component of the light.
    void SetDiffuseR(float val);
    /// 		Set the diffuse green component of the light.
    void SetDiffuseG(float val);
    /// 		Set the diffuse blue component of the light.
    void SetDiffuseB(float val);
    /// 		Set the diffuse alpha coefficient of the light.
    void SetDiffuseA(float val);

    /// 		Set the four specular components of the light.
    void SetSpecular(float, float, float, float);
    /// 		Set the specular red component of the light.
    void SetSpecularR(float val);
    /// 		Set the specular green component of the light.
    void SetSpecularG(float val);
    /// 		Set the specular blue component of the light.
    void SetSpecularB(float val);
    /// 		Set the specular alpha coefficient of the light.
    void SetSpecularA(float val);

    /// 		Set the four background components of the light.
    void SetBackground(float, float, float, float);
    /// 		Set the specular red component of the light.
    void SetBackgroundR(float val);
    /// 		Set the specular green component of the light.
    void SetBackgroundG(float val);
    /// 		Set the specular blue component of the light.
    void SetBackgroundB(float val);
    /// 		Set the specular alpha coefficient of the light.
    void SetBackgroundA(float val);

    GLuint getGLList();
    void refreshGLList();
    void setChanged( bool=true );

    void set( const carto::GenericObject & );
    void set( const carto::Object & );
    carto::Object genericDescription() const;

  protected:
    GLfloat _ambient[4];
    GLfloat _diffuse[4];
    GLfloat _specular[4];
    GLfloat _background[4];
    GLfloat _position[4];
    GLfloat _spotDirection[3];
    GLfloat _spotExponent;
    GLfloat _spotCutoff;
    GLfloat _constantAttenuation;
    GLfloat _linearAttenuation;
    GLfloat _quadraticAttenuation;
    GLfloat _modelAmbient[4];
    GLfloat _modelLocalViewer;
    GLfloat _modelTwoSide;
    GLuint  _GLList;
    bool    _GLLRefreshFlag;
  };

}


#endif
