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

#include <anatomist/surface/textobject.h>
#include <aims/mesh/surface.h>
#include <qfont.h>
#include <qfontmetrics.h>
#include <qimage.h>
#include <qpainter.h>
#include <qapplication.h>

using namespace anatomist;
using namespace aims;
using namespace std;


struct TextObject::Private
{
  Private();

  string text;
  GLfloat texcoords[8];
};


TextObject::Private::Private()
{
  texcoords[0] = 0;
  texcoords[1] = 0;
  texcoords[2] = 0;
  texcoords[3] = 1;
  texcoords[4] = 1;
  texcoords[5] = 1;
  texcoords[6] = 1;
  texcoords[7] = 0;
}


TextObject::TextObject( const std::string & text )
  : ASurface<3>(), d( new Private )
{
  AimsTimeSurface<3> *mesh = new AimsTimeSurface<3>();
  vector<Point3df> &vert = mesh->vertex();
  vector<AimsVector<uint32_t, 3> > & poly= mesh->polygon();
  vert.reserve( 4 );
  vert.push_back( Point3df( 0, 0, 0 ) );
  vert.push_back( Point3df( 0, 0, 1 ) );
  vert.push_back( Point3df( 1, 0, 1 ) );
  vert.push_back( Point3df( 1, 0, 0 ) );
  poly.reserve( 2 );
  poly.push_back( AimsVector<uint32_t, 3>( 0, 1, 2 ) );
  poly.push_back( AimsVector<uint32_t, 3>( 0, 2, 3 ) );
  mesh->updateNormals();
  setSurface( mesh );
  GetMaterial().setRenderProperty( Material::RenderFaceCulling, 0 );
  glAddTextures( 1 );
  TexExtrema  & te = GLComponent::glTexExtrema( 0 );
  te.min.push_back( 0 );
  te.max.push_back( 1 );
  te.minquant.push_back( 0 );
  te.maxquant.push_back( 1 );
  setText( text );
}


TextObject::~TextObject()
{
  delete d;
}


const string & TextObject::text() const
{
  return d->text;
}


void TextObject::setText( const std::string & text )
{
  d->text = text;
  glSetTexImageChanged( 0 );
  glSetChanged( glBODY );
}


unsigned TextObject::glDimTex( const ViewState &, unsigned ) const
{
  return 2;
}


unsigned TextObject::glTexCoordSize( const ViewState &, unsigned ) const
{
  return 4;
}


const GLfloat* TextObject::glTexCoordArray( const ViewState &, unsigned ) const
{
  return d->texcoords;
}


bool TextObject::glMakeTexImage( const ViewState &, const GLTexture & gltex,
                                 unsigned ) const
{
  GLuint texName = gltex.item();
  GLCaps::glActiveTexture( GLCaps::textureID( 0 ) );
  GLenum status = glGetError();
  if( status != GL_NO_ERROR )
  {
    cerr << "TextObject::glMakeTexImage : OpenGL error: "
        << gluErrorString(status) << endl;
    return false;
  }

  cout << "TextObject::glMakeTexImage\n";

  QFont font( qApp->font().family(), 30 );
  QFontMetrics fm( font );
  QRect br = fm.tightBoundingRect( d->text.c_str() );
  QSize sz = br.size();
  unsigned      dimx = sz.width(), dimy = sz.height(), x, utmp;
  for( x=0, utmp=1; x<32 && utmp<dimx; ++x )
    utmp = utmp << 1;
  dimx = utmp;
  for( x=0, utmp=1; x<32 && utmp<dimy; ++x )
    utmp = utmp << 1;
  dimy = utmp;
  if( dimx == 0 )
    dimx = 1;
  if( dimy == 0 )
    dimy = 1;

  QImage im( sz, QImage::Format_ARGB32 );
  im.fill( qRgba( 255, 255, 255, 0 ) );
  QPainter p( &im );
  p.setFont( font );
  p.drawText( QPoint( -br.x(), -br.y() ), d->text.c_str() );
  p.end();

  glTexImage2D( GL_PROXY_TEXTURE_2D, 0, 4, dimx, dimy, 0, GL_RGBA,
                GL_FLOAT, 0 );
  GLint w;
  glGetTexLevelParameteriv( GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w );
  if( w == 0 || status != GL_NO_ERROR )
  {
    cout << "texture too large.\n";
    return false;
  }

  glTexImage2D( GL_TEXTURE_2D, 0, 4, dimx, dimy, 0, GL_RGBA,
                GL_FLOAT, (GLvoid *) im.bits() );

  return true;
}


