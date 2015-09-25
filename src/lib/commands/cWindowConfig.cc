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

#include <anatomist/commands/cWindowConfig.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/window3D/window3D.h>
#include <anatomist/window/glwidget.h>
#include <anatomist/window/glwidgetmanager.h>
#include <anatomist/processor/Serializer.h>
#include <anatomist/processor/unserializer.h>
#include <anatomist/processor/Registry.h>
#include <anatomist/processor/context.h>
#include <anatomist/color/Light.h>
#include <graph/tree/tree.h>
#include <cartobase/object/syntax.h>
#include <vector>

using namespace anatomist;
using namespace carto;
using namespace std;


WindowConfigCommand::WindowConfigCommand( const set<AWindow *> & win, 
					  const GenericObject & config )
  : RegularCommand(), _windows( win ), 
    _config( config.clone() )
{
}


WindowConfigCommand::~WindowConfigCommand()
{
}


void WindowConfigCommand::write( Tree & com, Serializer* ser ) const
{
  Tree		*t = new Tree( true, name() );
  vector<int>	ids;
  set<AWindow *>::const_iterator	iw, fw = _windows.end();

  for( iw=_windows.begin(); iw!=fw; ++iw )
    ids.push_back( ser->serialize( *iw ) );

  t->copyProperties( _config );
  if( !ids.empty() )
    t->setProperty( "windows", ids );

  com.insert( t );
}


void WindowConfigCommand::doit()
{
  set<AWindow *>::iterator	iw, ew = _windows.end();
  vector<int>			size;
  vector<int>			geom;
  bool		linkon = false;
  bool		dosize = _config->getProperty( "view_size", size );
  bool		dogeom = _config->getProperty( "geometry", geom );
  bool		dolink = false;
  try
    {
      Object	olinkon 
        = _config->getProperty( "linkedcursor_on_slider_change" );
      if( olinkon )
        {
          linkon = (bool) olinkon->getScalar();
          dolink = true;
        }
    }
  catch( ... )
    {
    }
  int		recmode = -1;
  string	recbase;
  AWindow	*w;
  AWindow3D	*w3;
  GLWidgetManager	*v;
  string	polymode;
  AWindow3D::RenderingMode	ipolmode = AWindow3D::Normal;
  int		persp, zbuf, cull, flat, filt, fog, polysort, clip;
  bool		bpolmode = false, bpersp = false, bzbuf = false, 
    bcull = false, bflat = false, bfilt = false, bfog = false, bclip = false, 
    bclipd = false, bcurs, bpolysort = false;
  float		clipd;
  int		raise = 0, icon = 0, hascursor = -1;
  string	snap;
  int           showtoolbars = -1, showcursorpos = -1, fullscreen = -1;
  vector<string>	snapfiles;

  _config->getProperty( "record_mode", recmode );
  _config->getProperty( "record_basename", recbase );
  if( _config->getProperty( "polygon_mode", polymode ) )
    {
      bpolmode = true;
      if( polymode == "normal" )
        ipolmode = AWindow3D::Normal;
      else if( polymode == "wireframe" )
        ipolmode = AWindow3D::Wireframe;
      else if( polymode == "outline" )
        ipolmode = AWindow3D::Outlined;
      else if( polymode == "hiddenface_wireframe" )
        ipolmode = AWindow3D::HiddenWireframe;
      else
        bpolmode = false;
    }
  bpersp = _config->getProperty( "perspective", persp );
  bzbuf = _config->getProperty( "transparent_depth_buffer", zbuf );
  bcull = _config->getProperty( "face_culling", cull );
  bflat = _config->getProperty( "flat_shading", flat );
  bfilt = _config->getProperty( "polygon_filtering", filt );
  bfog = _config->getProperty( "fog", fog );
  bpolysort = _config->getProperty( "polygons_depth_sorting", polysort );
  bclip = _config->getProperty( "clipping", clip );
  if( clip > 2 )
    clip = 2;
  bclipd = _config->getProperty( "clip_distance", clipd );
  _config->getProperty( "raise", raise );
  _config->getProperty( "iconify", icon );
  _config->getProperty( "snapshot", snap );
  if( !snap.empty() )
    {
      if( _windows.size() == 1 )
        snapfiles.push_back( snap );
      else
        {
          istringstream	ss( snap );
          while( !ss.eof() )
            {
              string	s;
              ss >> s;
              snapfiles.push_back( s );
            }
        }
    }
  unsigned	nsnap = snapfiles.size(), i = 0;
  bcurs = _config->getProperty( "cursor_visibility", hascursor );
  _config->getProperty( "show_toolbars", showtoolbars );
  _config->getProperty( "show_cursor_position", showcursorpos );
  _config->getProperty( "fullscreen", fullscreen );
  Object light;
  try
  {
    light = _config->getProperty( "light" );
  }
  catch( ... )
  {
  }

  for( iw=_windows.begin(); iw!=ew; ++iw )
    {
      w = *iw;
      if( dogeom )
        {
          if( dosize || geom.size() < 4 )
            w->setGeometry( geom[0], geom[1], 0, 0 );
          else
            w->setGeometry( geom[0], geom[1], geom[2], geom[3] );
        }

      if( raise )
        w->show();
      if( icon )
        w->iconify();
      if( bcurs )
        w->setHasCursor( hascursor );
      if( showtoolbars >= 0 )
        w->showToolBars( showtoolbars );
      if( fullscreen >= 0 )
        w->setFullScreen( fullscreen );

      w3 = dynamic_cast<AWindow3D *>( w );
      if( w3 )
        {
          if( bpolmode )
            w3->setRenderingMode( ipolmode );
          if( bpersp )
            w3->enablePerspective( (bool) persp );
          if( bzbuf )
            w3->enableTransparentZ( (bool) zbuf );
          if( bcull )
            w3->setCulling( (bool) cull );
          if( bflat )
            w3->setFlatShading( (bool) flat );
          if( bfilt )
            w3->setSmoothing( (bool) filt );
          if( bfog )
            w3->setFog( (bool) fog );
          if( bpolysort )
            w3->setPolygonsSortingEnabled( (bool) polysort );
          if( bclip )
            w3->setClipMode( (AWindow3D::ClipMode) clip );
          if( bclipd )
            w3->setClipDistance( clipd );

          if( dosize )
            w3->resizeView( size[0], size[1] );

          if( showcursorpos >= 0 )
            w3->showStatusBar( showcursorpos );

          if( !light.isNull() )
            w3->light()->set( light );

          v = dynamic_cast<GLWidgetManager *>( w3->view() );
          if( v )
            {
              if( recmode != -1 )
              {
                if( recmode )
                  {
                    if( !recbase.empty() )
                      v->recordStart( recbase.c_str() );
                  }
                else
                  v->recordStop();
              }
            }

          if( bpolmode || bpersp || bzbuf || bcull || bflat || bfilt || bfog
              || bpolysort || bclip || bclipd || bcurs || !light.isNull() )
            {
              w3->setChanged();
              w3->notifyObservers( w3 );
              if( !v || i >= nsnap )
                w3->Refresh();
            }

          if( v && i < nsnap )
            {
              w3->refreshNow();
              v->saveContents( snapfiles[i].c_str(), QString::null );
              ++i;
            }
          if( dolink )
            w3->setLinkedCursorOnSliderChange( linkon );
        }
      else
        (*iw)->Refresh();
    }
}


Command* WindowConfigCommand::read( const Tree & com, CommandContext* context )
{
  vector<int>		winId;
  void			*ptr = 0;
  set<AWindow *>	win;
  unsigned		i, n;

  com.getProperty( "windows", winId );

  for( i=0, n=winId.size(); i<n; ++i )
    {
      ptr = context->unserial->pointer( winId[i], "AWindow" );
      if( ptr )
        win.insert( (AWindow *) ptr );
      else
        cerr << "window id " << winId[i] << " not found\n";
    }

  return( new WindowConfigCommand( win, com ) );
}


bool WindowConfigCommand::initSyntax()
{
  SyntaxSet	ss;
  Syntax	& s = ss[ "WindowConfig" ];

  s[ "windows"                       ] = Semantic( "int_vector", true );
  s[ "geometry"                      ].type = "int_vector";
  s[ "view_size"                     ].type = "int_vector";
  s[ "record_mode"                   ].type = "int";
  s[ "record_basename"               ].type = "string";
  s[ "polygon_mode"                  ].type = "string";
  s[ "perspective"                   ].type = "int";
  s[ "transparent_depth_buffer"      ].type = "int";
  s[ "face_culling"                  ].type = "int";
  s[ "flat_shading"                  ].type = "int";
  s[ "polygon_filtering"             ].type = "int";
  s[ "fog"                           ].type = "int";
  s[ "polygons_depth_sorting"        ].type = "int";
  s[ "clipping"                      ].type = "int";
  s[ "clip_distance"                 ].type = "float";
  s[ "raise"                         ].type = "int";
  s[ "iconify"                       ].type = "int";
  s[ "snapshot"                      ].type = "string";
  s[ "linkedcursor_on_slider_change" ] = Semantic( "int" );
  s[ "cursor_visibility"             ] = Semantic( "int" );
  s[ "show_toolbars"                 ] = Semantic( "int" );
  s[ "show_cursor_position"          ] = Semantic( "int" );
  s[ "fullscreen"                    ] = Semantic( "int" );
  s[ "light"                         ] = Semantic( "dictionary" );

  Registry::instance()->add( "WindowConfig", &read, ss );
  return( true );
}

