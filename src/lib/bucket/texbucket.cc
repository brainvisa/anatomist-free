
#include <anatomist/bucket/texbucket.h>
#include <anatomist/bucket/Bucket.h>
#include <aims/mesh/surface.h>


using namespace anatomist;
using namespace aims;
using namespace std;


struct ATexBucket::Private
{
  vector<GLfloat> texcoords; // FIXME should not be here
};


ATexBucket::ATexBucket( AObject *o1, AObject* o2 )
  : GLObjectVector(), d( new Private )
{
  _type = AObject::TEXBUCKET;

  if( !dynamic_cast<Bucket *>( o1 ) )
  {
    AObject *tmp = o1;
    o1 = o2;
    o2 = tmp;
  }
  insert( o1 );
  insert( o2 );
  setReferentialInheritance( o1 );
  unsigned	ntex = o2->glAPI()->glNumTextures();
  glAddTextures( ntex );
  for( unsigned tex=0; tex<ntex; ++tex )
  {
    TexExtrema  & te = GLComponent::glTexExtrema( tex );
    te.min.push_back( 0 );
    te.max.push_back( 0 );
    te.minquant.push_back( 0 );
    te.maxquant.push_back( 0 );
  }
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
  return (*(++begin()))->glAPI()->glNumTextures();
}


unsigned ATexBucket::glNumTextures( const ViewState & vs ) const
{
  return (*(++begin()))->glAPI()->glNumTextures( vs );
}


GLComponent::glTextureMode ATexBucket::glTexMode( unsigned tex ) const
{
  return (*(++begin()))->glAPI()->glTexMode( tex );
}


void ATexBucket::glSetTexMode( glTextureMode mode, unsigned tex )
{
  (*(++begin()))->glAPI()->glSetTexMode( mode, tex );
}


float ATexBucket::glTexRate( unsigned tex ) const
{
  return (*(++begin()))->glAPI()->glTexRate( tex );
}


void ATexBucket::glSetTexRate( float rate, unsigned tex )
{
  (*(++begin()))->glAPI()->glSetTexRate( rate, tex );
}


GLComponent::glTextureFiltering ATexBucket::glTexFiltering(
  unsigned tex ) const
{
  return (*(++begin()))->glAPI()->glTexFiltering( tex );
}


void ATexBucket::glSetTexFiltering( glTextureFiltering x, unsigned tex )
{
  (*(++begin()))->glAPI()->glSetTexFiltering( x, tex );
}


GLComponent::glTextureWrapMode ATexBucket::glTexWrapMode( unsigned coord,
                                                          unsigned tex ) const
{
  return (*(++begin()))->glAPI()->glTexWrapMode( coord, tex );
}


void ATexBucket::glSetTexWrapMode( glTextureWrapMode x, unsigned coord,
                                   unsigned tex )
{
  (*(++begin()))->glAPI()->glSetTexWrapMode( x, coord, tex );
}


void ATexBucket::glSetTexRGBInterpolation( bool x, unsigned tex )
{
  (*(++begin()))->glAPI()->glSetTexRGBInterpolation( x, tex );
}


bool ATexBucket::glTexRGBInterpolation( unsigned tex ) const
{
  return (*(++begin()))->glAPI()->glTexRGBInterpolation( tex );
}


GLComponent::glAutoTexturingMode ATexBucket::glAutoTexMode(
  unsigned tex ) const
{
  return (*(++begin()))->glAPI()->glAutoTexMode( tex );
}


void ATexBucket::glSetAutoTexMode( glAutoTexturingMode mode, unsigned tex )
{
  (*(++begin()))->glAPI()->glSetAutoTexMode( mode, tex );
}


const float *ATexBucket::glAutoTexParams( unsigned coord, unsigned tex ) const
{
  return (*(++begin()))->glAPI()->glAutoTexParams( coord, tex );
}


void ATexBucket::glSetAutoTexParams( const float* params, unsigned coord,
                                     unsigned tex )
{
  (*(++begin()))->glAPI()->glSetAutoTexParams( params, coord, tex );
}


const std::pair<const AimsSurface<4, Void>*, const std::vector<size_t> *>
ATexBucket::surfaceWithTexIndices( const ViewState & vs ) const
{
  Bucket *abk = static_cast<Bucket *>( *begin() );
  return abk->surfaceWithTexIndices( vs );
}


unsigned ATexBucket::glDimTex( const ViewState & vs, unsigned tex ) const
{
  return (*(++begin()))->glAPI()->glDimTex( vs, tex );
}


unsigned ATexBucket::glTexCoordSize( const ViewState & vs, unsigned tex ) const
{
  return surfaceWithTexIndices( vs ).second->size();
}


const GLfloat* ATexBucket::glTexCoordArray( const ViewState & vs, unsigned tex ) const
{
  const GLfloat *coords = (*(++begin()))->glAPI()->glTexCoordArray( vs, tex );
  auto si = *surfaceWithTexIndices( vs ).second;
  unsigned dimtex = glDimTex( vs, tex );
  d->texcoords.resize( si.size() * dimtex );
  size_t rs = (*(++begin()))->glAPI()->glTexCoordSize( vs, tex );
  for( size_t i=0; i<si.size(); ++i )
    for( unsigned j=0; j<dimtex; ++j )
      d->texcoords[i * dimtex + j] = coords[si[i] * dimtex + j];
  return &d->texcoords[0];
}

