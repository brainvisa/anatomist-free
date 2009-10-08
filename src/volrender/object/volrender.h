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


#ifndef ANATOMIST_OBJECT_VOLRENDER_H
#define ANATOMIST_OBJECT_VOLRENDER_H

#include <anatomist/mobject/objectVector.h>
#include <anatomist/surface/glcomponent.h>


namespace anatomist
{
  template<typename T> class AVolume;


  class VolRender : public ObjectVector, public GLComponent
  {
  public:
    struct Private;

    VolRender( AObject * vol );
    virtual ~VolRender();

    virtual bool render( PrimList &, const ViewState & );
    virtual bool Is2DObject() { return true; }
    virtual bool Is3DObject() { return false; }
    virtual Tree* optionTree() const;
    virtual bool CanRemove( AObject * ) { return false; }

    void createDefaultPalette( const std::string & name = "" );
    virtual AObjectPalette* palette();
    virtual const AObjectPalette* palette() const;
    virtual void setPalette( const AObjectPalette &pal );
    virtual Material & GetMaterial();
    virtual const Material & material() const;
    virtual void SetMaterial( const Material &mat );
    virtual bool isTransparent() const;
    virtual bool renderingIsObserverDependent() const;

    virtual void glSetChanged( glPart, bool = true ) const;
    virtual void glSetTexImageChanged( bool = true, unsigned tex = 0 ) const;
    virtual void glSetTexEnvChanged( bool = true, unsigned tex = 0 ) const;

    virtual const GLComponent* glAPI() const;
    virtual GLComponent* glAPI();
    virtual const Material* glMaterial() const;
    virtual const AObjectPalette* glPalette( unsigned tex = 0 ) const;

    virtual unsigned glDimTex( const ViewState &, unsigned tex = 0 ) const;
    virtual bool glMakeTexImage( const ViewState &state,
                                 const GLTexture &gltex, unsigned tex ) const;
    virtual bool glMakeBodyGLL( const ViewState &state,
                                const GLList &gllist ) const;
    virtual const GLComponent::TexExtrema &
        glTexExtrema( unsigned tex = 0 ) const;
    virtual GLComponent::TexExtrema & glTexExtrema( unsigned tex = 0 );
    virtual void update(const Observable* observable, void* arg);

    bool checkObject() const;
    std::string shaderType() const;
    bool setShaderType( const std::string & );
    unsigned maxSlices() const;
    void setMaxSlices( unsigned n );
    int slabSize() const;
    void setSlabSize( int n );

    static int classType();
    static void volrenderProperties( const std::set<anatomist::AObject *> & );

  protected:
    virtual std::string viewStateID( glPart part, const ViewState & ) const;

  private:
    Private *d;
  };

}


#endif



