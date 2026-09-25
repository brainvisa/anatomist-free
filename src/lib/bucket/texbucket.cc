
#include <anatomist/bucket/texbucket.h>
#include <anatomist/bucket/Bucket.h>
#include <aims/mesh/surface.h>


using namespace anatomist;
using namespace aims;
using namespace std;


struct ATexBucket::Private
{
};


ATexBucket::ATexBucket( AObject *o1, AObject* o2 )
  : GLObjectVector(), d( new Private )
{
  if( !dynamic_cast<Bucket *>( o1 ) )
  {
    AObject *tmp = o1;
    o1 = o2;
    o2 = tmp;
  }
  insert( o1 );
  insert( o2 );
  setReferentialInheritance( o1 );
}


ATexBucket::~ATexBucket()
{
  delete d;
}


unsigned ATexBucket::glNumVertex( const ViewState & vs ) const
{
  return surfaceWithTexIndices( vs ).first->vertex().size();
}


const GLfloat* ATexBucket::glVertexArray( const ViewState & vs ) const
{
  return &surfaceWithTexIndices( vs ).first->vertex()[0][0];
}


const GLfloat* ATexBucket::glNormalArray( const ViewState & vs ) const
{
  return &surfaceWithTexIndices( vs ).first->normal()[0][0];
}


unsigned ATexBucket::glPolygonSize( const ViewState & vs ) const
{
  return 4;
}


unsigned ATexBucket::glNumPolygon( const ViewState & vs ) const
{
  return surfaceWithTexIndices( vs ).first->polygon().size();
}


const GLuint* ATexBucket::glPolygonArray( const ViewState & vs ) const
{
  return &surfaceWithTexIndices( vs ).first->polygon()[0][0];
}


unsigned ATexBucket::glNumTextures() const
{
  return 0;
  // return (begin()++)->glNumTextures();
}


unsigned ATexBucket::glNumTextures( const ViewState & ) const
{
  return 0;
}


GLComponent::glTextureMode ATexBucket::glTexMode( unsigned tex ) const
{
  return (*(begin()++))->glAPI()->glTexMode( tex );
}


void ATexBucket::glSetTexMode( glTextureMode mode, unsigned tex )
{
  (*(begin()++))->glAPI()->glSetTexMode( mode, tex );
}


float ATexBucket::glTexRate( unsigned tex ) const
{
  return (*(begin()++))->glAPI()->glTexRate( tex );
}


void ATexBucket::glSetTexRate( float rate, unsigned tex )
{
  (*(begin()++))->glAPI()->glSetTexRate( rate, tex );
}


GLComponent::glTextureFiltering ATexBucket::glTexFiltering(
  unsigned tex ) const
{
  return (*(begin()++))->glAPI()->glTexFiltering( tex );
}


void ATexBucket::glSetTexFiltering( glTextureFiltering x, unsigned tex )
{
  (*(begin()++))->glAPI()->glSetTexFiltering( x, tex );
}


GLComponent::glTextureWrapMode ATexBucket::glTexWrapMode( unsigned coord,
                                                          unsigned tex ) const
{
  return (*(begin()++))->glAPI()->glTexWrapMode( coord, tex );
}


void ATexBucket::glSetTexWrapMode( glTextureWrapMode x, unsigned coord,
                                   unsigned tex )
{
  (*(begin()++))->glAPI()->glSetTexWrapMode( x, coord, tex );
}


void ATexBucket::glSetTexRGBInterpolation( bool x, unsigned tex )
{
  (*(begin()++))->glAPI()->glSetTexRGBInterpolation( x, tex );
}


bool ATexBucket::glTexRGBInterpolation( unsigned tex ) const
{
  return (*(begin()++))->glAPI()->glTexRGBInterpolation( tex );
}


GLComponent::glAutoTexturingMode ATexBucket::glAutoTexMode(
  unsigned tex ) const
{
  return (*(begin()++))->glAPI()->glAutoTexMode( tex );
}


void ATexBucket::glSetAutoTexMode( glAutoTexturingMode mode, unsigned tex )
{
  (*(begin()++))->glAPI()->glSetAutoTexMode( mode, tex );
}


const float *ATexBucket::glAutoTexParams( unsigned coord, unsigned tex ) const
{
  return (*(begin()++))->glAPI()->glAutoTexParams( coord, tex );
}


void ATexBucket::glSetAutoTexParams( const float* params, unsigned coord,
                                     unsigned tex )
{
  (*(begin()++))->glAPI()->glSetAutoTexParams( params, coord, tex );
}


const std::pair<const AimsSurface<4, Void>*, const std::vector<size_t> *>
ATexBucket::surfaceWithTexIndices( const ViewState & vs ) const
{
  Bucket *abk = static_cast<Bucket *>( *begin() );
  return abk->surfaceWithTexIndices( vs );
}

