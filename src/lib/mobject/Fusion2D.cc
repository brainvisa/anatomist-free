/* Copyright (c) 1995-2005 CEA
 *
 *  This software and supporting documentation were developed by
 *      CEA/DSV/SHFJ
 *      4 place du General Leclerc
 *      91401 Orsay cedex
 *      France
 *
 * This software is governed by the CeCILL license version 2 under 
 * French law and abiding by the rules of distribution of free software.
 * You can  use, modify and/or redistribute the software under the 
 * terms of the CeCILL license version 2 as circulated by CEA, CNRS
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
 * knowledge of the CeCILL license version 2 and that you accept its terms.
 */

#include <anatomist/mobject/Fusion2D.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/object/actions.h>
#include <anatomist/window/Window.h>
#include <anatomist/window/viewstate.h>
#include <anatomist/misc/error.h>
#include <anatomist/reference/Transformation.h>
#include <anatomist/reference/transformobserver.h>
#include <anatomist/reference/Referential.h>
#include <graph/tree/tree.h>
#include <cartobase/object/pythonwriter.h>
#include <qtranslator.h>
#include <math.h>


using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;


Tree*	Fusion2D::_optionTree = 0;


Fusion2D::Fusion2D( const vector<AObject *> & obj )
  : ObjectVector(), Sliceable()
{
  _type = AObject::FUSION2D;

  vector<AObject *>::const_iterator	io, fo=obj.end();

  for( io=obj.begin(); io!=fo; ++io )
    insert( shared_ptr<AObject>( shared_ptr<AObject>::Strong, *io ) );

  unsigned	i, n = obj.size();
  glAddTextures( n );
  glSetTexMode( glDECAL );
  for( i=1; i<n; ++i )
    {
      glSetTexRate( 1. / (n-i+1), i );
      glSetTexMode( glLINEAR, i );
    }
  setReferentialInheritance( *begin() );
}


Fusion2D::~Fusion2D()
{
  cleanupObserver();
  cleanup();
}


void Fusion2D::moveVolume( AObject* vol, int dstpos )
{
  datatype::iterator	ii, e = _data.end();
  int	i = 0;

  for( ii=_data.begin(); ii!=e && *ii!=vol; ++ii, ++i ) {}
  if( ii == e )
    return;

  shared_ptr<AObject> tmp = *ii;
  _data.erase( ii );
  if( dstpos < 0 )
    _data.push_back( tmp );
  else
    {
      for( ii=_data.begin(), i=0; i<dstpos && ii!=e; ++ii, ++i ) {}
      _data.insert( ii, tmp );
    }

  glSetTexImageChanged( true, 0 );
}


void Fusion2D::moveVolume( int srcpos, int dstpos )
{
  // cout << "Fusion2D::moveVolume " << srcpos << " -> " << dstpos << endl;
  datatype::iterator	ii, e = _data.end();
  int	i = 0;

  for( ii=_data.begin(); i<srcpos && ii!=e; ++ii, ++i ) {}
  if( ii == e )
    return;

  shared_ptr<AObject> tmp = *ii;
  _data.erase( ii );
  if( dstpos < 0 )
    _data.push_back( tmp );
  else
    {
      for( ii=_data.begin(), i=0; i<dstpos && ii!=e; ++ii, ++i ) {}
      _data.insert( ii, tmp );
    }

  glSetTexImageChanged( true, 0 );
}


void Fusion2D::reorder( const vector<AObject *> & ord )
{
  // check
  unsigned			i, n = ord.size();
  if( n != _data.size() )
  {
    cerr << "Fusion2D reorder: mismatching number of objects ("
          << n << ", expecting " << _data.size() << ")" << endl;
    return;
  }
  vector<shared_ptr<AObject> >  reord( ord.size() );
  map<AObject *, unsigned>  o;
  map<AObject *, unsigned>::iterator ioo, notfound = o.end();

  for( i=0; i<n; ++i )
    o[ ord[i] ] = i;

  datatype::iterator	io, eo = _data.end();
  for( io=_data.begin(); io!=eo; ++io )
  {
    ioo = o.find( io->get() );
    if( ioo == notfound )
    {
      cerr << "Fusion2D reorder: objects list do not match the fusionned "
            << "objects" << endl;
      return;
    }
    reord[ ioo->second ] = *io;
  }

  // now reorder
  _data.clear();
  for( i=0; i<n; ++i )
    _data.push_back( reord[i] );
}


void Fusion2D::update( const Observable* observable, void* arg )
{
  // cout << "Fusion2D::update\n";

  AObject::update( observable, arg );

  const AObject *o = dynamic_cast<const AObject *>( observable );
  datatype::iterator	iobj;
  if( o )
    iobj  = ::find( _data.begin(), _data.end(), o );

  if( !o || iobj == _data.end() )
  {
    const TransformationObserver
      *to = dynamic_cast<const TransformationObserver *>( observable );
    if( !to )
      return;

    //cout << "Transformation changed in Fusion2D\n";
    glSetTexImageChanged( true, 0 );
    notifyObservers((void*)this);
    return;
  }

  const GLComponent	*c = o->glAPI();

  if( !c )
    return;
  if( o->obsHasChanged( glTEXIMAGE ) )
    glSetTexImageChanged( true, 0 );
  if( o->obsHasChanged( glBODY ) )
    glSetChanged( glBODY, true );

  updateSubObjectReferential( o );
  notifyObservers((void*)this);
}


namespace
{

  template<int T> inline
  void _mix_item( unsigned char* src, unsigned char *dst, float rate );

  // geometric
  template<> inline
  void _mix_item<0>( unsigned char* src, unsigned char *dst, float )
  {
    *dst = (unsigned char) sqrt( ((float) *src) * ((float) *dst) );
    *(dst+1) = (unsigned char) 
      rint( sqrt( ((float) *(src+1)) * ((float) *(dst+1)) ) );
    *(dst+2) = (unsigned char) 
      rint( sqrt( ((float) *(src+2)) * ((float) *(dst+2)) ) );
    *(dst+3) = (unsigned char) 
      rint( sqrt( ((float) *(src+3)) * ((float) *(dst+3)) ) );
  }


  // linear
  template<> inline
  void _mix_item<1>( unsigned char* src, unsigned char *dst, float rate )
  {
    *dst = (unsigned char) rint( rate* (*src) + (1.-rate)*(*dst) );
    *(dst+1) = (unsigned char) rint( rate*(*(src+1)) + (1.-rate)*(*(dst+1)) );
    *(dst+2) = (unsigned char) rint( rate*(*(src+2)) + (1.-rate)*(*(dst+2)) );
    *(dst+3) = (unsigned char) rint( rate*(*(src+3)) + (1.-rate)*(*(dst+3)) );
  }


  // linear-on-defined
  template<> inline
  void _mix_item<2>( unsigned char* src, unsigned char *dst, float rate )
  {
    if( *src != 255 || *(src+1) != 255 || *(src+2) != 255 )
      _mix_item<1>( src, dst, rate );
  }


  // add
  template<> inline
  void _mix_item<3>( unsigned char* src, unsigned char *dst, float rate )
  {
    float	x = rate* (*src) + *dst;
    *dst = ( x >= 255 ? 255 : (unsigned char) rint( x ) );
    x = rate*(*(src+1)) + *(dst+1);
    *(dst+1) = ( x >= 255 ? 255 : (unsigned char) rint( x ) );
    x = rate*(*(src+2)) + *(dst+2);
    *(dst+2) = ( x >= 255 ? 255 : (unsigned char) rint( x ) );
    x = rate*(*(src+3)) + *(dst+3);
    *(dst+3) = ( x >= 255 ? 255 : (unsigned char) rint( x ) );
  }


  template<int T> 
  void _mix2( unsigned char *dst, unsigned char *src, unsigned w, 
              unsigned h, unsigned offset_xim, float rate )
  {
    // cout << "_mix2 " << T << endl;
    unsigned	i, j;
    for( j=0; j<h; ++j )
      {
        for( i=0; i<w; ++i )
          {
            _mix_item<T>( src, dst, rate );
            dst += 4;
            src += 4;
          }
        dst += offset_xim;
      }
  }


  void _mix( int mode, unsigned char *dst, unsigned char *src, unsigned w, 
             unsigned h, unsigned offset_xim, float rate )
  {
    switch( mode )
      {
      case GLComponent::glLINEAR:
        _mix2<1>( dst, src, w, h, offset_xim, rate );
        break;
      case GLComponent::glADD:
        _mix2<3>( dst, src, w, h, offset_xim, rate );
        break;
      case GLComponent::glLINEAR_ON_DEFINED:
        _mix2<2>( dst, src, w, h, offset_xim, rate );
        break;
      default:
        _mix2<0>( dst, src, w, h, offset_xim, rate );
      }
  }

}


bool Fusion2D::render( PrimList & prim, const ViewState & state )
{
  return AObject::render( prim, state );
}


bool Fusion2D::update2DTexture( AImage & ximage, const Point3df & pos, 
                                const SliceViewState & state, 
                                unsigned ) const
{
  const Referential	*winref = state.winref;

  /*
  cout << "Fusion2D::update2DTexture\n";
  cout << name() << " state:" << endl;
  Object	o = debugInfo();
  PythonWriter	pw;
  pw.attach( cout );
  try
    {
      pw.write( *o );
    }
  catch( ... )
    {
    }
  */

  int size = ximage.width * ximage.height * ximage.depth / 8;

  //cout << "im size : " << ximage.width << " x " << ximage.height << endl;
  if( (winref == NULL) && ( (mri()->getReferential() != NULL) 
                            || (functional()->getReferential() != NULL) ) )
  winref = getReferential();
  if( !winref )
  {
    cout << "Fusion2D::update2DTexture: no referential\n";
    return false;
  }

  char *functionalImage = new char[size];

  datatype::const_reverse_iterator	io=_data.rbegin(), fo=_data.rend();
  unsigned			w = ximage.width, h = ximage.height;
  unsigned			offset_xim 
    = ( ximage.effectiveWidth - w ) * ximage.depth / 8;
  AImage			fimage;
  unsigned			tex = _data.size() - 1;

  fimage.width = w;
  fimage.height = h;
  fimage.effectiveWidth = w;
  fimage.effectiveHeight = h;
  fimage.depth = ximage.depth;
  fimage.data = functionalImage;

  // begin with last image
  // cout << "Fusion2D: updating 1st image...\n";
  (*io)->glAPI()->sliceableAPI()->update2DTexture( fimage, pos, state );
  // cout << "passed\n";

  --fo; // stop after second image, the first one is a bit different
  fimage.data = ximage.data;

  // mix other images from end to start
  for( ++io; io!=fo; ++io, --tex )
    {
      //cout << "Fusion2D: updating other image...\n";
      (*io)->glAPI()->sliceableAPI()->update2DTexture( fimage, pos, state );
      // merge with previous images
      //cout << "mode " << tex << ": " << glTexMode( tex ) << endl;

     _mix( glTexMode( tex ), (unsigned char *) functionalImage, 
            (unsigned char *) ximage.data, w, h, 0, glTexRate( tex ) );
    }
  fimage.data = 0;

  // mix first image
  (*io)->glAPI()->sliceableAPI()->update2DTexture( ximage, pos, state );
  /*
  cout << "vol mode: " << (*io)->glAPI()->sliceableAPI()->glTexMode() << endl;
  cout << "mode 1: " << glTexMode( 1 ) << endl;
  */
  // merge with previous images
  _mix( glTexMode( 1 ), (unsigned char *) ximage.data, 
        (unsigned char *) functionalImage, w, h, offset_xim, 
        1. - glTexRate( 1 ) );
  //cout << "OK\n";

  delete[] functionalImage;
  return true;
}


Tree* Fusion2D::optionTree() const
{
  if( !_optionTree )
    {
      Tree	*t, *t2;
      _optionTree = new Tree( true, "option tree" );
      t = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu", "File" ) );
      _optionTree->insert( t );
      t2 = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu", 
					      "Rename object" ) );
      t2->setProperty( "callback", &ObjectActions::renameObject );
      t->insert( t2 );

      t = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu", "Color" ) );
      _optionTree->insert( t );
      t2 = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu", "Material" ) );
      t2->setProperty( "callback", &ObjectActions::colorMaterial );
      t->insert( t2 );
      t2 = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu", "Texturing" ) );
      t2->setProperty( "callback", &ObjectActions::textureControl );
      t->insert( t2 );

      t = new Tree( true, "Referential" );
      _optionTree->insert( t );
      t2 = new Tree( true, "Load" );
      t2->setProperty( "callback", &ObjectActions::referentialLoad );
      t->insert( t2 );

      t = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu", "Fusion" ) );
      _optionTree->insert( t );
      t2 = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu", 
					      "Control 2D fusion" ) );
      t2->setProperty( "callback", &ObjectActions::fusion2DControl );
      t->insert( t2 );
    }
  return( _optionTree );
}


Point3df Fusion2D::VoxelSize() const
{
  Point3df			vs, vs2;
  datatype::const_iterator	io, fo=_data.end();

  io = _data.begin();
  vs = (*io)->VoxelSize();

  for( ++io; io!=fo; ++io )
    {
      vs2 = (*io)->VoxelSize();
      if( vs2[0] < vs[0] )
	vs[0] = vs2[0];
      if( vs2[1] < vs[1] )
	vs[1] = vs2[1];
      if( vs2[2] < vs[2] )
	vs[2] = vs2[2];
    }
  return( vs );
}


Point3df Fusion2D::glVoxelSize() const
{
  return VoxelSize();
}


Point4df Fusion2D::glMin2D() const
{
  return Point4df( MinX2D(), MinY2D(), MinZ2D(), MinT() );
}


Point4df Fusion2D::glMax2D() const
{
  return Point4df( MaxX2D(), MaxY2D(), MaxZ2D(), MaxT() );
}


vector<float> Fusion2D::texValues( const Point3df & pos, float time ) const
{
  vector<float>			tv( _data.size() );
  datatype::const_iterator	io=_data.begin(), fo=_data.end();
  unsigned			i = 1;

  //	assume MRI referential...
  tv[0] = (*io)->mixedTexValue( pos, time );

  Referential	*ref = (*io)->getReferential();
  Point3df	vs = (*io)->VoxelSize();

  if( ref )
    for( ++io; io!=fo; ++io, ++i )
      tv[i] = (*io)->mixedTexValue( pos, time, ref, vs );
  else
    for( ++io; io!=fo; ++io, ++i )
      tv[i] = (*io)->mixedTexValue( pos, time );

  return( tv );
}


vector<float> Fusion2D::texValues( const Point3df & pos, float time, 
				   const Referential* orgRef, 
				   const Point3df & orgVoxSz ) const
{
  unsigned			i;
  vector<float>			tv( _data.size() );
  datatype::const_iterator	io, fo=_data.end();

  for( i = 0, io=_data.begin(); io!=fo; ++io, ++i )
    tv[i] = (*io)->mixedTexValue( pos, time, orgRef, orgVoxSz );

  return( tv );
}


float Fusion2D::mixedTexValue( const Point3df & pos, float time ) const
{
  return( mixedValue( texValues( pos, time ) ) );
}


float Fusion2D::mixedTexValue( const Point3df & pos, float time, 
			       const Referential* orgRef, 
			       const Point3df & orgVoxSz ) const
{
  return( mixedValue( texValues( pos, time, orgRef, orgVoxSz ) ) );
}


float Fusion2D::mixedValue( const vector<float> & tv ) const
{
  int	i, n = _data.size() - 1;
  float		val = tv[n], r;

  for( i=n-1; i>=0; --i )
    switch( glTexMode( i ) )
      {
      case GLComponent::glLINEAR:
        r = glTexRate( i );
        val = (1.-r) * tv[i] + r * val;
        break;
      case GLComponent::glCOMBINE:
        if( tv[i] )
        {
          r = glTexRate( i );
          val = (1.-r) * tv[i] + r * val;
        }
        else
          val = tv[i];
        break;
      default:
        val = sqrt( val * tv[i] );
      }
  return val;
}


const AObjectPalette* Fusion2D::glPalette( unsigned tex ) const
{
  // cout << "Fusion2D::glPalette " << tex << endl;
  unsigned		i;
  const_iterator	io;
  for( io=begin(), i=0; i<tex; ++i, ++io ) {}
  return (*io)->glAPI()->glPalette( 0 );
}


void Fusion2D::glClearHasChangedFlags() const
{
  ObjectVector::clearHasChangedFlags();
  Sliceable::glClearHasChangedFlags();
}


void Fusion2D::glSetChanged( glPart part, bool x ) const
{
  Sliceable::glSetChanged( part, x );
  obsSetChanged( part, x );
}


void Fusion2D::glSetTexImageChanged( bool x, unsigned tex ) const
{
  Sliceable::glSetTexImageChanged( x, tex );
  obsSetChanged( glTEXIMAGE_NUM + tex * 2, x );
  if( x && tex != 0 )
    {
      Sliceable::glSetTexImageChanged( true, 0 );
      obsSetChanged( glTEXIMAGE_NUM, true );
    }
}


void Fusion2D::glSetTexEnvChanged( bool x, unsigned tex ) const
{
  Sliceable::glSetTexEnvChanged( x, tex );
  obsSetChanged( glTEXENV_NUM + tex * 2, x );
  if( x && tex != 0 )
    {
      Sliceable::glSetTexEnvChanged( true, 0 );
      obsSetChanged( glTEXENV_NUM, true );
    }
}


const Material *Fusion2D::glMaterial() const
{
  return &material();
}


unsigned Fusion2D::glNumTextures( const ViewState & ) const
{
  return 1;
}


set<GLComponent::glTextureMode> 
Fusion2D::glAllowedTexModes( unsigned tex ) const
{
  if( tex == 0 )
    return GLComponent::glAllowedTexModes( 0 );

  set<glTextureMode>	a;
  a.insert( glGEOMETRIC );
  a.insert( glLINEAR );
  a.insert( glADD );
  a.insert( glLINEAR_ON_DEFINED );
  return a;
}


AObject* Fusion2D::fallbackReferentialInheritance() const
{
  return mri();
}


const Referential* Fusion2D::getReferential() const
{
  return ObjectVector::getReferential();
}


