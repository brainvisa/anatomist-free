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

#ifndef ANA_SURFACE_GLCOMPONENT_H
#define ANA_SURFACE_GLCOMPONENT_H

#include <anatomist/primitive/primitive.h>
#include <anatomist/window/glcaps.h>
#include <anatomist/window/viewstate.h>
#include <anatomist/surface/Shader.h>
#include <aims/rgb/rgb.h>
#include <cartobase/object/object.h>
#include <cartodata/volume/volume.h>
#include <vector>
#include <set>


namespace anatomist
{

  class Material;
  class AObjectPalette;
  class Sliceable;
  class Referential;


  /** Base class for all OpenGL objects in Anatomist (with OpenGL display
      lists)

      This object represents the virtual API which allows Anatomist to use
      objects geometric properties, and to render in 3D OpenGL views.

      For a general explanation on how it is organized and works, see the
      \ref objects_opengl page.
  */
  class GLComponent
  {
  public:
    enum glTextureMode
    {
      glGEOMETRIC,
      glLINEAR,
      glREPLACE,
      glDECAL,
      glBLEND,
      glADD,
      glCOMBINE,
      glLINEAR_A_IF_A,
      glLINEAR_ON_DEFINED,
      glLINEAR_A_IF_B = glLINEAR_ON_DEFINED,
      glLINEAR_A_IF_NOT_A,
      glLINEAR_A_IF_NOT_B,
      glLINEAR_B_IF_A,
      glLINEAR_B_IF_B,
      glLINEAR_B_IF_NOT_A,
      glLINEAR_B_IF_NOT_B,
      glLINEAR_A_IF_A_ALPHA,
      glLINEAR_A_IF_NOT_B_ALPHA,
      glLINEAR_B_IF_B_ALPHA,
      glLINEAR_B_IF_NOT_A_ALPHA,
      glMAX_CHANNEL,
      glMIN_CHANNEL,
      glMAX_ALPHA,
      glMIN_ALPHA,
      glGEOMETRIC_SQRT,
      glGEOMETRIC_LIGHTEN,
    };

    enum glTextureFiltering
    {
      glFILT_NEAREST,
      glFILT_LINEAR,
    };

    enum glPart
    {
      glGENERAL,
      glBODY,
      glMATERIAL,
      glGEOMETRY,
      glPALETTE,
      glREFERENTIAL,
      glTEXIMAGE,
      glTEXENV,

      glNOPART,
      /** glTEXIMAGE_NUM + 2* texnum flags texture #texnum, reserved for
          Observable pattern */
      glTEXIMAGE_NUM,
      /** glTEXENV_NUM + 2* texnum flags texture #texnum, reserved for
          Observable pattern */
      glTEXENV_NUM,
    };

    enum glAutoTexturingMode
    {
      glTEX_MANUAL,
      glTEX_OBJECT_LINEAR,
      glTEX_EYE_LINEAR,
      glTEX_SPHERE_MAP,
      glTEX_REFLECTION_MAP,
      glTEX_NORMAL_MAP,
    };

    typedef ViewState::glSelectRenderMode glSelectRenderMode;

    struct TexExtrema
    {
      TexExtrema();

      // min of data in memory (for OpenGL: between 0 and 1)
      std::vector<float>	min;
      // max of data in memory (for OpenGL: between 0 and 1)
      std::vector<float>	max;
      // min of initial (quantified) data
      std::vector<float>	minquant;
      // max of initial (quantified) data
      std::vector<float>	maxquant;
      bool			scaled;
    };

    struct TexInfo;

    GLComponent();
    virtual ~GLComponent();

    virtual const GLComponent* glAPI() const { return this; }
    virtual GLComponent* glAPI() { return this; }
    virtual const Sliceable* sliceableAPI() const { return 0; }
    virtual Sliceable* sliceableAPI() { return 0; }

    virtual void glClearHasChangedFlags() const;
    virtual void glSetChanged( glPart, bool = true ) const;
    virtual bool glHasChanged( glPart ) const;

    void setShader(const Shader &shader);
    void removeShader();
    /// create, setup, or remove the shader
    void setupShader();
    const Shader *getShader() const;
    Shader *getShader();
    virtual void setShaderParameters(const Shader &shader,
                                     const ViewState & state) const;

    virtual const Material *glMaterial() const;
    virtual const AObjectPalette* glPalette( unsigned tex = 0 ) const;

    virtual unsigned glNumVertex( const ViewState & ) const;
    virtual const GLfloat* glVertexArray( const ViewState & ) const;
    /// normals array (optional), default=0 (no normals, flat shaded faces)
    virtual const GLfloat* glNormalArray( const ViewState & ) const;
    /// number of vertices per polygon (default = 3: triangles)
    virtual unsigned glPolygonSize( const ViewState & ) const;
    virtual unsigned glNumPolygon( const ViewState & ) const;
    virtual const GLuint* glPolygonArray( const ViewState & ) const;
    /** Limit to the number of displayed polygons (default: 0, unlimited or )
        global default value) */
    virtual unsigned long glMaxNumDisplayedPolygons() const;
    virtual void glSetMaxNumDisplayedPolygons( unsigned long n );
    static unsigned long glGlobalMaxNumDisplayedPolygons();
    static void glSetGlobalMaxNumDisplayedPolygons( unsigned long n );

    // Texturing functions
    virtual void glSetTexImageChanged( bool x = true, 
                                       unsigned tex = 0 ) const ;
    virtual void glSetTexEnvChanged( bool x = true, unsigned tex = 0 ) const;
    virtual unsigned glNumTextures() const;
    virtual unsigned glNumTextures( const ViewState & ) const;
    virtual glTextureMode glTexMode( unsigned tex = 0 ) const;
    virtual void glSetTexMode( glTextureMode mode, unsigned tex = 0 );
    virtual float glTexRate( unsigned tex = 0 ) const;
    virtual void glSetTexRate( float rate, unsigned tex = 0 );
    virtual glTextureFiltering glTexFiltering( unsigned tex = 0 ) const;
    virtual void glSetTexFiltering( glTextureFiltering x, unsigned tex = 0 );
    virtual glAutoTexturingMode glAutoTexMode( unsigned tex = 0 ) const;
    virtual void glSetAutoTexMode( glAutoTexturingMode mode, 
                                   unsigned tex = 0 );
    virtual const float *glAutoTexParams( unsigned coord = 0, 
                                          unsigned tex = 0 ) const;
    virtual void glSetAutoTexParams( const float* params, unsigned coord = 0, 
                                     unsigned tex = 0 );
    virtual bool glTexImageChanged( unsigned tex = 0 ) const;
    virtual bool glTexEnvChanged( unsigned tex = 0 ) const;
    virtual GLint glGLTexMode( unsigned tex = 0) const;
    virtual GLint glGLTexFiltering( unsigned tex = 0 ) const;
    virtual void glSetTexRGBInterpolation( bool x, unsigned tex = 0 );
    virtual bool glTexRGBInterpolation( unsigned tex = 0 ) const;
    /// texture dimension (1, 2 [or 3])
    virtual unsigned glDimTex( const ViewState &, unsigned tex = 0 ) const;
    /// texture array size (must be >= numVertex to work), defalut=0
    virtual unsigned glTexCoordSize( const ViewState &, 
                                     unsigned tex = 0 ) const;
    virtual const GLfloat* glTexCoordArray( const ViewState &, 
                                            unsigned tex = 0 ) const;
    virtual const TexExtrema & glTexExtrema( unsigned tex = 0 ) const;
    virtual TexExtrema & glTexExtrema( unsigned tex = 0 );

    virtual GLPrimitives glMainGLL( const ViewState & );
    /** Frees unused display lists

    Display lists are created when requested by views, according to a view 
    state, and are stored in the object. As several view states will probably 
    be needed by different views, they are all kept internally (by a reference 
    counter). When a display list is no longer used by views, it is still not 
    destroyed because it could be costy to re-crete it for another view later.

    To avoid objects to grow and eat all memory, this function will be called 
    from time to time (when creating new lists) to free some lists which 
    are not currently used by any view.

    \param nkept if negative, glStateMemory() unused items are kept in the 
    object, otherwise the specified number are kept
    */
    virtual void glGarbageCollector( int nkept = -1 );
    /** Number of unused display lists in different states kept in memory
        (default: 4)
    */
    unsigned glStateMemory() const;
    virtual void glSetStateMemory( unsigned n );

    /**	If you want to make a non-standard GL display list for the body 
        geometry, overload this function to fill \c gllist
        \return true on success, false if the list could not be created
    */
    virtual bool glMakeBodyGLL( const ViewState & state, 
                                const GLList & gllist ) const;
    /// GL list to execute before the body is rendered
    virtual void glBeforeBodyGLL( const ViewState & state, 
				  GLPrimitives & pl ) const;
    /// GL list to execute after the body is rendered
    virtual void glAfterBodyGLL( const ViewState & state, 
                                 GLPrimitives & pl ) const;
    /**  If you make non-standard textures (ie not from a palette), overload
         this function.

         New in Anatomist 4.5, it is used by glMakeTexImage, and may also be
         used to save the texture data of an arbitrary object.

         \param dimx and dimy are target texture sizes. If left to -1 (default), the
         size is calculated internally.

         \param useTexScale if true (default), allow using OpenGL scaling in
         texture space. Otherwise the palette image will be adapted to
         fit the scale.
    */
    virtual carto::VolumeRef<AimsRGBA> glBuildTexImage(
      const ViewState & state, unsigned tex, int dimx = -1,
      int dimy = -1, bool useTexScale = true ) const;
    /** If you make non-standard textures (ie not from a palette), overload
        this function to fill \c gltex 

        Since Anatomist 4.5, glMakeTexImage calls glBuildTexImage, which is
        actually the method to be overloaded by subclasses.

        \return true on success, false if a texture could not be created
    */
    virtual bool glMakeTexImage( const ViewState & state, 
                                 const GLTexture & gltex, unsigned tex ) const;
    virtual bool glMakeTexEnvGLL( const ViewState & state, 
                                  const GLList & gllist, unsigned tex ) const;

    virtual GLPrimitives glBodyGLL( const ViewState & state ) const;
    virtual GLPrimitives glMaterialGLL( const ViewState & state ) const;
    virtual GLPrimitives glTexNameGLL( const ViewState &, 
                                       unsigned tex = 0 ) const;
    virtual GLPrimitives glTexEnvGLL( const ViewState &, 
                                      unsigned tex = 0 ) const;
    virtual void glSetMainGLL( const std::string & state, GLPrimitives x );
    virtual void glSetBodyGLL( const std::string & state, RefGLItem x );
    virtual void glSetMaterialGLL( const std::string & state, RefGLItem x );
    virtual void glSetTexNameGLL( const std::string & state, RefGLItem x, 
                                  unsigned tex = 0 );
    /// List of actually allowed texturing modes for this particular object
    virtual std::set<glTextureMode> 
    glAllowedTexModes( unsigned tex = 0 ) const;
    virtual std::set<glTextureFiltering> 
    glAllowedTexFilterings( unsigned tex = 0 ) const;
    virtual std::set<glAutoTexturingMode> 
    glAllowedAutoTexModes( unsigned tex = 0 ) const;
    virtual bool glAllowedTexRGBInterpolation( unsigned tex = 0 ) const;
    virtual bool glAllowedTexRate( unsigned tex = 0 ) const;

    /** Makes a unique ID from a viewstate.

    The ID must be unique within the GLComponent instance and for the part 
    \c part. Depending on the aspects of the viewstate to take into account, 
    subclasses may have to overload it.

    For instance, a mesh may build an ID only taking time into account, whereas
    a volume slice must also code the slice position and orientation into 
    the ID.
    */
    virtual std::string viewStateID( glPart part, const ViewState & ) const;
    virtual carto::Object debugInfo() const;

    /// helper function used in AObject::render()
    static GLPrimitives glHandleTransformation( const ViewState & vs,
                                                const Referential* myref );
    /** called after glHandleTransformation() and glMainGLL() to pop the
        transformation matrix */
    static GLPrimitives glPopTransformation( const ViewState & vs,
                                             const Referential* myref );
    int glObjectID() const;
    virtual TexInfo & glTexInfo( unsigned tex = 0 ) const;

  protected:
    void glAddTextures( unsigned ntex = 1 );

  private:
    struct Private;
    Private	*d;
    Shader      *_shader;
  };


  inline void GLComponent::setShader(const Shader &shader)
  { 
    if (_shader) delete _shader;
    _shader = new Shader(shader);
    glSetChanged( GLComponent::glBODY );
  }
  inline void GLComponent::removeShader(void)
  { 
    if (_shader)
    {
      delete _shader;
      _shader = NULL;
      glSetChanged( GLComponent::glBODY );
    }
  }
  inline const Material *GLComponent::glMaterial() const          { return 0; }
  inline const Shader *GLComponent::getShader() const { return _shader; }
  inline Shader *GLComponent::getShader() { return _shader; }
  inline void GLComponent::setShaderParameters(const Shader &shader, const ViewState & state) const { shader.setShaderParameters(*this, state); }
  inline const AObjectPalette* GLComponent::glPalette( unsigned ) const
  { return 0; }
  inline unsigned GLComponent::glNumVertex( const ViewState & ) const
  { return 0; }
  inline const GLfloat* GLComponent::glVertexArray( const ViewState & ) const
  { return 0; }
  inline const GLfloat* GLComponent::glNormalArray( const ViewState & ) const
  { return 0; }
  inline unsigned GLComponent::glPolygonSize( const ViewState & ) const
  { return 3; }
  inline unsigned GLComponent::glNumPolygon( const ViewState & ) const
  { return 0; }
  inline const GLuint* GLComponent::glPolygonArray( const ViewState & ) const
  { return 0; }

  inline unsigned GLComponent::glDimTex( const ViewState &, unsigned ) const
  { return 1; }
  inline unsigned GLComponent::glTexCoordSize( const ViewState &, 
                                               unsigned ) const
  { return 0; }
  inline const GLfloat* GLComponent::glTexCoordArray( const ViewState &, 
                                                      unsigned ) const 
  { return 0; }

}


#endif


