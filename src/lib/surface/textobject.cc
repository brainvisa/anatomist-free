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
  Private( const Point3df & pos );
  ~Private();

  string text;
  GLfloat texcoords[8];
  QFont* font;
  float scale;
  Point3df pos;
};


TextObject::Private::Private( const Point3df & pos )
  : font( 0 ), scale( 1. ), pos( pos )
{
  texcoords[0] = 0;
  texcoords[1] = 1;
  texcoords[2] = 0;
  texcoords[3] = 0;
  texcoords[4] = 1;
  texcoords[5] = 0;
  texcoords[6] = 1;
  texcoords[7] = 1;
}


TextObject::Private::~Private()
{
  delete font;
}


TextObject::TextObject( const std::string & text,
                        const Point3df & pos )
  : ASurface<3>(), d( new Private( pos ) )
{
  AimsTimeSurface<3> *mesh = new AimsTimeSurface<3>();
  vector<Point3df> &vert = mesh->vertex();
  vector<AimsVector<uint32_t, 3> > & poly= mesh->polygon();
  vert.reserve( 4 );
  vert.push_back( pos + Point3df( 0, 0, 0 ) );
  vert.push_back( pos + Point3df( 0, -1, 0 ) );
  vert.push_back( pos + Point3df( 1, -1, 0 ) );
  vert.push_back( pos + Point3df( 1, 0, 0 ) );
  poly.reserve( 2 );
  poly.push_back( AimsVector<uint32_t, 3>( 0, 1, 2 ) );
  poly.push_back( AimsVector<uint32_t, 3>( 0, 2, 3 ) );
  mesh->updateNormals();
  setSurface( mesh );
  GetMaterial().setRenderProperty( Material::RenderFaceCulling, 0 );
  GetMaterial().setRenderProperty( Material::RenderZBuffer, 0 );
  glAddTextures( 1 );
  TexExtrema  & te = GLComponent::glTexExtrema( 0 );
  te.min.push_back( 0 );
  te.max.push_back( 1 );
  te.minquant.push_back( 0 );
  te.maxquant.push_back( 1 );
  glSetTexMode( glREPLACE, 0 );
  glSetTexFiltering( glFILT_LINEAR, 0 );
  GetMaterial().SetDiffuse( 0, 0, 0, 1 );
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

  const QFont *font = d->font;
  auto_ptr<QFont> fontdel( 0 );
  if( !font )
  {
    fontdel.reset( new QFont( qApp->font().family(), 30 ) );
    font = fontdel.get();
  }
  QFontMetrics fm( *font );
  QRect br = fm.boundingRect( QRect( QPoint( 0, 0 ), QSize( 1000, 1000 ) ),
                              Qt::AlignLeft, d->text.c_str() );
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

  vector<Point3df> &vert = surface()->vertex();
  vert[0] = d->pos;
  vert[1] = d->pos;
  vert[2] = d->pos;
  vert[3] = d->pos;
  vert[1][1] -= sz.height() * d->scale;
  vert[2][0] += sz.width() * d->scale;
  vert[2][1] -= sz.height() * d->scale;
  vert[3][0] += sz.width() * d->scale;
  d->texcoords[1] = float(sz.height())/dimy;
  d->texcoords[4] = float(sz.width())/dimx;
  d->texcoords[6] = float(sz.width())/dimx;
  d->texcoords[7] = float(sz.height())/dimy;
  glSetChanged( glGEOMETRY );
  UpdateMinAndMax();
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

  glBindTexture( GL_TEXTURE_2D, texName );
  status = glGetError();
  if( status != GL_NO_ERROR )
  {
    cerr << "TextObject::glMakeTexImage : OpenGL error: "
        << gluErrorString(status) << endl;
    return false;
  }
  glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

  QFont *font = d->font;
  auto_ptr<QFont> fontdel( 0 );
  if( !font )
  {
    fontdel.reset( new QFont( qApp->font().family(), 30 ) );
    font = fontdel.get();
  }
  QFontMetrics fm( *font );
  QRect br = fm.boundingRect( QRect( QPoint( 0, 0 ), QSize( 0, 0 ) ),
                              Qt::AlignLeft, d->text.c_str() );
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

  QImage im( QSize( dimx, dimy ), QImage::Format_ARGB32 );
  im.fill( qRgba( 255, 255, 255, 0 ) );
  QPainter p( &im );
  const Material & mat = material();
  p.setPen( QPen( QColor( int( mat.Diffuse(0) * 255.99 ),
                          int( mat.Diffuse(1) * 255.99 ),
                          int( mat.Diffuse(2) * 255.99 ),
                          int( mat.Diffuse(3) * 255.99 ) ) ) );
  p.setFont( *font );
  p.drawText( br, Qt::AlignLeft, d->text.c_str() );
  p.end();

  glTexImage2D( GL_PROXY_TEXTURE_2D, 0, 4, dimx, dimy, 0, GL_BGRA,
                GL_UNSIGNED_BYTE, 0 );
  GLint w;
  glGetTexLevelParameteriv( GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w );
  if( w == 0 || status != GL_NO_ERROR )
  {
    cout << "texture too large.\n";
    return false;
  }

  glTexImage2D( GL_TEXTURE_2D, 0, 4, dimx, dimy, 0, GL_BGRA,
                GL_UNSIGNED_BYTE, (GLvoid *) im.bits() );
  return true;
}


void TextObject::setFont( QFont* font )
{
  delete d->font;
  d->font = font;
}


QFont* TextObject::font()
{
  return d->font;
}


const QFont* TextObject::font() const
{
  return d->font;
}


void TextObject::setScale( float s )
{
  if( s == 0. )
    return;
  if( d->scale != s )
  {
    vector<Point3df> &vert = surface()->vertex();
    float scf = s / d->scale;
    float w = ( vert[2][0] - vert[0][0] ) * scf;
    float h = ( vert[1][1] - vert[0][0] ) * scf;
    d->scale = s;
    vert[0] = d->pos;
    vert[1] = d->pos;
    vert[2] = d->pos;
    vert[3] = d->pos;
    vert[1][1] += h;
    vert[2][0] += w;
    vert[2][1] += h;
    vert[3][0] += w;
    glSetChanged( glGEOMETRY );
    UpdateMinAndMax();
  }
}


float TextObject::scale() const
{
  return d->scale;
}


void TextObject::glSetChanged( glPart part, bool x ) const
{
  ASurface<3>::glSetChanged( part, x );
  if( x && part == glMATERIAL )
    glSetTexImageChanged( true, 0 );
}


void TextObject::setPosition( const Point3df & pos )
{
  vector<Point3df> &vert = surface()->vertex();
  Point3df oldpos = vert[0];
  d->pos = pos;
  Point3df delta = pos - oldpos;
  vert[0] += delta;
  vert[1] += delta;
  vert[2] += delta;
  vert[3] += delta;
  glSetChanged( glGEOMETRY );
  UpdateMinAndMax();
}


Point3df TextObject::position() const
{
  return d->pos;
}



