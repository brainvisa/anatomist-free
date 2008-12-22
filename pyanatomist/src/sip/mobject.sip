namespace anatomist
{

class AIterator
{
%TypeHeaderCode
#include <anatomist/object/Object.h>
#include <anatomist/mobject/MObject.h>
#if SIP_VERSION < 0x040700
#include "sipanatomistsipanatomistAObject.h"
#endif
#include <pyanatomist/aobject.h>
%End

public:
    AIterator();

    AIterator( const anatomist::AIterator & );
    ~AIterator();
    bool operator == ( const anatomist::AIterator & ) const;
    bool operator != ( const anatomist::AIterator & ) const;

    anatomist::AObject * next();
%MethodCode
    sipRes = **sipCpp;
    ++(*sipCpp);
%End
};

class MObject : anatomist::AObject /Abstract/
{
%TypeHeaderCode
#include <anatomist/object/Object.h>
#include <anatomist/mobject/MObject.h>
#include <pyaims/vector/vector.h>
#if SIP_VERSION < 0x040700
#include "sipanatomistsipanatomistAIterator.h"
#include "sipanatomistsipanatomistAObject.h"
#endif
#include <pyanatomist/aobject.h>
%End

public:
    MObject();
    virtual ~MObject();

    virtual size_t size() const = 0;
    virtual void 	insert (anatomist::AObject *) = 0;
    virtual anatomist::AIterator begin() = 0;
    virtual anatomist::AIterator end() = 0;
    void erase( anatomist::AObject *);

    virtual float MinT() const /AutoGen/;
    virtual float MaxT() const /AutoGen/;

    virtual float MinX2D() const /AutoGen/;
    virtual float MinY2D() const /AutoGen/;
    virtual float MinZ2D() const /AutoGen/;
    virtual float MaxX2D() const /AutoGen/;
    virtual float MaxY2D() const /AutoGen/;
    virtual float MaxZ2D() const /AutoGen/;

    virtual Point3df VoxelSize() const /AutoGen/;
    virtual bool boundingBox( Point3df &, Point3df & ) const /AutoGen/;
    virtual anatomist::AObject* ObjectAt( float, float, float, float, float = 0 ) /AutoGen/;
    virtual bool Is2DObject() /AutoGen/;
    virtual bool Is3DObject() /AutoGen/;
    virtual bool isTransparent() const /AutoGen/;
    virtual void update( const anatomist::Observable*, void* ) /AutoGen/;
    virtual bool CanRemove( anatomist::AObject * ) /AutoGen/;
    virtual void setContentChanged() const /AutoGen/;
    bool hasContentChanged() const /AutoGen/;
    virtual void clearHasChangedFlags() const /AutoGen/;
    virtual void setPalette( const anatomist::AObjectPalette & ) /AutoGen/;

    anatomist::AIterator * __iter__() /AutoGen, TransferBack/;
%MethodCode
    sipRes = new anatomist::AIterator(sipCpp->begin());
%End

    anatomist::AObject * __getitem__(int) /AutoGen/;
%MethodCode
    if ( a0 < (int) sipCpp->size())
    {
      anatomist::AIterator current = sipCpp->begin();

      for(int index=0; (index < a0); index++)
        current++;

      sipRes = *current;
    }
    else
    {
      sipRes = 0;
    }
%End
};

};

