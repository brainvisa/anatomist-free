

%ModuleHeaderCode
#if SIP_VERSION < 0x040700
#include "sipanatomistsiprc_ptr_GLItem.h"
#endif
%End

namespace anatomist
{

class GLItem : carto::RCObject
{
%TypeHeaderCode
#include <anatomist/primitive/primitive.h>
%End

%ConvertToSubClassCode
  if( dynamic_cast<anatomist::GLList *>( sipCpp ) )
    sipClass = sipClass_anatomist_GLList;
  else if( dynamic_cast<anatomist::GLTexture *>( sipCpp ) )
    sipClass = sipClass_anatomist_GLTexture;
  else if( dynamic_cast<anatomist::GLItemList *>( sipCpp ) )
  {
    if( dynamic_cast<anatomist::GLNoExecItemList *>( sipCpp ) )
      sipClass = sipClass_anatomist_GLNoExecItemList;
    else
      sipClass = sipClass_anatomist_GLItemList;
  }
  else if( dynamic_cast<anatomist::Primitive *>( sipCpp ) )
    sipClass = sipClass_anatomist_Primitive;
  else
    sipClass = NULL;
%End

public:
  GLItem();
  virtual ~GLItem();

  virtual void callList() const = 0;
  virtual bool ghost() const;
  virtual void setGhost( bool );
};


typedef rc_ptr_GLItem		RefGLItem;
typedef list_RefGLItem	GLPrimitives;


class GLList : anatomist::GLItem
{
%TypeHeaderCode
#include <anatomist/primitive/primitive.h>
%End

public:
  GLList( unsigned gllist = 0 );
  virtual ~GLList();

  unsigned item() const;
  virtual void generate();
  virtual void callList() const;
};


class GLTexture : anatomist::GLItem
{
%TypeHeaderCode
#include <anatomist/primitive/primitive.h>
%End

public:
  GLTexture( unsigned gltex = 0 );
  virtual ~GLTexture();

  unsigned item() const;
  virtual void generate();
  virtual void callList() const;
};


class GLItemList : anatomist::GLItem
{
%TypeHeaderCode
#include <anatomist/primitive/primitive.h>
%End

public:
  GLItemList();
  virtual ~GLItemList();
  virtual void callList() const;

  // list_RefGLItem	items;
};


class GLNoExecItemList : anatomist::GLItemList
{
%TypeHeaderCode
#include <anatomist/primitive/primitive.h>
%End

public:
  GLNoExecItemList();
  virtual ~GLNoExecItemList();
  virtual void callList();
};


class Primitive : anatomist::GLItem
{
%TypeHeaderCode
#include <anatomist/primitive/primitive.h>
%End

public:
  Primitive();
  virtual ~Primitive();
  const list_U32 & glLists() const;
  const list_U32 & textures() const;
  void insertList( unsigned );
  void insertTexture( unsigned );
  void deleteList( unsigned );
  void deleteTexture( unsigned );
  virtual void callList() const;
  void clear();
};

};
