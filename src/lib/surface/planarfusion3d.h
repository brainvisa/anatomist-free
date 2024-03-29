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


#ifndef ANA_SURFACE_PLANARFUSION3D_H
#define ANA_SURFACE_PLANARFUSION3D_H


#include <anatomist/mobject/globjectvector.h>
#include <map>
#include <vector>


namespace anatomist
{

  class Referential;
  class Geometry;


  class PlanarFusion3D : public GLObjectVector
  {
  public:
    PlanarFusion3D( const std::vector<AObject *> & obj );
    virtual ~PlanarFusion3D();

    virtual int MType() const { return( type() ); }
    static int classType();

    virtual bool CanRemove( AObject* obj );

    virtual bool Is2DObject() { return( false ); }
    virtual bool Is3DObject() { return( true ); }

    virtual bool boundingBox( std::vector<float> & bmin,
                              std::vector<float> & bmax ) const;

    virtual bool refreshTexCoords( const ViewState & state ) const;

    virtual void update( const Observable* observable, void* arg );
    virtual void glClearHasChangedFlags() const;

    virtual Tree* optionTree() const;

    virtual unsigned glNumTextures() const;
    virtual unsigned glNumTextures( const ViewState & ) const;
    virtual unsigned glDimTex( const ViewState & state, 
                               unsigned tex = 0 ) const;
    virtual unsigned glTexCoordSize( const ViewState & state, 
                                     unsigned tex = 0 ) const;
    virtual const GLfloat* glTexCoordArray( const ViewState & state, 
                                            unsigned tex = 0 ) const;
    virtual void glSetTexImageChanged( bool, unsigned tex = 0 ) const;
    virtual glTextureMode glTexMode( unsigned tex = 0 ) const;
    virtual float glTexRate( unsigned tex = 0 ) const;
    virtual glTextureFiltering glTexFiltering( unsigned tex = 0 ) const;
    virtual glAutoTexturingMode glAutoTexMode( unsigned tex = 0 ) const;
    virtual const float *glAutoTexParams( unsigned coord = 0, 
                                          unsigned tex = 0 ) const;
    virtual void glSetAutoTexParams( const float* params, unsigned coord = 0, 
                                     unsigned tex = 0 );
    virtual std::string viewStateID( glPart part, const ViewState & ) const;

    const AObject* volume() const;
    AObject* volume();
    const AObject* mesh() const;
    AObject* mesh();
    virtual const AObjectPalette* glPalette( unsigned ) const;
    virtual const GLComponent* glTexture( const ViewState & vs,
                                          unsigned ) const;
    virtual GLComponent* glTexture( const ViewState & vs, unsigned );

    virtual carto::VolumeRef<AimsRGBA> glBuildTexImage(
      const ViewState & state, unsigned tex, int dimx = -1,
      int dimy = -1, bool useTexScale = true ) const;
    virtual bool glMakeTexImage( const ViewState & state,
                                 const GLTexture & gltex, unsigned tex ) const;
    virtual AObject* fallbackReferentialInheritance() const;

  private:
    struct Private;
    ///	ensures the object class is registered in Anatomist
    static int registerClass();

    Private	*d;
  };


  inline bool PlanarFusion3D::CanRemove( AObject * ) 
  { 
    return(false);
  } 

}


#endif


