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


#ifndef ANATOMIST_PRIMITIVE_PRIMITIVE_H
#define ANATOMIST_PRIMITIVE_PRIMITIVE_H

#include <cartobase/smart/rcobject.h>
#include <QOpenGLShaderProgram>
#include <anatomist/surface/IShaderModule.h>
#include <list>
#include <set>


namespace anatomist
{
  class GLWidgetManager;
  class GLComponent;
  class ViewState;


  /** OpenGL item (display list, texture, ...) with reference counter and 
      cleanup upon destruction */
  class GLItem : public carto::RCObject
  {
  public:
    inline GLItem() : carto::RCObject(), _ghost( false ) {}
    virtual ~GLItem();
    virtual carto::rc_ptr<QOpenGLShaderProgram> shader() const { return carto::rc_ptr<QOpenGLShaderProgram>();}

    /// renders the display list
    virtual void callList() const = 0;
    /** ghost lists are not selectable and not rendered in Z buffers for 
        selection */
    virtual bool ghost() const;
    virtual void setGhost( bool );

  private:
    bool	_ghost;
  };


  /// Multiple display lists and texture objects
  class Primitive : public GLItem
  {
  public:
    Primitive();
    /// Primitive destructor makes the shatred context current
    virtual ~Primitive();

    const std::list<unsigned> & glLists() const;
    const std::list<unsigned> & textures() const;
    void insertList( unsigned );
    void insertTexture( unsigned );
    void deleteList( unsigned );
    void deleteTexture( unsigned );
    virtual void callList() const;

    void clear();

  protected:
    std::list<unsigned>	_gll;
    std::list<unsigned>	_tex;

  private:
  };


  // Single OpenGL display list
  class GLList : public GLItem
  {
  public:
    inline GLList( unsigned gllist = 0 ) : GLItem(), _item( gllist ) {}
    /// GLList destructor makes the shatred context current
    virtual ~GLList();

    unsigned item() const { return _item; }
    /** generates a new display list. Must be called when a valid OpenGL 
        context is bound */
    virtual void generate();
    virtual void callList() const;

  private:
    unsigned	_item;
  };


  // Single OpenGL texture object
  class GLTexture : public GLItem
  {
  public:
    GLTexture( unsigned gltex = 0 ) : GLItem(), _item( gltex ) {}
    /// GLTexture destructor makes the shatred context current
    virtual ~GLTexture();

    unsigned item() const { return _item; }
    /** generates a new texture name. Must be called when a valid OpenGL 
        context is bound */
    virtual void generate();
    virtual void callList() const;

  private:
    unsigned	_item;
  };

  // OpenGL Shader binding
  class GLBindShader : public GLItem
  {
  public:
    GLBindShader(carto::rc_ptr<QOpenGLShaderProgram> glShader) : GLItem(), _shaderProgram(glShader) {}
    virtual ~GLBindShader();
    carto::rc_ptr<QOpenGLShaderProgram> shader() const override { return _shaderProgram; }
    virtual void callList() const;
    private:
      carto::rc_ptr<QOpenGLShaderProgram> _shaderProgram;
  };

  // OpenGL Shader Release
  class GLReleaseShader : public GLItem
  {
  public:
    GLReleaseShader(carto::rc_ptr<QOpenGLShaderProgram> glShader) : GLItem(), _shaderProgram(glShader) {}
    virtual ~GLReleaseShader();
    virtual void callList() const;
    private:
      carto::rc_ptr<QOpenGLShaderProgram> _shaderProgram;
  };


  typedef carto::rc_ptr<GLItem>		RefGLItem;
  typedef carto::rc_ptr<Primitive>	RefPrimitive;
  typedef carto::rc_ptr<GLList>		RefGLList;
  typedef carto::rc_ptr<GLTexture>	RefGLTexture;
  typedef std::list<RefGLItem>		GLPrimitives;


  class GLItemList : public GLItem
  {
  public:
    GLItemList() : GLItem() {}
    virtual ~GLItemList();
    virtual void callList() const;

    std::list<RefGLItem>	items;
  };


  typedef carto::rc_ptr<GLItemList>	RefGLItemList;


  class GLNoExecItemList : public GLItemList
  {
  public:
    GLNoExecItemList() : GLItemList() {}
    virtual ~GLNoExecItemList();
    virtual void callList() const {}
  };



    // Setup Scene uniforms for a shader
  class GLSceneUniforms : public GLItem
  {
  public:
    GLSceneUniforms(carto::rc_ptr<IShaderModule> shaderModule ,carto::rc_ptr<QOpenGLShaderProgram> glShader, GLWidgetManager* scene) : GLItem(), _module(shaderModule), _shader(glShader), _scene(scene) {}
    virtual ~GLSceneUniforms();
    virtual void callList() const;
  private:
    carto::rc_ptr<IShaderModule> _module;
    carto::rc_ptr<QOpenGLShaderProgram> _shader;
    GLWidgetManager* _scene;

  };

  // Setup Object uniforms for a shader
  class GLObjectUniforms : public GLItem
  {
  public:
    struct UniformsLocations
    {
      std::array<GLint, 3> texture = {-1, -1, -1};
      std::array<GLint, 3> nbTexture = {-1, -1, -1};
      GLint hasTexture = -1;
      GLint textureDim = -1;
      GLint texModes = -1;
    };

    struct TexturesData
    {
      std::vector<GLint> texUnits1D;
      std::vector<GLint> texUnits2D;
      std::vector<GLint> texUnits3D;
      std::vector<GLint> texModes;
      std::set<unsigned> usedUnits;
      int nbTexture = 0;
      std::vector<int> textureDim;
    };

    GLObjectUniforms(carto::rc_ptr<IShaderModule> shaderModule, carto::rc_ptr<QOpenGLShaderProgram> glShader, GLComponent* glObj) : GLItem(),_module(shaderModule), _shader(glShader), _glObj(glObj) {}
    virtual ~GLObjectUniforms();
    virtual void callList() const;
    virtual void getUniformsLocations(UniformsLocations & locations) const;
    virtual void getTexturesData(const ViewState& vs, const unsigned maxSampler, TexturesData& data) const;
    virtual void updateTextureUniforms(const UniformsLocations& locations,TexturesData& data, const unsigned maxSampler) const;
  private:
    carto::rc_ptr<IShaderModule> _module;
    carto::rc_ptr<QOpenGLShaderProgram> _shader;
    GLComponent* _glObj;
  };

}

#endif
