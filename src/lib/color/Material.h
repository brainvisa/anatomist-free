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


#ifndef ANA_COLOR_MATERIAL_H
#define ANA_COLOR_MATERIAL_H

#include <anatomist/window/glcaps.h>
#include <cartobase/object/object.h>


namespace anatomist
{

  /** This class has to be rewritten, it's really really a shame.......
   */
  class Material
  {
  public:
    /// Rendering properties flags
    enum RenderProperty
      {
        /// lighting effect according to vertices normals
        RenderLighting, 
        /// polygons color interpolation
        RenderSmoothShading, 
        /// line/polygon filtering
        RenderFiltering, 
        /// print in Z-buffer
        RenderZBuffer, 
        /// filter back side polygons
        RenderFaceCulling, 
        /// wireframe rendering
        RenderMode,
        /// ghost mode (invisible to clicks)
        Ghost,
      };

  enum RenderingMode
    {
      Normal, 
      Wireframe, 
      Outlined, 
      HiddenWireframe, 
      Fast,
      ExtOutlined,
    };

    Material();
    Material(const Material &);

    virtual ~Material();

    GLfloat *Ambient() { return(_ambient); }
    GLfloat *Diffuse() { return(_diffuse); }
    GLfloat *Specular() { return(_specular); }
    GLfloat Shininess() const { return(_shininess); }
    GLfloat *Emission() { return(_emission); }

    GLfloat Ambient(int i) const { return(_ambient[i]); }
    GLfloat Diffuse(int i) const { return(_diffuse[i]); }
    GLfloat Specular(int i) const { return(_specular[i]); }
    GLfloat Emission(int i) const { return(_emission[i]); }

    void SetAmbient(float, float, float, float);
    void SetDiffuse(float, float, float, float);
    void SetSpecular(float, float, float, float);
    void SetShininess(float val);
    void SetEmission(float, float, float, float);

    void SetAmbientR(float val);
    void SetAmbientG(float val);
    void SetAmbientB(float val);
    void SetAmbientA(float val);
    void SetDiffuseR(float val);
    void SetDiffuseG(float val);
    void SetDiffuseB(float val);
    void SetDiffuseA(float val);
    void SetSpecularR(float val);
    void SetSpecularG(float val);
    void SetSpecularB(float val);
    void SetSpecularA(float val);
    void SetEmissionR(float val);
    void SetEmissionG(float val);
    void SetEmissionB(float val);
    void SetEmissionA(float val);
    /// color used without lighting (filar meshes, wireframe)
    GLfloat *unlitColor() const;
    GLfloat unlitColor( int i ) const;
    void setUnlitColor( float, float, float, float );
    float lineWidth() const;
    void setLineWidth( float w );

    /** setup OpenGL properties.
        This is an almost private function that should only be called by 
        GLComponent internals. It implies a glPushAttrib() so popGLState() 
        should be called after the object rendering
    */
    void setGLMaterial() const;
    /** Pops previous OpenGL state.
        This is an almost private function that should only be called by 
        GLComponent internals */
    void popGLState() const;

    bool IsBlended() const;
    /// a property set to -1 is neutral (use window or app default)
    int renderProperty( RenderProperty ) const;
    void setRenderProperty( RenderProperty, int );

    Material &operator = (const Material &);
    friend std::istream &operator >> (std::istream &, anatomist::Material &);
    friend std::ostream &operator << (std::ostream &, 
				      const anatomist::Material &);
    bool operator != ( const Material & ) const;
    bool operator == ( const Material & mat ) const
      { return( !operator != ( mat ) ); }

    void set( const carto::GenericObject & );
    carto::Object genericDescription() const;

  protected:
    GLfloat	_ambient[4];
    GLfloat	_diffuse[4];
    GLfloat	_specular[4];
    GLfloat	_shininess;
    GLfloat	_emission[4];

  private:
    struct Private;
    Private	*d;
  };

  std::istream &operator >> (std::istream &, anatomist::Material &);
  std::ostream &operator << (std::ostream &, const anatomist::Material &);

}

#endif
