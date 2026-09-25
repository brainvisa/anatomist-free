#ifndef ANA_SURFACE_TEXBUCKET_H
#define ANA_SURFACE_TEXBUCKET_H


#include <anatomist/mobject/globjectvector.h>


template <int D, typename T> class AimsSurface;
class Void;


namespace anatomist
{

    class ATexBucket : public GLObjectVector
    {
    public:
      ATexBucket( AObject* bucket, AObject* texture );
      virtual ~ATexBucket();
      virtual bool CanRemove( AObject *obj ) { return false; }

      virtual GLComponent* glGeometry( const ViewState & ) { return this; }
      virtual GLComponent* glTexture( const ViewState & , unsigned n = 0 )
      { return this; }
      virtual const GLComponent* glGeometry( const ViewState & ) const
      { return this; }
      virtual const GLComponent* glTexture( const ViewState & ,
                                            unsigned n = 0 ) const
      { return this; }

      virtual unsigned glNumVertex( const ViewState & ) const;
      virtual const GLfloat* glVertexArray( const ViewState & ) const;
      virtual const GLfloat* glNormalArray( const ViewState & ) const;
      virtual unsigned glPolygonSize( const ViewState & ) const;
      virtual unsigned glNumPolygon( const ViewState & ) const;
      virtual const GLuint* glPolygonArray( const ViewState & ) const;

      virtual unsigned glNumTextures() const;
      virtual unsigned glNumTextures( const ViewState & ) const;
      virtual glTextureMode glTexMode( unsigned tex = 0 ) const;
      virtual void glSetTexMode( glTextureMode mode, unsigned tex = 0 );
      virtual float glTexRate( unsigned tex = 0 ) const;
      virtual void glSetTexRate( float rate, unsigned tex = 0 );
      virtual glTextureFiltering glTexFiltering( unsigned tex = 0 ) const;
      virtual void glSetTexFiltering( glTextureFiltering x, unsigned tex = 0 );
      virtual glTextureWrapMode glTexWrapMode( unsigned coord = 0,
                                              unsigned tex = 0 ) const;
      virtual void glSetTexWrapMode( glTextureWrapMode x, unsigned coord = 0,
                                  unsigned tex = 0 );
      virtual void glSetTexRGBInterpolation( bool x, unsigned tex = 0 );
      virtual bool glTexRGBInterpolation( unsigned tex = 0 ) const;
      virtual glAutoTexturingMode glAutoTexMode( unsigned tex = 0 ) const;
      virtual void glSetAutoTexMode( glAutoTexturingMode mode,
                                  unsigned tex = 0 );
      virtual const float *glAutoTexParams( unsigned coord = 0,
                                          unsigned tex = 0 ) const;
      virtual void glSetAutoTexParams( const float* params, unsigned coord = 0,
                                      unsigned tex = 0 );

      const std::pair<const AimsSurface<4, Void>*, const std::vector<size_t> *>
      surfaceWithTexIndices( const ViewState & ) const;

    private:
      struct Private;
      Private *d;
    };

}

#endif
