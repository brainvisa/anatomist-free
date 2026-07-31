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
  class VObject : public ObjectVector, public GLComponent
  {
   
  public:
    struct Private;

    VObject( AObject * vol );
    virtual ~VObject();

    virtual bool render( PrimList &, RenderContext & );
    virtual bool Is2DObject() { return true; }
    virtual bool Is3DObject() { return false; }
    virtual bool CanRemove( AObject * );
    virtual Tree* optionTree() const;

    virtual const GLComponent* glAPI() const override;
    virtual GLComponent* glAPI() override;

    // --- Texture 3D ---
    virtual const AObjectPalette* glPalette( unsigned tex = 0 ) const;
    virtual unsigned glDimTex( const ViewState &, unsigned tex = 0 ) const;
    virtual bool glMakeTexImage( const ViewState &state,
                                 const GLTexture &gltex, unsigned tex ) const;

    // --- Géométrie proxy ---
    virtual bool glMakeBodyGLL( const ViewState &state,
                                const GLList &gllist ) const;


    void createDefaultPalette( const std::string & name = "" );
    virtual AObjectPalette* palette();
    virtual const AObjectPalette* palette() const;
    virtual void setPalette( const AObjectPalette &pal );
    virtual Material & GetMaterial();
    virtual const Material & material() const;
    virtual const Material* glMaterial() const;
    virtual void SetMaterial( const Material &mat );
    virtual bool isTransparent() const;
    virtual void updateObjectUniforms(QOpenGLShaderProgram* _shader) override;


    virtual void glSetChanged( glPart, bool = true ) const;
    virtual void glSetTexImageChanged( bool = true, unsigned tex = 0 ) const;
    virtual void glSetTexEnvChanged( bool = true, unsigned tex = 0 ) const;
    virtual void update( const Observable*, void* );

    virtual bool renderingIsObserverDependent() const;

    virtual std::string glVertexShaderTemplate() const;
    virtual std::string glFragmentShaderTemplate() const;

    bool checkObject() const;
    static int classType();

  protected:
    virtual std::string viewStateID( glPart part, const ViewState & ) const;

  private:
    Private *d;
  };
}

#endif //ANA_MOBJECT_VOBJECT_H