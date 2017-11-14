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


#ifndef ANA_SURFACE_TEXTURE_H
#define ANA_SURFACE_TEXTURE_H


#include <anatomist/surface/globject.h>
#include <anatomist/graph/pythonAObject.h>
#include <aims/mesh/texture.h>


namespace anatomist
{

  /**	High-level Texture object in Anatomist. Encapsulates lower-level 
	templated textures (AIMS). Contains a vector of texture coordinates 
	for use with a surface (via a fusion)
  */
  class ATexture : public AGLObject, public PythonAObject
  {
  public:
    ATexture();
    virtual ~ATexture();

    virtual ObjectMenu* optionMenu() const;

    virtual unsigned size( float time = 0 ) const;
    virtual const float* textureCoords() const;
    virtual const float* textureCoords( float time ) const;
    virtual float textureTime( float time ) const;
    virtual const float* glTexCoordArray( const ViewState &, 
                                          unsigned tex = 0 ) const;
    virtual unsigned glNumTextures() const;
    virtual unsigned glNumTextures( const ViewState & ) const;
    virtual unsigned glDimTex( const ViewState &, unsigned tex = 0 ) const;
    virtual unsigned glTexCoordSize( const ViewState &, 
                                     unsigned tex = 0 ) const;

    virtual bool Is2DObject() { return( false ); }
    virtual bool Is3DObject() { return( false ); }

    virtual float MinT() const { return( MinT3D() ); }
    virtual float MaxT() const { return( MaxT3D() ); }
    virtual float MinT3D() const;
    virtual float MaxT3D() const;
    virtual unsigned dimTexture() const;

    virtual AObject* objectAt( const std::vector<float> &, float = 0 )
    { return 0 ; }

    virtual void setTexExtrema();
    void setTexExtrema(float min,float max);

    virtual void normalize();
    template <typename T> void setTexture( carto::rc_ptr<TimeTexture<T> >
        tex, bool normalize_data = true );
    template <typename T>
    carto::rc_ptr<TimeTexture<T> > texture( bool rescaled = false,
                                            bool always_copy = false );
    virtual void createDefaultPalette( const std::string & name = "" );
    virtual void update( const Observable* observable, void* arg );
    virtual void notifyObservers( void * = 0 );
    virtual void setInternalsChanged();

    virtual bool loadable() const { return true; }
    virtual bool savable() const { return true; }
    virtual bool reload( const std::string & filename );
    bool save( const std::string & filename );
    virtual AObject* clone( bool shallow = true );
    virtual carto::GenericObject* attributed();
    virtual const carto::GenericObject* attributed() const;

  protected:
    virtual void freeTexture();

  private:
    struct Private;
    template <typename T> struct Private_;
    Private		*d;
  };

}


#endif
