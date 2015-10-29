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


#ifndef ANA_MOBJECT_FUSION3D_H
#define ANA_MOBJECT_FUSION3D_H


#include <anatomist/mobject/globjectvector.h>
#include <map>
#include <vector>


namespace anatomist
{

  class Referential;
  class Geometry;


  /**	Anatomist object which contains a volume of functional data (fMRI, 
	PET...) and a surface of the MRI anatomic data
  */
  class Fusion3D : public GLObjectVector
  {
  public:
    /// 3D mapping methods
    enum Method
    {
      POINT_TO_POINT,
      INSIDE_POINT_TO_POINT,
      OUTSIDE_POINT_TO_POINT,
      LINE_TO_POINT,
      INSIDE_LINE_TO_POINT,
      OUTSIDE_LINE_TO_POINT,
      SPHERE_TO_POINT,
    };

    /// 3D mapping sub-methods
    enum SubMethod
    {
      MAX,
      MIN,
      MEAN,
      CORRECTED_MEAN,
      ENHANCED_MEAN,
      ABSMAX,
      MEDIAN,
    };

    Fusion3D( const std::vector<AObject *> & obj );
    virtual ~Fusion3D();

    virtual int MType() const { return( AObject::FUSION3D ); }

    virtual bool CanRemove( AObject* obj );

    virtual bool render( PrimList &, const ViewState & );
    virtual bool Is2DObject();
    virtual bool Is3DObject() { return true; }

    virtual bool boundingBox( Point3df & bmin, Point3df & bmax ) const;

    void setMethod( Method method );
    Method method() { return _method; }

    void setSubMethod( SubMethod submethod );
    SubMethod subMethod() { return _submethod; }

    void setDepth( float depth );
    float depth() { return _depth; }
    void setStep( float step );
    float step() { return _step; }

    virtual void update( const Observable* observable, void* arg );

    virtual void glClearHasChangedFlags() const;

    virtual Tree* optionTree() const;

    static Tree*	_optionTree;

    iterator firstVolume();
    const_iterator firstVolume() const;

    /*
    virtual bool glHasChanged( glPart ) const;
    virtual void glSetChanged( glPart, bool = true );
    */
    virtual unsigned glTexCoordSize( const ViewState &, unsigned ) const;
    virtual const GLfloat* glTexCoordArray( const ViewState &, 
                                            unsigned ) const;
    virtual bool hasTexture() const { return true; }
    virtual const GLComponent* glTexture( unsigned tex = 0 ) const;
    virtual GLComponent* glTexture( unsigned tex = 0 );
    virtual unsigned glNumTextures() const;
    virtual unsigned glNumTextures( const ViewState & ) const;
    virtual unsigned glDimTex( const ViewState & s, unsigned tex = 0 ) const;
    virtual const AObjectPalette* glPalette( unsigned tex = 0 ) const;
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

    //virtual bool glTexImageChanged( unsigned tex = 0 ) const;
    virtual void glSetTexImageChanged( bool x = true, unsigned tex = 0 ) const;
    //virtual bool glTexEnvChanged( unsigned tex = 0 ) const;
    virtual void glSetTexEnvChanged( bool x = true, unsigned tex = 0 ) const;
    virtual GLPrimitives glTexNameGLL( const ViewState &, 
                                       unsigned tex = 0 ) const;
    virtual GLPrimitives glTexEnvGLL( const ViewState &, 
                                      unsigned tex = 0 ) const;
    const TexExtrema & glTexExtrema( unsigned tex = 0 ) const;
    TexExtrema & glTexExtrema( unsigned tex = 0 );

    virtual void glGarbageCollector( int nkept = -1 );

  protected:
    ///		Refresh texture values
    void  refreshVTexture( const ViewState & ) const;
    const AObject* volume( unsigned n ) const;
    virtual std::string viewStateID( glPart part, const ViewState & ) const;

    ///		number of 3D objects
    unsigned	_nsurf;
    Method	_method;
    SubMethod	_submethod;
    float	_depth;
    float	_step;

  private:
    struct Private;

    ///	Disable edition
    virtual void insert( AObject* o ) { GLObjectVector::insert( o ); }
    virtual void insert( AObject* o, int pos )
    { GLObjectVector::insert( o, pos ); }
    virtual void insert( const carto::shared_ptr<AObject> & o,
                         int pos = -1 )
    { GLObjectVector::insert( o, pos ); }
    ///	Disable edition
    virtual void erase( iterator & i ) { GLObjectVector::erase( i ); }

    void  refreshVTextureWithPointToPoint( const ViewState &, 
                                           unsigned tex ) const;
    void  refreshVTextureWithLineToPoint( const ViewState &, 
                                          unsigned tex ) const;
    void  refreshVTextureWithInsideLineToPoint( const ViewState &, 
                                                unsigned tex ) const;
    void  refreshVTextureWithOutsideLineToPoint( const ViewState &, 
                                                 unsigned tex ) const;
    void  refreshVTextureWithSphereToPoint( const ViewState &, 
                                            unsigned tex ) const;
    /**		Refresh textures with lines and various ranges along the line. 
		Called by the various refreshTextureWith*Line* functions
    */
    void refreshLineTexture( int minIter, int maxIter, float estep, 
			     const ViewState &, unsigned tex ) const;

    float enhancedMean( float value, int nbElemCorrected, int nbElem ) const;

    Private	*d;
  };


  //


  inline bool Fusion3D::CanRemove( AObject * ) 
  { 
    return(false);
  } 



  inline float Fusion3D::enhancedMean( float value, 
				       int nbElemCorrected, 
				       int nbElem ) const
  {
    if( nbElemCorrected == 0 )
      return(0.0);
    return( value / ((float) nbElemCorrected) 
	    * ( 1. - 0.7 * ((float) nbElemCorrected) / ((float) nbElem) ) );
  }

}


#endif
