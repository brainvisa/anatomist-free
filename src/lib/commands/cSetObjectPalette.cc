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


#include <anatomist/commands/cSetObjectPalette.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/object/Object.h>
#include <anatomist/color/objectPalette.h>
#include <anatomist/color/paletteList.h>
#include <anatomist/processor/Registry.h>
#include <anatomist/processor/unserializer.h>
#include <anatomist/processor/Serializer.h>
#include <anatomist/processor/context.h>
#include <anatomist/surface/glcomponent.h>
#include <cartobase/object/syntax.h>
#include <graph/tree/tree.h>

using namespace anatomist;
using namespace carto;
using namespace std;


SetObjectPaletteCommand::SetObjectPaletteCommand( const set<AObject *> & obj,
						  const string & palname1, 
						  bool min1flg, 
						  float min1, bool max1flg, 
						  float max1, 
						  const string & palname2,
						  bool min2flg, 
						  float min2, bool max2flg, 
						  float max2, 
						  const string & mixMethod, 
						  bool mixFacFlg, 
						  float linMixFactor, 
						  const string & pal1Dmapping,
                                                  bool absmode, int sizex,
                                                  int sizey )
  : RegularCommand(), _objL( obj ), _pal1( palname1 ), _pal2( palname2 ),
    _pal1Dmapping(pal1Dmapping),
    _min1( min1 ), _max1( max1 ), _min2( min2 ), _max2( max2 ), 
    _mixMethod( mixMethod ), _linMixFactor( linMixFactor ), 
    _min1flg( min1flg ), _max1flg( max1flg ), _min2flg( min2flg ), 
    _max2flg( max2flg ), _mixFacFlg( mixFacFlg ), _absmode( absmode ),
    _sizex( sizex ), _sizey( sizey )
{
}


SetObjectPaletteCommand::~SetObjectPaletteCommand()
{
}


bool SetObjectPaletteCommand::initSyntax()
{
  SyntaxSet	ss;
  Syntax	& s = ss[ "SetObjectPalette" ];
  
  s[ "objects"          ] = Semantic( "int_vector", true );
  s[ "palette"          ] = Semantic( "string", false );
  s[ "palette2"         ] = Semantic( "string", false );
  s[ "palette1Dmapping" ] = Semantic( "string", false );
  s[ "min"              ] = Semantic( "float", false );
  s[ "max"              ] = Semantic( "float", false );
  s[ "min2"             ] = Semantic( "float", false );
  s[ "max2"             ] = Semantic( "float", false );
  s[ "mixMethod"        ] = Semantic( "string", false );
  s[ "linMixFactor"     ] = Semantic( "float", false );
  s[ "absoluteMode"     ] = Semantic( "int", false );
  s[ "sizex"            ] = Semantic( "int", false );
  s[ "sizey"            ] = Semantic( "int", false );
  Registry::instance()->add( "SetObjectPalette", &read, ss );
  return( true );
}


void SetObjectPaletteCommand::doit()
{
  set<AObject *>::const_iterator	io, fo = _objL.end();
  rc_ptr<APalette>			p1, p2;
  const PaletteList			& pall = theAnatomist->palettes();
  AObject				*o;

  if( !_pal1.empty() )
    {
      p1 = pall.find( _pal1 );
      if( !p1 )
      {
        cerr << "SetObjectPaletteCommand : warning: palette \"" << _pal1
          << "\" not found\n";
        return;
      }
    }

  if( !_pal2.empty() )
    p2 = pall.find( _pal2 );

  for( io=_objL.begin(); io!=fo; ++io )
    if( theAnatomist->hasObject( *io ) )
      {
	o = *io;

	if( _pal1.empty() )
	  p1 = o->getOrCreatePalette()->refPalette();

	AObjectPalette	pal( p1 );

	if( p2 )
	  pal.setRefPalette2( p2 ) ;
	else
	  pal.setRefPalette2( o->getOrCreatePalette()->refPalette2() );
	
	if( !_pal1Dmapping.empty() )
	  pal.setPalette1DMappingName( _pal1Dmapping ) ;
	else
	  pal.setPalette1DMappingName( o->palette()->palette1DMappingName() ) ;
	if( _pal1Dmapping == "Diagonal" )
	  pal.set2dMode( true ) ;
        bool absmode = false;
        if( _absmode )
        {
          GLComponent *glc = o->glAPI();
          if( glc )
          {
            GLComponent::TexExtrema	& te = glc->glTexExtrema();
            absmode = true;
            float m0 = te.minquant[0];
            float scl0;
            if( te.maxquant[0] != m0 )
              scl0 = 1. / (te.maxquant[0] - m0);
            else
              scl0 = 1.;
            if( _min1flg )
              pal.setMin1( ( _min1 - m0 ) * scl0 );
            else
              pal.setMin1( o->palette()->min1() );
            if( _max1flg )
            {
              pal.setMax1( ( _max1 - m0 ) * scl0 );
            }
            else
              pal.setMax1( o->palette()->max1() );
            float m1 = 0, scl1 = 1;
            if( te.minquant.size() >= 2 )
            {
              m1 = te.minquant[1];
              if( te.maxquant[1] != m1 )
                scl1 = 1. / (te.maxquant[1] - m1);
              else
                scl1 = 1.;
              if( _min2flg )
                pal.setMin2( ( _min2 - m1 ) * scl1 );
              else
                pal.setMin2( o->palette()->min2() );
              if( _max2flg )
                pal.setMax2( ( _max2 - m1 ) * scl1 );
              else
                pal.setMax2( o->palette()->max2() );
            }
          }
        }
        if( !absmode )
        {
          if( _min1flg )
            pal.setMin1( _min1 );
          else
            pal.setMin1( o->palette()->min1() );
          if( _max1flg )
            pal.setMax1( _max1 );
          else
            pal.setMax1( o->palette()->max1() );
          if( _min2flg )
            pal.setMin2( _min2 );
          else
            pal.setMin2( o->palette()->min2() );
          if( _max2flg )
            pal.setMax2( _max2 );
          else
            pal.setMax2( o->palette()->max2() );
        }
        if( !_mixMethod.empty() )
          pal.setMixMethod( _mixMethod );
        else
          pal.setMixMethod( o->palette()->mixMethodName() );
        if( _mixFacFlg )
          pal.setLinearMixFactor( _linMixFactor );
        else
          pal.setLinearMixFactor( o->palette()->linearMixFactor() );
        int sx = _sizex, sy = _sizey;
        if( sx == -2 )
        {
          sx = o->palette()->glMaxSizeX();
        }
        if( sy == -2 )
          sy = o->palette()->glMaxSizeY();
        o->palette()->glSetMaxSize( sx, sy );
        // TODO: incomplete
        o->palette()->setMaxSize( -1, -1 );
        o->setPalette( pal );
        o->notifyObservers( o );
      }
}


Command* SetObjectPaletteCommand::read( const Tree & com, 
					CommandContext* context )
{
  vector<int>		obj;
  set<AObject *>	objL;
  unsigned		i, n;
  void			*ptr;
  string		pal1, pal2, pal1Dmapping, mix;
  float			min1 = -1, min2 = -1, max1 = -1, max2 = -1;
  bool			min1f = false, max1f = false, min2f = false, 
    max2f = false, mixf = false;
  float			linmix = -1;
  int                   absmode = 0, sizex = 0, sizey = 0;

  if( !com.getProperty( "objects", obj ) )
    return( 0 );

  for( i=0, n=obj.size(); i<n; ++i )
    {
      ptr = context->unserial->pointer( obj[i], "AObject" );
      if( ptr )
	objL.insert( (AObject *) ptr );
      else
	{
	  cerr << "object id " << obj[i] << " not found\n";
	  return( 0 );
	}
    }

  com.getProperty( "palette", pal1 );
  com.getProperty( "palette2", pal2 );
  if( com.getProperty( "min", min1 ) )
    min1f = true;
  if( com.getProperty( "max", max1 ) )
    max1f = true;
  if( com.getProperty( "min2", min2 ) )
    min2f = true;
  if( com.getProperty( "max2", max2 ) )
    max2f = true;
  com.getProperty( "mixMethod", mix );
  com.getProperty( "palette1Dmapping", pal1Dmapping );
  if( com.getProperty( "linMixFactor", linmix ) )
    mixf = true;
  com.getProperty( "absoluteMode", absmode );
  com.getProperty( "sizex", sizex );
  com.getProperty( "sizey", sizey );

  return( new SetObjectPaletteCommand( objL, pal1, min1f, min1, max1f, max1, 
				       pal2, min2f, min2, max2f, max2, mix, 
				       mixf, linmix, pal1Dmapping,
                                       absmode != 0, sizex, sizey ) );
}


void SetObjectPaletteCommand::write( Tree & com, Serializer* ser ) const
{
  Tree	*t = new Tree( true, name() );

  set<AObject *>::const_iterator	io;
  vector<int>				obj;

  for( io=_objL.begin(); io!=_objL.end(); ++io ) 
    obj.push_back( ser->serialize( *io ) );

  t->setProperty( "objects", obj );
  if( !_pal1.empty() )
    t->setProperty( "palette", _pal1 );
  if( _min1flg )
    t->setProperty( "min", _min1 );
  if( _min1flg )
    t->setProperty( "min", _min1 );
  if( _max1flg )
    t->setProperty( "max", _max1 );
  if( _min2flg )
    t->setProperty( "min2", _min2 );
  if( _max2flg )
    t->setProperty( "max2", _max2 );
  if( !_pal2.empty() )
    t->setProperty( "palette2", _pal2 );
  if( !_pal1Dmapping.empty() )
    t->setProperty( "palette1Dmapping", _pal1Dmapping );
  if( !_mixMethod.empty() )
    t->setProperty( "mixMethod", _mixMethod );
  if( _mixFacFlg )
    t->setProperty( "linMixFactor", _linMixFactor );
  if( _absmode )
    t->setProperty( "absoluteMode", 1 );
  if( _sizex != 0 )
    t->setProperty( "sizex", _sizex );
  if( _sizey != 0 )
    t->setProperty( "sizey", _sizey );
  com.insert( t );
}
