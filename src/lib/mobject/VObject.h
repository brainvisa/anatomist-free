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

#ifndef ANA_MOBJECT_VOBJECT_H
#define ANA_MOBJECT_VOBJECT_H

#include <anatomist/mobject/objectVector.h>
#include <anatomist/surface/glcomponent.h>

class QOpenGLShaderProgram;

namespace anatomist
{

  /** GPU ray-marching volume renderer.

      VObject displays a scalar volume (AVolume<T>) using direct volume
      rendering: a proxy cube matching the volume bounding box is rasterized,
      and for each covered pixel the fragment shader marches a ray through
      the volume, accumulating color and opacity front-to-back.

      Rendering overview:
      - Geometry: a proxy cube (bounding box of the volume), see
        glMakeBodyGLL(). Its only purpose is to generate fragments.
      - Shaders: dedicated templates (volume.vs.glsl / volume.fs.glsl),
        assembled by the dynamic shader builder. The "V" shader module ID
        keeps VObject shaders separate from regular mesh shaders.
      - Ray setup: the camera uses an orthographic projection, so all rays
        share the same direction. The entry point in the volume is computed
        analytically with a ray/AABB (slab) intersection.
      - Shading: per-sample Blinn-Phong lighting, using the density gradient
        as surface normal.
      - Depth: the depth of the first sample reaching a given accumulated
        opacity is written to gl_FragDepth, so that the volume integrates
        correctly with other objects (opaque meshes, depth peeling).
      - Clipping: scene clip planes and object clip planes (ClippedObject)
        are applied per sample inside the ray-marching loop.

      Textures (managed by the GLComponent texture system, and exposed to the
      shader by GLObjectUniforms):
      - tex 0: volume data, 3D, GL_R32F (raw values)      -> u_texture3D[0]
      - tex 1: density gradient, 3D, GL_RGBA16F / RGB16F  -> u_texture3D[1]
      - tex 2: transfer function, 1D, GL_RGBA (palette)   -> u_texture1D[0]

      The source volume is inserted as a child object: VObject inherits its
      referential and keeps it alive while in use.
  */
  class VObject : public ObjectVector, public GLComponent
  {
   
  public:
    struct Private;

    /** Builds a volume renderer for the given volume object.
        \param vol source volume. Must be an AVolume<T> with a scalar voxel
        type T among int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t,
        float and double. Other objects (RGB/RGBA volumes, 2D fusions, other
        sliceable objects) are accepted but cannot be rendered: the volume
        texture fails to build and nothing is displayed.
        The volume becomes a child of the VObject.
    */
    VObject( AObject * vol );
    virtual ~VObject();

    /** Renders the object using the standard AObject path (GL lists from
        glMainGLL()), bypassing MObject::render() which would render the
        child volume itself.
    */
    virtual bool render( PrimList &, RenderContext & );
    virtual bool Is2DObject() { return true; }
    virtual bool Is3DObject() { return false; }
    /// The source volume cannot be removed while the VObject exists
    virtual bool CanRemove( AObject * );
    virtual Tree* optionTree() const;

    /// VObject provides its own GLComponent
    virtual const GLComponent* glAPI() const override;
    virtual GLComponent* glAPI() override;

    // --- Textures ---

    /// Palette used for the transfer function (tex 2)
    virtual const AObjectPalette* glPalette( unsigned tex = 0 ) const;
    /** Texture dimension for each texture index:
        3 for the volume (tex 0) and the gradient (tex 1),
        1 for the transfer function (tex 2).
    */
    virtual unsigned glDimTex( const ViewState &, unsigned tex = 0 ) const;
    /** Fills the GL texture allocated by GLComponent for texture \p tex.
        Dispatches to makeVolumeTexture(), makeGradientTexture() or
        makeTransferFunctionTexture(). Textures are expected to be built in
        index order: the gradient (tex 1) is computed from the volume
        texture (tex 0).
    */
    virtual bool glMakeTexImage( const ViewState &state,
                                 const GLTexture &gltex, unsigned tex ) const;

    // --- Proxy geometry ---
    /** Compiles the proxy cube (volume bounding box) into a GL list.
        Face culling is disabled so that rays can start from any visible
        face. The actual volume content is computed in the fragment shader.
    */
    virtual bool glMakeBodyGLL( const ViewState &state,
                                const GLList &gllist ) const;


    void createDefaultPalette( const std::string & name = "" );
    virtual AObjectPalette* palette();
    virtual const AObjectPalette* palette() const;
    /** Sets the palette and flags the transfer function texture (tex 2)
        for rebuild. Palette bounds (min1/max1) are sent as uniforms every
        frame and do not require a texture rebuild.
    */
    virtual void setPalette( const AObjectPalette &pal );
    virtual Material & GetMaterial(); 
    virtual const Material & material() const;
    virtual const Material* glMaterial() const;
    virtual void SetMaterial( const Material &mat );
    /// Always true: the ray-marching result is alpha-blended
    virtual bool isTransparent() const;

    /** Sends VObject-specific uniforms to the shader program
        (bounding box, volume maximum value, palette bounds).
        Called by GLObjectUniforms right after the program is bound.
        Textures are bound by the generic GLObjectUniforms mechanism.
    */
    virtual void updateObjectUniforms(QOpenGLShaderProgram* shader) override;

    // --- Change notification ---

    virtual void glSetChanged( glPart, bool = true ) const;
    virtual void glSetTexImageChanged( bool = true, unsigned tex = 0 ) const;
    virtual void glSetTexEnvChanged( bool = true, unsigned tex = 0 ) const;
    /** Reacts to changes of the source volume: volume data changes flag
        both the volume (tex 0) and gradient (tex 1) textures for rebuild.
    */
    virtual void update( const Observable*, void* );
    
    /// Always true: the result depends on the view direction
    virtual bool renderingIsObserverDependent() const;

    // --- Shaders ---

    /// Vertex shader template: "volume.vs.glsl"
    virtual std::string glVertexShaderTemplate() const;
    /// Fragment shader template: "volume.fs.glsl"
    virtual std::string glFragmentShaderTemplate() const;

    /** Checks that the source volume is valid and updates the cached
        bounding box. Returns false if the volume is missing or has no
        bounding box.
    */
    bool checkObject() const;
    static int classType();

  protected:
    virtual std::string viewStateID( glPart part, const ViewState & ) const;

  private:

    /** Uploads the volume data (tex 0) as a GL_R32F 3D texture, records
        its GL name for the gradient computation, and updates the texture
        extrema with the volume maximum value.
    */
    bool makeVolumeTexture( GLuint texName ) const;

    /** Computes the density gradient (tex 1) with central differences.
        Uses a compute shader (OpenGL 4.3) when available, reading the
        volume texture directly on the GPU; falls back to a CPU computation
        otherwise.
    */
    bool makeGradientTexture( GLuint texName ) const;

    /** Builds the 1D transfer function (tex 2) from the VObject palette
        (colors and alpha).
    */
    bool makeTransferFunctionTexture( GLuint texName ) const;

    Private *d;
  };
}

#endif //ANA_MOBJECT_VOBJECT_H