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


#if defined(_WS_X11_) || defined(Q_WS_X11)
#define private public
#define protected public
#endif
#include <qgl.h>
#if defined(_WS_X11_) || defined(Q_WS_X11)
#undef protected
#undef private
#endif
#include <iostream>

using namespace std;


namespace carto
{


  class GLContext : public QGLContext
  {
  public:
    GLContext( const QGLFormat& format, QPaintDevice* device );
    virtual ~GLContext();
#if defined(_WS_X11_) || defined(Q_WS_X11)
    virtual void *tryVisual( const QGLFormat& f, int bufDepth = 1 );
#endif
  };


}

using namespace carto;


GLContext::GLContext( const QGLFormat& format, QPaintDevice* device )
  : QGLContext( format, device )
{
}


GLContext::~GLContext()
{
}


#if defined(_WS_X11_) || defined(Q_WS_X11)
#include <GL/glx.h>
#include <X11/Xlib.h>


void *GLContext::tryVisual( const QGLFormat& f, int bufDepth )
{
    XVisualInfo* info = (XVisualInfo*)QGLContext::tryVisual( f, bufDepth );

#if defined(_OS_SOLARIS_) || defined (Q_OS_SOLARIS)
    // qDebug( "default visual id: %ld", info->visualid );

    /* locate the visuals with the same class as 'info' */
    XVisualInfo r_info;
    r_info.c_class = info->c_class;
    r_info.screen = device()->x11Screen();
    long info_mask = VisualClassMask | VisualScreenMask;
    int visuals;
    XVisualInfo* info_list = XGetVisualInfo( device()->x11Display(),
					     info_mask, &r_info, &visuals );

    /* find similar visuals, stop on first match */
    int	i;
    for ( i = 0; i < visuals; ++i )
      {
	// qDebug( "\ttrying visual %ld", info_list[i].visualid );
	if ( info_list[i].depth == info->depth &&
	     info_list[i].c_class == info->c_class &&
	     info_list[i].red_mask == info->red_mask &&
	     info_list[i].green_mask == info->green_mask &&
	     info_list[i].blue_mask == info->blue_mask &&
	     info_list[i].colormap_size == info->colormap_size &&
	     info_list[i].bits_per_rgb == info->bits_per_rgb )
	  {
	    *info = info_list[i];
	    break;
	  }
      }

    /* found a match */
    if ( i < visuals ) {
	/* replace old visual with new similar one */
	*info = info_list[i];
    }

    XFree( info_list );

    // qDebug( "actual visual id: %ld", info->visualid );
#endif

    return info;
}


#endif


#include "anatomist/window/glcontext.h"


GLWidget::GLWidget( QWidget* parent, const char* name,
		    const QGLWidget* shareWidget, Qt::WFlags f )
  : QGLWidget( parent, name, shareWidget, f )
{
    QGLFormat format = QGLFormat::defaultFormat();
    if ( shareWidget )
	setContext( new GLContext( format, this ), shareWidget->context() );
    else
	setContext( new GLContext( format, this ) );
    setBackgroundMode( Qt::NoBackground );
}


GLWidget::GLWidget( const QGLFormat& format, QWidget* parent, 
		    const char* name, 
		    const QGLWidget* shareWidget, Qt::WFlags f )
  : QGLWidget( format, parent, name, shareWidget, f )
{
    if ( shareWidget )
	setContext( new GLContext( format, this ), 
		    shareWidget->context() );
    else
      setContext( new GLContext( format, this ) );
    setBackgroundMode( Qt::NoBackground );
}


GLWidget::~GLWidget()
{
}
