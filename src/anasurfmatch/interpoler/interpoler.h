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


#ifndef ANATOMIST_INTERPOLER_INTERPOLER_H
#define ANATOMIST_INTERPOLER_INTERPOLER_H


#include <anatomist/mobject/globjectvector.h>


namespace anatomist
{

  template<int D> class ASurface;
  typedef ASurface<3> ATriangulated;
  //class ATriangulated;
  class ATexture;
  class ATexSurface;


  /**	Interpoler object: associates a ATexSurface and a Triangulated to 
	map the TexSurface's texture onto the Triangulated mesh, using 
	interpolation (spheric spline for ex.). The two surfaces must 
	approximately match (use a ASurfMatcher) in space.
  */
  class AInterpoler : public GLObjectVector
  {
  public:
    AInterpoler( AObject* texsurf, AObject* surf );
    virtual ~AInterpoler();

    virtual bool CanRemove( AObject* ) { return( false ); }

    virtual GLComponent* glGeometry();
    virtual const GLComponent* glGeometry() const;
    virtual GLComponent* glTexture();
    virtual const GLComponent* glTexture() const;
    virtual GLComponent* orgGeom();
    virtual const GLComponent* orgGeom() const;
    virtual GLComponent* texSurf();
    virtual const GLComponent* texSurf() const;

    virtual bool Is2DObject() { return false; }
    virtual bool Is3DObject() { return true; }

    virtual unsigned glTexCoordSize( const ViewState &, 
                                     unsigned tex = 0 ) const;
    virtual const GLfloat* glTexCoordArray( const ViewState & s, 
                                    unsigned tex = 0 ) const;

    virtual void update( const Observable* observable, void* arg );
    void setNeighboursChanged();
    void clearNeighboursChanged();
    bool hasNeighboursChanged() const;

    static int classType() { return( _classType ); }
    virtual Tree* optionTree() const;

    static Tree*	_optionTree;

  protected:
    virtual void computeNeighbours( const ViewState & state );
    void refreshTexCoordArray( const ViewState & state );

  private:
    struct Private;
    Private		*d;

    ///	ensures the object class is registered in Anatomist
    static int registerClass();

    static int	_classType;
  };

}


#endif
