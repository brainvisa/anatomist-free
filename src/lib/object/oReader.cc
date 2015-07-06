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


#include <cstdlib>
#include <anatomist/object/oReader.h>
#include <aims/io/process.h>
#include <aims/io/reader.h>
#include <aims/io/finder.h>
#include <aims/mesh/texture.h>
#include <anatomist/volume/Volume.h>
#include <anatomist/bucket/Bucket.h>
#include <anatomist/graph/Graph.h>
#include <anatomist/surface/triangulated.h>
#include <anatomist/surface/texture.h>
#include <anatomist/reference/Referential.h>
#include <anatomist/reference/Transformation.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/application/globalConfig.h>
#include <anatomist/fusion/fusionFactory.h>
#include <anatomist/color/objectPalette.h>
#include <anatomist/color/paletteList.h>
#include <anatomist/object/actions.h>
#include <anatomist/object/loadevent.h>
#include <anatomist/sparsematrix/sparsematrix.h>
#include <aims/data/pheader.h>
#include <aims/utility/converter_hsv.h>
#include <aims/utility/converter_rgb.h>
#include <aims/utility/converter_texture.h>
#include <aims/utility/converter_volume.h>
#include <aims/def/path.h>
#include <aims/io/fileFormat.h>
#include <aims/io/readerasobject.h>
#include <aims/sparsematrix/sparseordensematrix.h>
#include <soma-io/io/formatdictionary.h>
#include <soma-io/datasourceinfo/datasourceinfoloader.h>
#include <cartobase/stream/fileutil.h>
#include <qapplication.h>
#include <time.h>
#include <zlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>


using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;


//	default reader

ObjectReader * ObjectReader::reader()
{
  static ObjectReader *_theReader = 0;
  if( _theReader == 0 )
    _theReader = new ObjectReader;
  return _theReader;
}


ObjectReader::ObjectReader()
{
}


ObjectReader::~ObjectReader()
{
}


namespace
{

  template<class T> 
  bool loadVolume( Process & p, const string & fname, Finder & f );
  template<>
  bool loadVolume<AimsHSV>( Process & p, const string & fname, Finder & f );
  template<class T>
  bool loadData( T & obj, const string & fname, Finder & f, Object options );
  template<long D>
  bool loadMesh( Process & p, const string & fname, Finder & f );
  template<long D, typename T>
  bool loadTexturedMesh( Process & p, const string & fname, Finder & f );
  bool loadTex1d( Process & p, const string & fname, Finder & f );
  bool loadTex2d( Process & p, const string & fname, Finder & f );
  template<typename T>
  bool loadTex1dInt( Process & p, const string & fname, Finder & f );
  template<typename T>
  bool loadTex2dInt( Process & p, const string & fname, Finder & f );
  bool loadBucket( Process & p, const string & fname, Finder & f );
  bool loadGraph( Process & p, const string & fname, Finder & f );
  bool loadSparseMatrix( Process & p, const string & fname, Finder & f );


  class AimsLoader : public Process
  {
  public:
    AimsLoader( Object opts ) : Process(), object( 0 ), options( opts )
    {
      Object	restricted;
      bool	restr = false;
      if( !options.isNull() )
        try
          {
            restricted = options->getProperty( "restrict_object_types" );
            restr = true;
          }
        catch( ... )
          {
          }

      bool	r2 = false;
      Object	rtypes;

      set<string>			types;
      set<string>::const_iterator	eir = types.end();
      if( restr )
        try
          {
            rtypes = restricted->getProperty( "Volume" );
            Object	i = rtypes->objectIterator();
            for( ; i->isValid(); i->next() )
              types.insert( i->currentValue()->getString() );
            r2 = false;
          }
        catch( ... )
          {
            r2 = true;
          }
      bool	vols16 = false;
      if( !r2 )
        {
          if( !restr || types.find( "S8" ) != eir )
            registerProcessType( "Volume", "S8", &loadVolume<int8_t> );
          if( !restr || types.find( "U8" ) != eir )
            registerProcessType( "Volume", "U8", &loadVolume<uint8_t> );
          if( !restr || types.find( "S16" ) != eir )
            {
              registerProcessType( "Volume", "S16", &loadVolume<int16_t> );
              vols16 = true;
            }
          if( !restr || types.find( "U16" ) != eir )
            registerProcessType( "Volume", "U16", &loadVolume<uint16_t> );
          if( !restr || types.find( "S32" ) != eir )
            registerProcessType( "Volume", "S32", &loadVolume<int32_t> );
          if( !restr || types.find( "U32" ) != eir )
            registerProcessType( "Volume", "U32", &loadVolume<uint32_t> );
          if( !restr || types.find( "FLOAT" ) != eir )
            registerProcessType( "Volume", "FLOAT", &loadVolume<float> );
          if( !restr || types.find( "DOUBLE" ) != eir )
            registerProcessType( "Volume", "DOUBLE", &loadVolume<double> );
          if( !restr || types.find( "RGB" ) != eir )
            registerProcessType( "Volume", "RGB", &loadVolume<AimsRGB> );
          if( !restr || types.find( "RGBA" ) != eir )
            registerProcessType( "Volume", "RGBA", &loadVolume<AimsRGBA> );
          if( !restr || types.find( "HSV" ) != eir )
            registerProcessType( "Volume", "HSV", &loadVolume<AimsHSV> );
        }

      r2 = restr && !restricted->hasProperty( "Segments" );
      if( !r2 )
      {
        if( restr )
          try
            {
              types.clear();
              rtypes = restricted->getProperty( "Segments" );
              Object	i = rtypes->objectIterator();
              for( ; i->isValid(); i->next() )
                types.insert( i->getString() );
            }
          catch( ... )
            {
            }
        if( !restr || types.find( "VOID" ) != eir )
          registerProcessType( "Segments", "VOID", &loadMesh<2> );
        if( !restr || types.find( "FLOAT" ) != eir )
          registerProcessType( "Segments", "FLOAT",
                               &loadTexturedMesh<2, float> );
        if( !restr || types.find( "POINT2DF" ) != eir )
          registerProcessType( "Segments", "POINT2DF",
                               &loadTexturedMesh<2, Point2df> );
      }
      r2 = restr && !restricted->hasProperty( "Mesh" );
      if( !r2 )
      {
        if( restr )
          try
            {
              types.clear();
              rtypes = restricted->getProperty( "Mesh" );
              Object	i = rtypes->objectIterator();
              for( ; i->isValid(); i->next() )
                types.insert( i->getString() );
            }
          catch( ... )
            {
            }
        if( !restr || types.find( "VOID" ) != eir )
          registerProcessType( "Mesh", "VOID", &loadMesh<3> );
        if( !restr || types.find( "FLOAT" ) != eir )
          registerProcessType( "Mesh", "FLOAT", &loadTexturedMesh<3, float> );
        if( !restr || types.find( "POINT2DF" ) != eir )
          registerProcessType( "Mesh", "POINT2DF", &loadTexturedMesh<3,
                               Point2df> );
      }
      r2 = restr && !restricted->hasProperty( "Mesh4" );
      if( !r2 )
      {
        if( restr )
          try
            {
              types.clear();
              rtypes = restricted->getProperty( "Mesh4" );
              Object	i = rtypes->objectIterator();
              for( ; i->isValid(); i->next() )
                types.insert( i->getString() );
            }
          catch( ... )
            {
            }
        if( !restr || types.find( "VOID" ) != eir )
          registerProcessType( "Mesh4", "VOID", &loadMesh<4> );
        if( !restr || types.find( "FLOAT" ) != eir )
          registerProcessType( "Mesh4", "FLOAT", &loadTexturedMesh<4, float> );
        if( !restr || types.find( "POINT2DF" ) != eir )
          registerProcessType( "Mesh4", "POINT2DF", &loadTexturedMesh<4,
                               Point2df> );
      }
      r2 = restr && !restricted->hasProperty( "Bucket" );
      if( !r2 )
        registerProcessType( "Bucket", "VOID", &loadBucket );

      if( restr )
        try
          {
            types.clear();
            rtypes = restricted->getProperty( "Texture" );
            Object	i = rtypes->objectIterator();
            for( ; i->isValid(); i->next() )
              types.insert( i->getString() );
            r2 = false;
          }
        catch( ... )
          {
            r2 = true;
          }
      else
        r2 = false;
      if( !r2 )
        {
          if( !restr || types.find( "FLOAT" ) != eir )
            registerProcessType( "Texture", "FLOAT", &loadTex1d );
          if( !restr || types.find( "POINT2DF" ) != eir )
            registerProcessType( "Texture", "POINT2DF", &loadTex2d );
          if( !restr || types.find( "S16" ) != eir )
            registerProcessType( "Texture", "S16", &loadTex1dInt<int16_t> );
          if( !restr || types.find( "S32" ) != eir )
            registerProcessType( "Texture", "S32", &loadTex1dInt<int32_t> );
          if( !restr || types.find( "U32" ) != eir )
            registerProcessType( "Texture", "U32", &loadTex1dInt<uint32_t> );
          if( !restr || types.find( "POINT2D" ) != eir )
            registerProcessType( "Texture", "POINT2D", &loadTex2dInt<short> );
          if( !restr || types.find( "DOUBLE" ) != eir )
            registerProcessType( "Texture", "DOUBLE", &loadTex1dInt<double> );
        }

      r2 = restr && !restricted->hasProperty( "Graph" );
      if( !r2 )
        {
          registerProcessType( "Graph", "VOID", &loadGraph );
          registerProcessType( "Bundles", "any", &loadGraph );
          if( !vols16 )
            registerProcessType( "Volume", "S16", &loadGraph );
        }

      r2 = restr && !restricted->hasProperty( "SparseMatrix" );
      if( !r2 )
        registerProcessType( "SparseMatrix", "DOUBLE", &loadSparseMatrix );
    }

    AObject	*object;
    Object	options;
    ObjectReader::PostRegisterList subObjectsToRegister;
  };


  template<class T>
  bool loadVolume( Process & p, const string & fname, Finder & f )
  {
    AimsLoader	& ap = (AimsLoader &) p;
    VolumeRef<T> vref;
    if( !loadData( vref, fname, f, ap.options ) )
    {
      return( false );
    }
    AVolume<T>  *vol = new AVolume<T>( fname.c_str() /*, type*/ );
    vol->setVolume( vref );
    ap.object = vol;
    vol->setFileName( fname );
    vol->SetExtrema();
    vol->adjustPalette();
    return true;
  }

  template<>
  bool loadVolume<AimsHSV>( Process & p, const string & fname, Finder & f )
  {
    AimsLoader  & ap = (AimsLoader &) p;
    VolumeRef<AimsHSV> vref;
    if( !loadData( vref, fname, f, ap.options ) )
    {
      return false;
    }
    Converter<VolumeRef<AimsHSV>, VolumeRef<AimsRGB> > conv;
    VolumeRef<AimsRGB> *vref_tmp = conv( vref );
    VolumeRef<AimsRGB> vref2 = *vref_tmp;
    delete vref_tmp;
    AVolume<AimsRGB>  *vol = new AVolume<AimsRGB>( fname.c_str() /*, type*/ );
    vol->setVolume( vref2 );
    ap.object = vol;
    vol->setFileName( fname );
    vol->SetExtrema();
    vol->adjustPalette();
    return true;
  }

  template<class T>
  bool loadData( T & obj, const string & fname, Finder & f, Object options )
  {
    Reader<T>	reader( fname );
    reader.setAllocatorContext( AllocatorContext
                                ( AllocatorStrategy::ReadOnly, 
                                  DataSource::none(), false, 0.5 ) );
    reader.setOptions( options );
    string	format = f.format();
    try
    {
      reader.read( obj, 0, &format );
    }
    catch( exception & e )
    {
      cerr << e.what() << endl;
      return false;
    }
    return( true );
  }


  template<long D>
  bool loadMesh( Process & p, const string & fname, Finder & f )
  {
    AimsLoader	& ap = (AimsLoader &) p;
    AimsTimeSurface<D, Void>	*surf = new AimsTimeSurface<D, Void>;
    if( !loadData( *surf, fname, f, ap.options ) )
    {
      delete surf;
      return( false );
    }
    ASurface<D>	*ao = new ASurface<D>( fname.c_str() );
    ao->setSurface( surf );
    ap.object = ao;
    ao->setFileName( fname );
    string name = FileUtil::removeExtension( FileUtil::basename( fname ) );

    Object		otex;
    carto::ObjectVector	*tex = 0;
    unsigned		i, j, n, m;

    // cout << "check textures\n";
    try
    {
      otex = surf->header().getProperty( "textures" );
      tex = &otex->GenericObject::value<carto::ObjectVector>();
      // cout << "existing textures: " << tex->size() << endl;
    }
    catch( ... )
    {
      tex = 0;
    }
    try
      {
        Object	tfiles = surf->header().getProperty( "texture_filenames" );
        Object	iter;
        string	dirn = FileUtil::dirname( fname ) + FileUtil::separator();
        if( !tex )
          n = 0;
        else
          n = tex->size();
        for( i=0, iter=tfiles->objectIterator(); i<n && iter->isValid(); 
             ++i, iter->next() ) {}
        if( !tex && iter->isValid() )
          {
            otex = Object::value( ObjectVector() );
            surf->header().setProperty( "textures", otex );
            tex = &otex->GenericObject::value<ObjectVector>();
          }
        for( ; iter->isValid(); ++i, iter->next() )
          {
            string	texfname = iter->currentValue()->getString();
            if( !FileUtil::isAbsPath( texfname ) )
              texfname = dirn + texfname;
            ReaderAsObject	ord( texfname );
            try
              {
                tex->push_back( ord.read() );
              }
            catch( ... )
              {
              }
          }
        // cout << "textures loaded, now:" << tex->size() << endl;
      }
    catch( ... )
      {
      }

    if( tex && tex->size() > 0 )
      try
        {
          // cout << "textures found\n";
          vector<string>	texnames, texdisp;
          vector<Object>	otd;
          map<string, int>	texd;

          try
            {
              Object	tn = surf->header().getProperty( "texture_names" );
              Object	iter;
              texnames.reserve( tn->size() );
              for( iter=tn->objectIterator(); iter->isValid(); iter->next() )
                {
                  string	x;
                  try
                    {
                      x = iter->currentValue()->getString();
                    }
                  catch( ... )
                    {
                    }
                  texnames.push_back( x );
                }
            }
          catch( ... )
            {
            }
          n = texnames.size();
          if( surf->header().getProperty( "texture_displayed", otd ) )
            {
              m=otd.size();
              texdisp.reserve( m );
              for( i=0; i<m; ++i )
                try
                  {
                    texdisp.push_back( otd[i]->getString() );
                  }
                catch( ... )
                  {
                  }
              if( texnames.empty() )
                texnames = texdisp;
            }
          else
            texdisp = texnames;

          m = texdisp.size();
          for( i=0; i<n; ++i )
            {
              for( j=0; j<m; ++j )
                if( texnames[i] == texdisp[j] )
                  {
                    texd[ texnames[i] ] = j;
                    break;
                  }
            }

          n = tex->size();
          m = texnames.size();
          vector<AObject *>	atex;
          ATexture		*at;
          map<string, int>::const_iterator	itm, etm = texd.end();
          atex.reserve( n );
          string		tname;
          bool		toapply;

          for( i=0; i<n; ++i )
            {
              at = new ATexture;
              toapply = false;
              if( texd.empty() )
                toapply = true;
              if( i < m )
                {
                  tname = texnames[i];
                  if( texd.find( tname ) != etm )
                    toapply = true;
                  tname = string( "texture_" ) + name + '_' + tname;
                }
              else
                tname = string( "texture_" ) + name;
              if( (*tex)[i]->type() == DataTypeCode<Texture1d>::name() )
                at->setTexture( rc_ptr<Texture1d>( new Texture1d
                    ( (*tex)[i]->GenericObject::value<Texture1d>() ) ) );
              else if( (*tex)[i]->type() == DataTypeCode<Texture2d>::name() )
                at->setTexture( rc_ptr<Texture2d>( new Texture2d
                    ( (*tex)[i]->GenericObject::value<Texture2d>() ) ) );
              else
              { // element not recognized as a texture: forget it.
                delete at;
                at = 0;
              }
              if( at )
              {
                // at->setFileName( "texture" );
                at->setHeaderOptions();
                at->setName( theAnatomist->makeObjectName( tname ) );
                if( toapply )
                  {
                    ap.subObjectsToRegister.push_back( make_pair( at,
                                                                  false ) );
                    atex.push_back( at );
                  }
                else
                  ap.subObjectsToRegister.push_back( make_pair( at, true ) );
              }
            }
          AObject		*tex = 0;
          FusionFactory	*ff = FusionFactory::factory();
          FusionMethod	*fm;
          // eventually, fix n
          n = atex.size();
          if( n > 1 )
            {
              if( n >= 3 )
                {
                  int	ired = -1, igreen = -1, iblue = -1;
                  itm = texd.find( "red" );
                  if( itm != etm )
                    ired = itm->second;
                  itm = texd.find( "green" );
                  if( itm != etm )
                    igreen = itm->second;
                  itm = texd.find( "blue" );
                  if( itm != etm )
                    iblue = itm->second;
                  if( ired >= 0 && igreen >= 0 && iblue >= 0 )
                    {
                      const PaletteList & pall = theAnatomist->palettes();
                      rc_ptr<APalette>
                        pal = pall.find( "multitex-geom-red-mask" );
                      AObjectPalette	*opal;
                      if( pal )
                        {
                          atex[ired]->getOrCreatePalette();
                          opal = atex[ired]->palette();
                          opal->setRefPalette( pal );
                          opal->fill();
                          atex[ired]->setPalette( *opal );
                        }
                      pal = pall.find( "multitex-geom-green-mask" );
                      if( pal )
                        {
                          atex[igreen]->getOrCreatePalette();
                          opal = atex[igreen]->palette();
                          opal->setRefPalette( pal );
                          opal->fill();
                          atex[igreen]->setPalette( *opal );
                        }
                      pal = pall.find( "multitex-geom-blue-mask" );
                      if( pal )
                        {
                          atex[iblue]->getOrCreatePalette();
                          opal = atex[iblue]->palette();
                          opal->setRefPalette( pal );
                          opal->fill();
                          atex[iblue]->setPalette( *opal );
                        }
                    }
                }
              fm = ff->method( "FusionMultiTextureMethod" );
              tex = fm->fusion( atex );
              tex->setName( theAnatomist->makeObjectName
                            ( string( "mtexture_" ) + name ) );
              ap.subObjectsToRegister.push_back( make_pair( tex, false ) );
            }
          else if( n > 0 )
            tex = atex[0];
          if( tex )
          {
            vector<AObject *>	ts(2);
            ts[0] = ao;
            ts[1] = tex;
            fm = ff->method( "FusionTexSurfMethod" );
            AObject	*tso = fm->fusion( ts );
            ao->setName( theAnatomist->makeObjectName( name + "_mesh" ) );
            ao->setHeaderOptions();
            ap.subObjectsToRegister.push_back( make_pair( ao, false ) );
            surf->header().removeProperty( "textures" );
            // copy material from mesh to texsurf.
            tso->SetMaterial( ao->GetMaterial() );
            ap.object = tso;
          }
        }
      catch( ... )
        {
        }
    return true;
  }


  template <long D, typename T>
  bool loadTexturedMesh( Process & p, const string & fname, Finder & f )
  {
    // load textured mesh
    AimsLoader	& ap = (AimsLoader &) p;
    AimsTimeSurface<D, T>	*surf = new AimsTimeSurface<D, T>;
    if( !loadData( *surf, fname, f, ap.options ) )
    {
      delete surf;
      return false;
    }

    // split mesh / texture
    AimsTimeSurface<D, Void> *mesh = new AimsTimeSurface<D, Void>;
    TimeTexture<T> *tex = new TimeTexture<T>;
    typename AimsTimeSurface<D, T>::iterator im, em = surf->end();

    for( im=surf->begin(); im!=em; ++im )
    {
      AimsSurface<D, Void> & s0 = (*mesh)[im->first];
      s0.vertex() = im->second.vertex();
      s0.normal() = im->second.normal();
      s0.polygon() = im->second.polygon();
      Texture<T> & t0 = (*tex)[im->first];
      t0.data() = im->second.texture();
    }
    mesh->header().copy( surf->header() );
    tex->header().copy( surf->header() );
    delete surf; // no need for it any longer

    // build AObjects

    ASurface<D>	*ao = new ASurface<D>( fname.c_str() );
    ao->setSurface( mesh );
    ap.object = ao;
    string name = FileUtil::removeExtension( FileUtil::basename( fname ) );
    ao->setFileName( name + "_mesh.gii" );
    ap.subObjectsToRegister.push_back( make_pair( ao, false ) );

    ATexture *to = new ATexture;
    to->setTexture( rc_ptr<TimeTexture<T> >( tex ) );
    ap.subObjectsToRegister.push_back( make_pair( to, false ) );

    vector<AObject *>	ts(2);
    ts[0] = ao;
    ts[1] = to;
    FusionFactory	*ff = FusionFactory::factory();
    FusionMethod	*fm = ff->method( "FusionTexSurfMethod" );
    AObject	*tso = fm->fusion( ts );

    ao->setName( theAnatomist->makeObjectName( name + "_mesh" ) );
    ao->setHeaderOptions();
    to->setName( theAnatomist->makeObjectName( name + "_texture" ) );
    to->setHeaderOptions();
//     surf->header().removeProperty( "textures" );
    // copy material from mesh to texsurf.
    tso->SetMaterial( ao->GetMaterial() );
    ap.object = tso;

    return true;
  }


  bool loadTex1d( Process & p, const string & fname, Finder & f )
  {
    AimsLoader	& ap = (AimsLoader &) p;
    rc_ptr<Texture1d>	tex( new Texture1d );
    if( !loadData( *tex, fname, f, ap.options ) )
      return false;
    ATexture	*ao = new ATexture;
    ao->setTexture( tex );
    ap.object = ao;
    return( true );
  }


  template<typename T>
  bool loadTex1dInt( Process & p, const string & fname, 
                     Finder & f )
  {
    AimsLoader		& ap = (AimsLoader &) p;
    TimeTexture<T>	ts;

    if( !loadData( ts, fname, f, ap.options ) )
      return false;
    rc_ptr<Texture1d>	tex( new Texture1d );

    Converter<TimeTexture<T>, Texture1d>	c;
    c.convert( ts, *tex );

    ATexture	*ao = new ATexture;
    ao->setTexture( tex );
    ap.object = ao;
    return( true );
  }


  bool loadTex2d( Process & p, const string & fname, 
                  Finder & f )
  {
    AimsLoader	& ap = (AimsLoader &) p;
    rc_ptr<Texture2d>	tex( new Texture2d );
    if( !loadData( *tex, fname, f, ap.options ) )
      return false;
    ATexture	*ao = new ATexture;
    ao->setTexture( tex );
    ap.object = ao;
    return( true );
  }


  template<typename T>
  bool loadTex2dInt( Process & p, const string & fname, 
                     Finder & f )
  {
    AimsLoader	& ap = (AimsLoader &) p;
    TimeTexture<AimsVector<T, 2> > ts;

    if( !loadData( ts, fname, f, ap.options ) )
      return( false );
    rc_ptr<Texture2d> tex( new Texture2d );
    typename TimeTexture<AimsVector<T, 2> >::const_iterator 
      it, et = ts.end();
    unsigned	i, n;

    for( it=ts.begin(); it!=et; ++it )
    {
      Texture<Point2df>	& tx = (*tex)[ it->first ];
      n = it->second.nItem();
      tx.reserve( n );
      for( i=0; i<n; ++i )
        tx.push_back( Point2df( it->second.item( i )[0],
                                it->second.item( i )[1] ) );
    }
    tex->header() = ts.header();

    ATexture	*ao = new ATexture;
    ao->setTexture( tex );
    ap.object = ao;
    return( true );
  }


  bool loadBucket( Process & p, const string & fname, Finder & f )
  {
    AimsLoader	& ap = (AimsLoader &) p;
    Bucket		*ao = new Bucket( fname.c_str() );
    if( !loadData( ao->bucket(), fname, f, ap.options ) )
      {
        delete ao;
        return( false );
      }
    ap.object = ao;
    ao->setFileName( fname );
    ao->setGeomExtrema();
    return( true );
  }


  bool loadGraph( Process & p, const string & fname, Finder & )
  {
    AimsLoader	& ap = (AimsLoader &) p;
    AObject	*ao = AGraph::LoadGraph( fname.c_str(), ap.options );
    if( ao )
    {
      ap.object = ao;
      return( true );
    }
    return( false );
  }


  bool loadSparseMatrix( Process & p, const string & fname, 
                         Finder & f )
  {
    AimsLoader  & ap = (AimsLoader &) p;
    rc_ptr<SparseOrDenseMatrix>   obj( new SparseOrDenseMatrix );
    if( !loadData( *obj, fname, f, ap.options ) )
      return false;
    ASparseMatrix    *ao = new ASparseMatrix;
    ao->setMatrix( obj );
    ap.object = ao;
    return true;
  }


  bool checkCopyFile( const string & filein, const string & extension, 
                      set<string> & tempFiles, const string & fbase );
  bool readCompressed( const string & filein, const string & fileout );
  template <typename T>
  void checkFormats( const string & ext, set<string> & exts );

  bool checkCompressedFiles( const string & filename, string & newFilename, 
                             set<string> & tempFiles )
  {
    if( ( filename.length() < 3 
          || filename.substr( filename.length() - 2, 2 ) != ".Z" )
        && ( filename.length() < 4 
             || filename.substr( filename.length() - 3, 3 ) != ".gz" ) )
      return( false );	// not compressed

    string	fbase 
      = FileUtil::removeExtension( FileUtil::basename( filename ) );
    string	ext = FileUtil::extension( fbase );

    int		fd;
    newFilename = FileUtil::temporaryFile( fbase, fd );
    if( fd == -1 )
      return false;
    ::close( fd );

    if( !readCompressed( filename, newFilename ) )
      return( 0 );	// cound not uncompress
    tempFiles.insert( newFilename );  // remember to remove the file at the end

    //	look for other files going together with the 1st compressed one
    set<string>	others;

    checkFormats<AimsData<int8_t> >( ext, others );
    checkFormats<AimsData<uint8_t> >( ext, others );
    checkFormats<AimsData<int16_t> >( ext, others );
    checkFormats<AimsData<uint16_t> >( ext, others );
    checkFormats<AimsData<int32_t> >( ext, others );
    checkFormats<AimsData<uint32_t> >( ext, others );
    checkFormats<AimsData<int64_t> >( ext, others );
    checkFormats<AimsData<uint64_t> >( ext, others );
    checkFormats<AimsData<float> >( ext, others );
    checkFormats<AimsData<double> >( ext, others );
    checkFormats<AimsData<AimsRGB> >( ext, others );
    checkFormats<AimsData<AimsRGBA> >( ext, others );
    checkFormats<AimsSurfaceTriangle>( ext, others );
    checkFormats<BucketMap<Void> >( ext, others );
    checkFormats<Texture1d>( ext, others );
    checkFormats<Texture2d>( ext, others );
    checkFormats<TimeTexture<short> >( ext, others );
    checkFormats<TimeTexture<int> >( ext, others );
    checkFormats<TimeTexture<unsigned> >( ext, others );
    checkFormats<TimeTexture<Point2d> >( ext, others );
    checkFormats<AimsSurfaceTriangle>( ext, others );
    checkFormats<AimsTimeSurface<2,Void> >( ext, others );
    checkFormats<AimsTimeSurface<4,Void> >( ext, others );
    checkFormats<AimsTimeSurface<2, float> >( ext, others );
    checkFormats<AimsTimeSurface<3, float> >( ext, others );
    checkFormats<AimsTimeSurface<4, float> >( ext, others );
    checkFormats<Graph>( ext, others );

    //	check extensions
    set<string>::iterator	ie, ee = others.end();

    for( ie=others.begin(); ie!=ee; ++ie )
      {
        if( *ie != ext )
          checkCopyFile( filename, string( "." ) + *ie, tempFiles, 
                         newFilename );
        checkCopyFile( filename, string( "." ) + *ie + ".minf", tempFiles, 
                       newFilename );
      }

    return true;
  }


  bool checkCopyFile( const string & filein, const string & extension, 
                      set<string> & tempFiles, const string & fbase )
  {
    string	filename = filein, file2, zext = FileUtil::extension( filein );
    string	fb = FileUtil::removeExtension( fbase ) + extension;

    if( zext == "Z" || zext == "gz" )
      filename = FileUtil::removeExtension( filename );
    filename = FileUtil::removeExtension( filename ) + extension;

    cout << "checking file " << filename << endl;

    struct stat	s;

    if( !stat( filename.c_str(), &s ) )
      {
        cout << "file found\n";

        FILE	*f = fopen( filename.c_str(), "rb" );
        FILE	*d;
        int	c;

        if( !f )
          {
            cerr << "cannot open file " << filename << endl;
            return( false );
          }

        cout << "copying file " << filename << " to " << fb << endl;

        d = fopen( fb.c_str(), "wb" );
        if( !d )
          {
            cerr << "cannot open output file\n";
            return( false );
          }

        char	buffer[ 65536 ];
        while( ( c = fread( buffer, 1, 65536, f ) ) == 65536 )
          fwrite( buffer, 1, 65536, d );
        if( c > 0 )
          fwrite( buffer, 1, c, d );
        fclose( d );
        fclose( f );

        tempFiles.insert( fb );
        return true;
      }
    else if( !stat( (file2 = filename + ".Z").c_str(), &s ) 
             || !stat( (file2 = filename + ".gz" ).c_str(), &s ) )
      {
        if( readCompressed( file2, fb ) )
          {
            tempFiles.insert( fb );
            return true;
          }
        else
          return false;
      }
    return false;
  }


  bool readCompressed( const string & filein, const string & fileout )
  {
    FILE		*f;
    bool		zformat = false;

    if( FileUtil::extension( filein ) == ".Z" )
      zformat = true;	// .Z, not .gz

    cout << "uncompressing file " << filein << " into " << fileout << endl;

    f = fopen( fileout.c_str(), "wb" );
    if( !f )
      {
        cerr << "cannot open output file\n";
        return( false );
      }

    int		c, errnum = Z_OK;

    if( zformat )
      {
        cout << "Z format\n";

        /*unsigned char	ibuf[65536], obuf[65536];
          FILE		*ist = fopen( filein, "rb" );
          int		st;
          z_stream		zstr;
          bool		initdone = false;

          if( !ist )
          {
	  cerr << "cannot open compressed " << filein << endl;
	  fclose( f );
	  remove( );
	  return( false );
          }

          zstr.total_in = 0;
          zstr.total_out = 0;
          zstr.msg = 0;
          zstr.zalloc = Z_NULL;
          zstr.zfree = Z_NULL;
          zstr.opaque = 0;
          zstr.data_type = Z_BINARY;

          while( !feof( ist ) )
          {
	  c = fread( ibuf, 1, 65536, ist );
	  if( c <= 0 )
          break;
	  cout << "read " << c << " bytes\n";
	  zstr.next_in = ibuf;
	  zstr.avail_in = c;
	  zstr.next_out = obuf;
	  zstr.avail_out = 65536;

	  if( !initdone && inflateInit( &zstr ) != Z_OK )
          {
          cerr << "uncompress init failed. I can't read .Z format\n";
          fclose( f );
          fclose( ist );
          remove( .. );
          return( false );
          }

	  do
          {
          st = inflate( &zstr, Z_SYNC_FLUSH );
          fwrite( obuf, 1, zstr.next_out - obuf, f );
          zstr.next_out = obuf;
          zstr.avail_out = 65536;
          }
	  while( ( st == Z_OK || st == Z_STREAM_END ) && zstr.avail_in > 0 );

	  inflateEnd( &zstr );

	  if( st != Z_OK || st != Z_STREAM_END )
          {
          cerr << "Error " << st << " in uncompress\n";
          fclose( f );
          fclose( ist );
          remove( .. );
          return( false );
          }
          }
          fclose( f );
          fclose( ist ); */

        // bon, j'y arrive pas, on fait simple.
        fclose( f );
        string	cmd = "uncompress -c ";
        int	err;
        cmd += filein + " > " + fileout;
        cout << "executing : " << cmd << endl;
        err = system( cmd.c_str() );

        if( err )
          {
            remove( fileout.c_str() );
            return( false );
          }
      }
    else
      {
        unsigned char		buffer[65536];
        gzFile	gf = gzopen( filein.c_str(), "rb" );
        if( !gf )
          {
            cout << "cannot open gzipped " << filein << endl;
            fclose( f );
            unlink( fileout.c_str() );
            return( false );
          }

        while( ( c = gzread( gf, buffer, 65536 ) ) == 65536 )
          {
            gzerror( gf, &errnum );
            if( errnum != Z_OK )
              break;
            fwrite( buffer, 1, 65536, f );
          }
        if( c > 0 )
          fwrite( buffer, 1, c, f );

        gzclose( gf );
        fclose( f );
        if( errnum != Z_OK && errnum != Z_STREAM_END )
          {
            cerr << "error in decompression. Not in gzip format ?\n";
            unlink( fileout.c_str() );
            return false;
          }
      }

    return true;
  }


  void delFiles( const set<string> & files )
  {
    set<string>::const_iterator	is, fs=files.end();

    for( is = files.begin(); is!=fs; ++is )
      {
        remove( (*is).c_str() );
      }
  }


  template<typename T>
  void checkFormats( const string & ext, set<string> & exts )
  {
    const map<string, list<string> >	& extf
      = FileFormatDictionary<T>::extensions();
    map<string, list<string> >::const_iterator
      ie = extf.find( ext ), ee = extf.end();
    if( ie == ee )
      return;	// ext not found for this type

    set<string>			fmts;
    set<string>::iterator	ef = fmts.end();
    list<string>::const_iterator  ief, eef = ie->second.end();

    for( ief=ie->second.begin(); ief!=eef; ++ief )
      fmts.insert( *ief );

    for( ie=extf.begin(); ie!=ee; ++ie )
      for( ief=ie->second.begin(), eef=ie->second.end(); ief!=eef; ++ief )
        if( fmts.find( *ief ) != ef )
          exts.insert( ie->first );
  }

}

AObject* ObjectReader::load_internal( const string & filename, 
                                      PostRegisterList & subObjectsToRegister,
                                      Object options ) const
{
  AObject       *object = 0;

  string extension = FileUtil::extension( filename );
  if( !extension.empty() )
  {
    pair<_storagetype::const_iterator, _storagetype::const_iterator>
        il = _loaders().equal_range( extension );
    _storagetype::const_iterator im, em = il.second;
    for( im = il.first; im!=em; ++im )
    {
      object = im->second->load( filename, subObjectsToRegister, options );
      if( object )
        return object;
    }
  }

  if( !object )
  //  extension not recognized : let's try the Aims finder
    object = ObjectReader::readAims( filename, subObjectsToRegister, options );

  return object;
}


AObject* ObjectReader::load( const string & filename,
                             PostRegisterList & subObjectsToRegister,
                             bool notifyFail,
                             Object options, void* clientid ) const
{
//   cout << "ObjectReader::load\n";
  AObject       *object = load_internal( filename, subObjectsToRegister,
                                         options );
//   cout << "load_internal done\n";
  if( !object )
  {
    //	Tentative de lecture des .Z et .gz

    string	newFilename;
    set<string>	tempFiles;

    if( checkCompressedFiles( filename, newFilename, tempFiles ) )
    {
      object = load_internal( newFilename, subObjectsToRegister, options );
      delFiles( tempFiles );
    }
  }

  bool async = false;
  if( options )
  {
    try
    {
      Object x = options->getProperty( "asynchronous" );
      if( x && (bool) x->getScalar() )
        async = true;
    }
    catch( ... )
    {
    }
  }

  if( object )
  {
    object->setFileName( filename );
    object->setName( theAnatomist->makeObjectName(
        FileUtil::basename( filename ) ) );
    object->setHeaderOptions();
    bool autoref = false;
    try
    {
      Object autorefo = theAnatomist->config()->getProperty(
          "setAutomaticReferential" );
      if( !autorefo.isNull() )
        autoref = (bool) autorefo->getScalar();
    }
    catch( ... )
    {
    }
    /* int	spmo = 0;
    theAnatomist->config()->getProperty( "useSpmOrigin", spmo );
    */
    if( autoref )
    {
      set<AObject *> so;
      so.insert( object );
      ObjectActions::setAutomaticReferential( so );
    }
    object->setLoadDate( time( 0 ) );

    if( !async )
      // sync mode: immediately update
      object->notifyObservers( (void *) this );
  }
  else
  {
    cerr << "Could not read object in file " << filename << " !\n";
    if( notifyFail )
    {
        // warn message box...
        // in the MAIN thread...
    }
  }

  if( async )
  {
    // async mode: post an event
    AObjectLoadEvent* ev = new AObjectLoadEvent( object,
                                                 subObjectsToRegister,
                                                 options, clientid );
    qApp->postEvent( ObjectReaderNotifier::notifier(), ev );
  }

  return object;
}


bool ObjectReader::reload( AObject* object, bool notifyFail, 
                           bool /* onlyoutdated TODO */ ) const
{
  string	fname = object->fileName();
  if( !object->loadable() || fname.empty() )
    return false;

  //	Tentative de lecture des .Z et .gz

  set<string>	tempFiles;

  bool  status = false;
  try
  {
    status = object->reload( fname /*, options*/ );
  }
  catch( ... )
  {
  }

  if( !status )
  {
    string        file;
    if( checkCompressedFiles( fname, file, tempFiles ) )
    {
      status = object->reload( file /*, options*/ );
      delFiles( tempFiles );
    }
  }

  if( status )
  {
    object->setLoadDate( time( 0 ) );

    GLComponent	*c = object->glAPI();
    if( c )
    {
      c->glSetChanged( GLComponent::glGEOMETRY );
      c->glSetChanged( GLComponent::glTEXIMAGE );
      c->glSetChanged( GLComponent::glTEXENV );
    }
    else
      object->setChanged();
    Referential *oldref = 0;
    if( !object->referentialInheritance() )
      oldref = object->getReferential();
    object->setHeaderOptions();
    if( object->getReferential() == theAnatomist->centralReferential() )
      object->setReferential( oldref );
    object->notifyObservers( object );
  }
  else
  {
    cout << "Reload failed !\n";
    if( notifyFail )
    {
      //  message box etc.
    }
  }

  return( status );
}


AObject* ObjectReader::readAims( const string & file,
                                 PostRegisterList & subObjectsToRegister,
                                 Object options ) const
{
  AimsLoader		proc( options );

  proc.setReadOptions( options );

  string url_options;
  if( options && options->hasProperty( "url_options" ) )
  {
    options->getProperty( "url_options", url_options );
    // in case options are "sx=12&sy=15" instead of "?sx=12&sy=15"
    if( !url_options.empty() && url_options[0] != '?' )
      url_options = "?" + url_options;
  }

  try
    {
      if( !proc.execute( file + url_options ) )
        return 0;
      subObjectsToRegister = proc.subObjectsToRegister;
      return proc.object;
    }
  catch( exception & e )
    {
      cerr << e.what() << endl;
    }
  return 0;
}


namespace
{

  class ObjectReader_LoadFunctionPtrClass
  : public ObjectReader::LoadFunctionClass
  {
  public:
    ObjectReader_LoadFunctionPtrClass( ObjectReader::LoadFunction x )
      : ObjectReader::LoadFunctionClass(), func( x )
    {
    }

    virtual
    ~ObjectReader_LoadFunctionPtrClass()
    {
    }

    virtual AObject * load( const string & filename,
                            ObjectReader::PostRegisterList &
                            subObjectsToRegister,
                            Object options )
    {
      return func( filename, subObjectsToRegister, options );
    }

    private:
      ObjectReader::LoadFunction func;
  };

}


void
ObjectReader::registerLoader( const string & extension, LoadFunction newFunc )
{
  registerLoader( extension,
                  new ObjectReader_LoadFunctionPtrClass( newFunc ) );
}


void
ObjectReader::registerLoader( const string & extension,
                              LoadFunctionClass *newFunc )
{
  _loaders().insert( std::make_pair( extension,
           rc_ptr<LoadFunctionClass>( newFunc ) ) );
}


ObjectReader::_storagetype & ObjectReader::_loaders()
{
  static _storagetype loaders;
  return loaders;
}

// -------


ObjectReader::LoadFunctionClass::~LoadFunctionClass()
{
}


namespace
{

  void addZippedFormats( set<string> & exts )
  {
    set<string>::iterator i, e = exts.end();
    for( i=exts.begin(); i!=e; ++i )
      if( !i->empty() )
      {
        if( i->length() < 2 || ( i->substr( i->length()-2, 2 ) != ".Z"
            && ( i->length() < 3
                || i->substr( i->length()-3, 3 ) != ".gz" ) ) )
        exts.insert( *i + ".Z" );
        if( i->length() < 3 || ( i->substr( i->length()-3, 3 ) != ".gz"
            && i->substr( i->length()-2, 2 ) != ".Z" ) )
        exts.insert( *i + ".gz" );
      }
  }


  void formatsString( const set<string> & exts, string & sexts )
  {
    bool first = true;
    set<string>::iterator i, e = exts.end();
    for( i=exts.begin(); i!=e; ++i )
      if( !i->empty() )
    {
      if( first )
        first = false;
      else
        sexts += " ";
      sexts += string( "*." ) + *i;
    }
  }

}


set<string> ObjectReader::anatomistSupportedFileExtensionsSet()
{
  set<string> exts;
  _storagetype::const_iterator it, et = _loaders().end();
  for( it=_loaders().begin(); it!=et; ++it )
    exts.insert( it->first );
  addZippedFormats( exts );
  return exts;
}


string ObjectReader::allSupportedFileExtensions()
{
  // anatomist-specific extensions
  set<string> exts = anatomistSupportedFileExtensionsSet();
  // aims extensions
  set<string> aexts = supportedFileExtensionsSet( "Volume" );
  exts.insert( aexts.begin(), aexts.end() );
  aexts = supportedFileExtensionsSet( "Mesh" );
  exts.insert( aexts.begin(), aexts.end() );
  aexts = supportedFileExtensionsSet( "Mesh4" );
  exts.insert( aexts.begin(), aexts.end() );
  aexts = supportedFileExtensionsSet( "Segments" );
  exts.insert( aexts.begin(), aexts.end() );
  aexts = supportedFileExtensionsSet( "Texture" );
  exts.insert( aexts.begin(), aexts.end() );
  aexts = supportedFileExtensionsSet( "Graph" );
  exts.insert( aexts.begin(), aexts.end() );
  aexts = supportedFileExtensionsSet( "Hierarchy" );
  exts.insert( aexts.begin(), aexts.end() );
  aexts = supportedFileExtensionsSet( "Bucket" );
  exts.insert( aexts.begin(), aexts.end() );
  aexts = supportedFileExtensionsSet( "SparseMatrix" );
  exts.insert( aexts.begin(), aexts.end() );

  string sexts;
  formatsString( exts, sexts );
  return sexts;
}


namespace
{
  set<string> somaObjectTypesFor( const string & aimstype )
  {
    set<string> sotypes;
    if( aimstype == "Volume" )
    {
      sotypes.insert( "carto_volume of S8" );
      sotypes.insert( "carto_volume of U8" );
      sotypes.insert( "carto_volume of S16" );
      sotypes.insert( "carto_volume of U16" );
      sotypes.insert( "carto_volume of S32" );
      sotypes.insert( "carto_volume of U32" );
      sotypes.insert( "carto_volume of S64" );
      sotypes.insert( "carto_volume of U64" );
      sotypes.insert( "carto_volume of FLOAT" );
      sotypes.insert( "carto_volume of DOUBLE" );
      sotypes.insert( "carto_volume of RGB" );
      sotypes.insert( "carto_volume of RGBA" );
    }
    else
      sotypes.insert( aimstype );
    return sotypes;
  }

}


set<string> ObjectReader::supportedFileExtensionsSet(
  const std::string & objtype )
{
  set<string> exts;

  // Soma IO extensions
  set<string> sotypes = somaObjectTypesFor( objtype );
  set<string>::const_iterator esot = sotypes.end();
  vector<map<string, soma::IOObjectTypesDictionary::FormatInfo> *> io;
  io.push_back( &soma::IOObjectTypesDictionary::readTypes() );
  io.push_back( &soma::IOObjectTypesDictionary::writeTypes() );
  vector<map<string, soma::IOObjectTypesDictionary::FormatInfo> *>
    ::const_iterator iio, eio = io.end();
  for( iio=io.begin(); iio!=eio; ++iio )
  {
    map<string, soma::IOObjectTypesDictionary::FormatInfo>::const_iterator
      isf, esf = (*iio)->end();
    for( isf=(*iio)->begin(); isf!=esf; ++isf )
      if( sotypes.find( isf->first ) != esot )
      {
        set<string> formats = isf->second();
        set<string>::const_iterator ifo, efo = formats.end();
        for( ifo=formats.begin(); ifo!=efo; ++ifo )
        {
          set<string> e = DataSourceInfoLoader::extensions( *ifo );
          exts.insert( e.begin(), e.end() );
        }
      }
  }

  // AIMS IO extensions
  const map<string, map<string, aims::IOObjectTypesDictionary::FormatInfo> >
      & types = aims::IOObjectTypesDictionary::types();
  if( !types.empty() )
  {
    map<string, map<string,
      aims::IOObjectTypesDictionary::FormatInfo> >::const_iterator
        it = types.find( objtype );
    map<string, aims::IOObjectTypesDictionary::FormatInfo>::const_iterator
      ifo, efo;
    set<string>::iterator ifm, efm;
    if( it != types.end() )
    {
      for( ifo=it->second.begin(), efo=it->second.end(); ifo!=efo; ++ifo )
      {
        const set<string> & formats = ifo->second();
        for( ifm=formats.begin(), efm=formats.end(); ifm!=efm; ++ifm )
        {
          set<string> e = Finder::extensions( *ifm );
          exts.insert( e.begin(), e.end() );
        }
      }
    }
  }

  addZippedFormats( exts );

  return exts;
}


string ObjectReader::supportedFileExtensions( const string & objtype )
{
  set<string> exts = supportedFileExtensionsSet( objtype );
  string sexts;
  formatsString( exts, sexts );
  return sexts;
}


string ObjectReader::supportedFileExtensions( const set<string> & objtypes )
{
  set<string> exts, exts2;
  set<string>::const_iterator i, e = objtypes.end();
  for( i=objtypes.begin(); i!=e; ++i )
  {
    exts2 = supportedFileExtensionsSet( *i );
    exts.insert( exts2.begin(), exts2.end() );
  }
  string sexts;
  formatsString( exts, sexts );
  return sexts;
}


string ObjectReader::anatomistSupportedFileExtensions()
{
  set<string> exts = anatomistSupportedFileExtensionsSet();
  string sexts;
  formatsString( exts, sexts );
  return sexts;
}


