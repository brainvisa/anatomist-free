namespace anatomist
{

class GLComponent
{
%TypeHeaderCode
#include <anatomist/object/Object.h>
#include <anatomist/mobject/ObjectList.h>
#include <anatomist/surface/glcomponent.h>
#include <pyanatomist/refglitem.h>
%End


public:
  enum glTextureMode
  {
    glGEOMETRIC,
    glLINEAR,
    glREPLACE,
    glDECAL,
    glBLEND,
    glADD,
    glCOMBINE,
    glLINEAR_ON_DEFINED,
  };

  enum glTextureFiltering
  {
    glFILT_NEAREST,
    glFILT_LINEAR,
  };

  enum glPart
  {
    glGENERAL,
    glBODY,
    glMATERIAL,
    glGEOMETRY,
    glPALETTE,
    glREFERENTIAL,
    glTEXIMAGE,
    glTEXENV,

    glNOPART,
    glTEXIMAGE_NUM,
    glTEXENV_NUM,
  };

  enum glAutoTexturingMode
  {
    glTEX_MANUAL,
    glTEX_OBJECT_LINEAR,
    glTEX_EYE_LINEAR,
    glTEX_SPHERE_MAP,
    glTEX_REFLECTION_MAP,
    glTEX_NORMAL_MAP,
  };


  class TexExtrema
  {
%TypeHeaderCode
#include <anatomist/surface/glcomponent.h>
#ifndef PYAIMSSIP_VECTOR_FLOAT_DEFINED
#define PYAIMSSIP_VECTOR_FLOAT_DEFINED
typedef std::vector<float> vector_FLOAT;
#endif
%End

  public:
    TexExtrema();
    virtual ~TexExtrema();

    vector_FLOAT	min;
    vector_FLOAT	max;
    vector_FLOAT	minquant;
    vector_FLOAT	maxquant;
    bool	scaled;
  };

  GLComponent();
  virtual ~GLComponent();

  virtual anatomist::GLComponent *glAPI() = 0;
  virtual const anatomist::GLComponent::TexExtrema &
    glTexExtrema(unsigned = 0) const /AutoGen/;

  virtual anatomist::GLPrimitives glMainGLL( const anatomist::ViewState & )
    /AutoGen/;

  virtual bool glMakeBodyGLL( const anatomist::ViewState & state,
                              const anatomist::GLList & gllist ) const /AutoGen/;
  virtual void glBeforeBodyGLL( const anatomist::ViewState & state,
                                anatomist::GLPrimitives & pl ) const /AutoGen/;
  virtual void glAfterBodyGLL( const anatomist::ViewState & state,
                               anatomist::GLPrimitives & pl ) const /AutoGen/;
  virtual bool glMakeTexImage( const anatomist::ViewState & state,
                              const anatomist::GLTexture & gltex,
                               unsigned tex ) const /AutoGen/;
  virtual bool glMakeTexEnvGLL( const anatomist::ViewState & state,
                                const anatomist::GLList & gllist,
                                unsigned tex ) const /AutoGen/;

  virtual unsigned glNumVertex( const anatomist::ViewState & ) const /AutoGen/;
  virtual unsigned glNumPolygon( const anatomist::ViewState & ) const /AutoGen/;
  virtual unsigned glNumTextures() const /AutoGen/;
  virtual unsigned glNumTextures( const anatomist::ViewState & ) const /AutoGen/;
  virtual anatomist::GLComponent::glTextureMode glTexMode
    ( unsigned tex = 0 ) const /AutoGen/;
  virtual void glSetTexMode( anatomist::GLComponent::glTextureMode mode,
                             unsigned tex = 0 ) /AutoGen/;
  virtual float glTexRate( unsigned tex = 0 ) const /AutoGen/;
  virtual void glSetTexRate( float rate, unsigned tex = 0 ) /AutoGen/;
  virtual anatomist::GLComponent::glTextureFiltering glTexFiltering
    ( unsigned tex = 0 ) const /AutoGen/;
  virtual void glSetTexFiltering( anatomist::GLComponent::glTextureFiltering x,
                                  unsigned tex = 0 ) /AutoGen/;
  virtual anatomist::GLComponent::glAutoTexturingMode glAutoTexMode
    ( unsigned tex = 0 ) const /AutoGen/;
  virtual void glSetAutoTexMode
    ( anatomist::GLComponent::glAutoTexturingMode mode, unsigned tex = 0 )
    /AutoGen/;
  virtual GLint glGLTexMode( unsigned tex = 0) const /AutoGen/;
  virtual GLint glGLTexFiltering( unsigned tex = 0 ) const /AutoGen/;
  virtual void glSetTexRGBInterpolation( bool x, unsigned tex = 0 ) /AutoGen/;
  virtual bool glTexRGBInterpolation( unsigned tex = 0 ) const /AutoGen/;
  virtual unsigned glDimTex( const anatomist::ViewState &,
                             unsigned tex = 0 ) const /AutoGen/;
  virtual unsigned glTexCoordSize( const anatomist::ViewState &,
                                   unsigned tex = 0 ) const /AutoGen/;
  virtual bool glAllowedTexRGBInterpolation( unsigned tex = 0 ) const;
  virtual bool glAllowedTexRate( unsigned tex = 0 ) const /AutoGen/;
  virtual std::string viewStateID( anatomist::GLComponent::glPart part,
    const anatomist::ViewState & ) const /AutoGen/;
  virtual carto::Object debugInfo() const /AutoGen/;
};

};

