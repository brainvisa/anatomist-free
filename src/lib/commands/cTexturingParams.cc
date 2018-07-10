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


#include <anatomist/commands/cTexturingParams.h>
#include <anatomist/processor/Registry.h>
#include <anatomist/processor/Serializer.h>
#include <anatomist/processor/unserializer.h>
#include <anatomist/processor/context.h>
#include <anatomist/object/Object.h>
#include <anatomist/surface/glcomponent.h>
#include <anatomist/application/Anatomist.h>
#include <cartobase/object/syntax.h>
#include <graph/tree/tree.h>

using namespace anatomist;
using namespace carto;
using namespace std;


TexturingParamsCommand::TexturingParamsCommand( const set<AObject *> & obj, 
                                                unsigned tex, int mode, 
                                                int filter, int gen, 
                                                float rate, int rgbint, 
                                                const float* genp1, 
                                                const float* genp2, 
                                                const float* genp3 )
  : RegularCommand(), _objects( obj ), _tex( tex ), _mode( mode ), 
    _filter( filter ), _gen( gen ), _rate( rate ), _rgbinter( rgbint )
{
  if( genp1 )
    {
      _genparams_1.reserve( 4 );
      _genparams_1.push_back( genp1[0] );
      _genparams_1.push_back( genp1[1] );
      _genparams_1.push_back( genp1[2] );
      _genparams_1.push_back( genp1[3] );
    }
  if( genp2 )
    {
      _genparams_2.reserve( 4 );
      _genparams_2.push_back( genp2[0] );
      _genparams_2.push_back( genp2[1] );
      _genparams_2.push_back( genp2[2] );
      _genparams_2.push_back( genp2[3] );
    }
  if( genp3 )
    {
      _genparams_3.reserve( 4 );
      _genparams_3.push_back( genp3[0] );
      _genparams_3.push_back( genp3[1] );
      _genparams_3.push_back( genp3[2] );
      _genparams_3.push_back( genp3[3] );
    }
}


TexturingParamsCommand::~TexturingParamsCommand()
{
}


bool TexturingParamsCommand::initSyntax()
{
  SyntaxSet	ss;
  Syntax	& s = ss[ "TexturingParams" ];
  
  s[ "objects"             ] = Semantic( "int_vector", true );
  s[ "texture_index"       ] = Semantic( "int", false );
  s[ "mode"                ] = Semantic( "string", false );
  s[ "filtering"           ] = Semantic( "string", false );
  s[ "generation"          ] = Semantic( "string", false );
  s[ "rate"                ] = Semantic( "float", false );
  s[ "interpolation"       ] = Semantic( "string", false );
  s[ "generation_params_1" ] = Semantic( "float_vector", false );
  s[ "generation_params_2" ] = Semantic( "float_vector", false );
  s[ "generation_params_3" ] = Semantic( "float_vector", false );
  Registry::instance()->add( "TexturingParams", &read, ss );
  return true;
}


void TexturingParamsCommand::doit()
{
  set<AObject *>::const_iterator	io, eo = _objects.end();
  GLComponent				*gc;
  for( io=_objects.begin(); io!=eo; ++io )
    if( theAnatomist->hasObject( *io ) )
      {
        gc = (*io)->glAPI();
        if( gc && gc->glNumTextures() > _tex )
          {
            if( _mode >= 0 )
            {
              /* check if texturing mode is compatible with the object /
              texture */
              set<GLComponent::glTextureMode> atm = gc->glAllowedTexModes(
                _tex );
              if( atm.find( (GLComponent::glTextureMode) _mode ) != atm.end() )
                gc->glSetTexMode( (GLComponent::glTextureMode) _mode, _tex );
            }
            if( _filter >= 0 )
              gc->glSetTexFiltering( (GLComponent::glTextureFiltering) 
                                     _filter, _tex );
            if( _gen >= 0 )
              gc->glSetAutoTexMode( (GLComponent::glAutoTexturingMode) _gen, 
                                    _tex );
            if( _rate >= 0 )
              gc->glSetTexRate( _rate, _tex );
            if( _rgbinter >= 0 )
              gc->glSetTexRGBInterpolation( (bool) _rgbinter, _tex );

            if( _genparams_1.size() >= 4 )
              gc->glSetAutoTexParams( &_genparams_1[0], 0, _tex );
            if( _genparams_2.size() >= 4 )
              gc->glSetAutoTexParams( &_genparams_2[0], 1, _tex );
            if( _genparams_3.size() >= 4 )
              gc->glSetAutoTexParams( &_genparams_3[0], 2, _tex );

            if( (*io)->hasChanged() )
              (*io)->notifyObservers( this );
          }
      }
}


Command* TexturingParamsCommand::read( const Tree & com, 
                                       CommandContext* context )
{
  vector<int>		id;
  void			*ptr = 0;
  set<AObject *>	obj;
  string		smode, sfilt, sgen, sinter;
  int			mode = -1;
  int			filt = -1;
  int			gen = -1;
  int			tex = 0;
  int			inter = -1;
  float			rate = -1;
  unsigned		i, n;
  vector<float>		genparam1, genparam2, genparam3;

  com.getProperty( "objects", id );
  try
  {
    tex = int( com.getProperty( "texture_index" )->getScalar() );
  } catch( ... ) {}

  for( i=0, n=id.size(); i<n; ++i )
    {
      ptr = context->unserial->pointer( id[i], "AObject" );
      if( !ptr )
        {
          cerr << "object id " << id[i] << " not found\n";
          return 0;
        }
      obj.insert( (AObject *) ptr );
    }

  try
  {
    smode = com.getProperty( "mode" )->getString();
  } catch( ... ) {}
  try
  {
    sfilt = com.getProperty( "filtering" )->getString();
  } catch( ... ) {}
  try
  {
    sgen = com.getProperty( "generation" )->getString();
  } catch( ... ) {}
  try
  {
    rate = com.getProperty( "rate" )->getScalar();
  } catch( ... ) {}
  try
  {
    sinter = com.getProperty( "interpolation" )->getString();
  } catch( ... ) {}
  com.getProperty( "generation_params_1", genparam1 );
  com.getProperty( "generation_params_2", genparam2 );
  com.getProperty( "generation_params_3", genparam3 );

  static map<string, GLComponent::glTextureMode>	modes;
  static map<string, GLComponent::glTextureFiltering>	filters;
  static map<string, GLComponent::glAutoTexturingMode>	gens;
  static map<string, bool>				inters;
  if( modes.empty() )
    {
      modes[ "geometric"                 ] = GLComponent::glGEOMETRIC;
      modes[ "linear"                    ] = GLComponent::glLINEAR;
      modes[ "replace"                   ] = GLComponent::glREPLACE;
      modes[ "decal"                     ] = GLComponent::glDECAL;
      modes[ "blend"                     ] = GLComponent::glBLEND;
      modes[ "add"                       ] = GLComponent::glADD;
      modes[ "combine"                   ] = GLComponent::glCOMBINE;
      modes[ "linear_on_nonnul"          ] = GLComponent::glLINEAR_A_IF_B;
      modes[ "linear_A_if_A_white"       ] = GLComponent::glLINEAR_A_IF_A;
      modes[ "linear_A_if_B_white"       ] = GLComponent::glLINEAR_A_IF_B;
      modes[ "linear_A_if_A_black"       ] = GLComponent::glLINEAR_A_IF_NOT_A;
      modes[ "linear_A_if_B_black"       ] = GLComponent::glLINEAR_A_IF_NOT_B;
      modes[ "linear_A_if_A_opaque"      ]
        = GLComponent::glLINEAR_A_IF_A_ALPHA;
      modes[ "linear_A_if_B_transparent" ]
        = GLComponent::glLINEAR_A_IF_NOT_B_ALPHA;
      modes[ "linear_B_if_A_white"       ] = GLComponent::glLINEAR_B_IF_A;
      modes[ "linear_B_if_B_white"       ] = GLComponent::glLINEAR_B_IF_B;
      modes[ "linear_B_if_A_black"       ] = GLComponent::glLINEAR_B_IF_NOT_A;
      modes[ "linear_B_if_B_black"       ] = GLComponent::glLINEAR_B_IF_NOT_B;
      modes[ "linear_B_if_B_opaque"      ]
        = GLComponent::glLINEAR_B_IF_B_ALPHA;
      modes[ "linear_B_if_A_transparent" ]
        = GLComponent::glLINEAR_B_IF_NOT_A_ALPHA;
      modes[ "max_channel"               ] = GLComponent::glMAX_CHANNEL;
      modes[ "min_channel"               ] = GLComponent::glMIN_CHANNEL;
      modes[ "max_opacity"               ] = GLComponent::glMAX_ALPHA;
      modes[ "min_opacity"               ] = GLComponent::glMIN_ALPHA;
      modes[ "geometric_sqrt"            ] = GLComponent::glGEOMETRIC_SQRT;
      modes[ "geometric_lighten"         ] = GLComponent::glGEOMETRIC_LIGHTEN;
      filters[ "nearest"     ] = GLComponent::glFILT_NEAREST;
      filters[ "linear"      ] = GLComponent::glFILT_LINEAR;
      gens[ "none"           ] = GLComponent::glTEX_MANUAL;
      gens[ "object_linear"  ] = GLComponent::glTEX_OBJECT_LINEAR;
      gens[ "eye_linear"     ] = GLComponent::glTEX_EYE_LINEAR;
      gens[ "sphere_map"     ] = GLComponent::glTEX_SPHERE_MAP;
      gens[ "reflection_map" ] = GLComponent::glTEX_REFLECTION_MAP;
      gens[ "normal_map"     ] = GLComponent::glTEX_NORMAL_MAP;
      inters[ "palette" ] = false;
      inters[ "rgb"     ] = true;
    }

  map<string, GLComponent::glTextureMode>::const_iterator	im;
  map<string, GLComponent::glTextureFiltering>::const_iterator	iflt;
  map<string, GLComponent::glAutoTexturingMode>::const_iterator	itg;
  map<string, bool>::const_iterator				iti;

  if( !smode.empty() )
    {
      im = modes.find( smode );
      if( im != modes.end() )
        mode = im->second;
    }
  if( !sfilt.empty() )
    {
      iflt = filters.find( sfilt );
      if( iflt != filters.end() )
        filt = iflt->second;
    }
  if( !sgen.empty() )
    {
      itg = gens.find( sgen );
      if( itg != gens.end() )
        gen = itg->second;
    }
  if( !sinter.empty() )
    {
      iti = inters.find( sinter );
      if( iti != inters.end() )
        inter = iti->second;
    }

  float	*g1 = 0, *g2 = 0, *g3 = 0;
  if( genparam1.size() >= 4 )
    g1 = &genparam1[0];
  if( genparam2.size() >= 4 )
    g2 = &genparam2[0];
  if( genparam3.size() >= 4 )
    g3 = &genparam3[0];

  return new TexturingParamsCommand( obj, (unsigned) tex, mode, filt, gen, 
                                     rate, inter, g1, g2, g3 );
}


void TexturingParamsCommand::write( Tree & com, Serializer* ser ) const
{
  Tree	*t = new Tree( true, name() );

  set<AObject *>::const_iterator	io, eo = _objects.end();
  vector<int>				ids;

  ids.reserve( _objects.size() );
  for( io=_objects.begin(); io!=eo; ++io )
    ids.push_back( ser->serialize( *io ) );

  t->setProperty( "objects", ids );

  if( _tex != 0 )
    t->setProperty( "texture_index", (int) _tex );

  if( _mode >= 0 && _mode <= GLComponent::glMIN_ALPHA )
    {
      static const string smode[] = 
        { "geometric", "linear", "replace", "decal", "blend", "add", "combine",
          "linear_A_if_A_white", "linear_A_if_B_white", "linear_A_if_A_black", "linear_A_if_B_black", "linear_B_if_A_white", " linear_B_if_B_white", "linear_B_if_A_black", "linear_B_if_B_black", "linear_A_if_A_opaque", "linear_A_if_B_transparent", "linear_B_if_B_opaque", "linear_B_if_A_transparent", "max_channel", "min_channel", "max_opacity", "min_opacity", "geometric_sqrt", "geometric_lighten",
        };
      t->setProperty( "mode", smode[_mode] );
    }

  if( _filter >= 0 && _filter < 2 )
    {
      static const string sfilt[] = 
        { "nearest", "linear" };
      t->setProperty( "filtering", sfilt[_filter] );
    }

  if( _gen >= 0 && _gen < 6 )
    {
      static const string sgen[] = 
        { "none", "object_linear", "eye_linear", "sphere_map", 
          "reflection_map", "normal_map"
        };
      t->setProperty( "generation", sgen[_gen] );
    }

  if( _rgbinter >= 0 && _rgbinter < 2 )
    {
      static const string sinter[] = 
        { "palette", "rgb", 
        };
      t->setProperty( "interpolation", sinter[ _rgbinter ] );
    }

  if( _rate >= 0 && _rate <= 1 )
    t->setProperty( "rate", _rate );

  if( _genparams_1.size() == 4 )
    t->setProperty( "generation_params_1", _genparams_1 );
  if( _genparams_2.size() == 4 )
    t->setProperty( "generation_params_2", _genparams_2 );
  if( _genparams_3.size() == 4 )
    t->setProperty( "generation_params_3", _genparams_3 );

  com.insert( t );
}


