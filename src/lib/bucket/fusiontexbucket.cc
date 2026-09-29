#include <anatomist/bucket/fusiontexbucket.h>
#include <anatomist/bucket/texbucket.h>
#include <anatomist/bucket/Bucket.h>
#include <anatomist/window/viewstate.h>
#include <qobject.h>

using namespace anatomist;
using namespace std;


string FusionTexBucketMethod::ID() const
{
  return( QT_TRANSLATE_NOOP( "FusionChooser", "FusionTexBucketMethod" ) );
}


string FusionTexBucketMethod::generatedObjectType() const
{
  return AObject::objectTypeName( AObject::TEXSURFACE );
}


int FusionTexBucketMethod::canFusion( const set<AObject *> & obj )
{
  if( obj.size() != 2 )
    return 0;

  AObject *ao1 = 0, *ao2 = 0;
  GLComponent *o2;
  set<AObject *>::const_iterator	io;

  io = obj.begin();
  if( dynamic_cast<Bucket *>( *io ) )
    ao1 = *io;
  else
  {
    o2 = (*io)->glAPI();
    if( !o2 || o2->glNumTextures() == 0 )
      return 0;
    ao2 = *io;
  }
  ++io;
  if( !ao1 )
  {
    if( dynamic_cast<Bucket *>( *io ) )
      ao1 = *io;
    else
      return 0; // no bucket
  }
  else
  {
    o2 = (*io)->glAPI();
    if( !o2 || o2->glNumTextures() == 0 )
      return 0;
    ao2 = *io;
  }

  return 150;

  return 0;
}


AObject* FusionTexBucketMethod::fusion( const vector<AObject *> & obj )
{
  vector<AObject *>::const_iterator io = obj.begin();
  AObject *o1 = *io;

  ++io;

  return new ATexBucket( o1, *io );
}
