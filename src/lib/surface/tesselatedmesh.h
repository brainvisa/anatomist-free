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


#ifndef ANA_SURFACE_TESSELATEDMESH_H
#define ANA_SURFACE_TESSELATEDMESH_H


#include <anatomist/mobject/globjectvector.h>
#include <vector>


namespace anatomist
{

  class Referential;
  class Geometry;


  class TesselatedMesh : public GLObjectVector
  {
  public:
    struct Private;

    TesselatedMesh( const std::vector<AObject *> & obj );
    virtual ~TesselatedMesh();

    virtual int MType() const { return( type() ); }
    static int classType();

    virtual bool CanRemove( AObject* obj );

    virtual void update( const Observable* observable, void* arg );
    virtual bool render( PrimList &, const ViewState & );
    virtual bool Is2DObject() { return( false ); }
    virtual bool Is3DObject() { return( true ); }
    virtual unsigned glNumVertex( const ViewState & ) const;
    virtual const GLfloat* glVertexArray( const ViewState & ) const;
    virtual const GLfloat* glNormalArray( const ViewState & ) const;
    virtual unsigned glPolygonSize( const ViewState & ) const;
    virtual unsigned glNumPolygon( const ViewState & ) const;
    virtual const GLuint* glPolygonArray( const ViewState & ) const;

    virtual void SetMaterial( const Material & );
    virtual const Material *glMaterial() const;
    virtual Material & GetMaterial();
    virtual const Material & material() const;
    virtual AObject* fallbackReferentialInheritance() const;

    virtual Tree* optionTree() const;

    virtual GLComponent* glGeometry();
    virtual GLComponent* glTexture( unsigned n = 0 );
    virtual const GLComponent* glGeometry() const;
    virtual const GLComponent* glTexture( unsigned n = 0 ) const;

    const AObject* tesselatedMesh() const;
    AObject* tesselatedMesh();
    const AObject* firstPolygon() const;
    AObject* firstPolygon();
    void tesselate( const ViewState & ) const;

  private:
    /// ensures the object class is registered in Anatomist
    static int registerClass();

    mutable Private     *d;
  };


  inline bool TesselatedMesh::CanRemove( AObject * )
  {
    return false;
  }

}


#endif


