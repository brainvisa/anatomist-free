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

#include <anatomist/surface/tesselatedmesh.h>
#include <anatomist/reference/Transformation.h>
#include <anatomist/reference/transformobserver.h>
#include <anatomist/reference/Referential.h>
#include <graph/tree/tree.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/fusion/defFusionMethods.h>
#include <anatomist/surface/triangulated.h>
#include <anatomist/object/actions.h>
#include <aims/mesh/surfaceOperation.h>
#include <anatomist/control/qObjTree.h>
#include <anatomist/application/settings.h>
#include <qpixmap.h>
#include <qtranslator.h>


using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;


int TesselatedMesh::registerClass()
{
  int   type = registerObjectType( "TesselatedMesh" );
  return type;
}


struct TesselatedMesh::Private
{
  Private() : tesselator( 0 ), polytess( 0 ),
  vertices( 0 ), polygons( 0 ), nvert( 0 ), polytype( 0 ), count( 0 ),
  index0( 0 ), index1( 0 ), odd( false )
  {}

  GLUtesselator *tesselator;
  GLUtesselator *polytess;
  vector<Point3df> *vertices;
  vector<AimsVector<uint,3> > *polygons;
  unsigned nvert;
  list<Point3df> addedVertices;
  GLuint polytype;
  unsigned count;
  unsigned index0;
  unsigned index1;
  bool odd;
};


TesselatedMesh::TesselatedMesh( const vector<AObject *> & obj )
  : GLObjectVector(), d( new TesselatedMesh::Private )
{
  _type = classType();

  if( QObjectTree::TypeNames.find( _type ) == QObjectTree::TypeNames.end() )
  {
    string str = Settings::globalPath() + "/icons/list_tesselatedmesh.xpm";
    if( !QObjectTree::TypeIcons[ _type ].load( str.c_str() ) )
    {
      QObjectTree::TypeIcons.erase( _type );
      cerr << "Icon " << str.c_str() << " not found\n";
    }

    QObjectTree::TypeNames[ _type ] = "Tesselated Mesh";
  }

  vector<AObject *>::const_iterator     io, fo=obj.end();
  ViewState vs;
  ATriangulated *surf = new ATriangulated;
  surf->setName( theAnatomist->makeObjectName( "TessSubMesh" ) );
  theAnatomist->registerObject( surf, false );
  surf->setSurface( new AimsSurfaceTriangle );
  insert( surf );
  AObject       *s = 0;

  for( io=obj.begin(); io!=fo; ++io )
  {
    GLComponent *gc = (*io)->glAPI();
    if( gc )
    {
      if( !s )
        s = *io;
      if( gc->glPolygonSize( vs ) == 2 )
        insert( *io );
    }
  }

  if( s )
  {
    setReferentialInheritance( s );
    surf->setReferentialInheritance( s );
    Material & mat = surf->GetMaterial();
    mat.setRenderProperty( Material::RenderFaceCulling, 0 );
    // surf->SetMaterial( s->GetMaterial() );
  }
}


TesselatedMesh::~TesselatedMesh()
{
  if( d->tesselator )
  {
    gluDeleteTess( d->polytess );
    gluDeleteTess( d->tesselator );
  }
  iterator      i = begin();
  AObject *o = *i;
  erase( i );
  if( o->Parents().empty() )
    delete o;
  delete d;
}


int TesselatedMesh::classType()
{
  static int    _classType = registerClass();
  return _classType;
}


const AObject* TesselatedMesh::tesselatedMesh() const
{
  return *begin();
}


AObject* TesselatedMesh::tesselatedMesh()
{
  return *begin();
}


const AObject* TesselatedMesh::firstPolygon() const
{
  const_iterator i = begin();
  ++i;
  return *i;
}


AObject* TesselatedMesh::firstPolygon()
{
  iterator i = begin();
  ++i;
  return *i;
}


GLComponent* TesselatedMesh::glGeometry()
{
  return glAPI();
}


GLComponent* TesselatedMesh::glTexture( unsigned )
{
  return tesselatedMesh()->glAPI();
}


const GLComponent* TesselatedMesh::glGeometry() const
{
  return glAPI();
}


const GLComponent* TesselatedMesh::glTexture( unsigned ) const
{
  return tesselatedMesh()->glAPI();
}


unsigned TesselatedMesh::glNumVertex( const ViewState & vs ) const
{
  tesselate( vs );
  return tesselatedMesh()->glAPI()->glNumVertex( vs );
}


const GLfloat* TesselatedMesh::glVertexArray( const ViewState & vs ) const
{
  tesselate( vs );
  return tesselatedMesh()->glAPI()->glVertexArray( vs );
}


const GLfloat* TesselatedMesh::glNormalArray( const ViewState & vs ) const
{
  tesselate( vs );
  return tesselatedMesh()->glAPI()->glNormalArray( vs );
}


unsigned TesselatedMesh::glPolygonSize( const ViewState & vs ) const
{
  tesselate( vs );
  return tesselatedMesh()->glAPI()->glPolygonSize( vs );
}


unsigned TesselatedMesh::glNumPolygon( const ViewState & vs ) const
{
  tesselate( vs );
  return tesselatedMesh()->glAPI()->glNumPolygon( vs );
}


const GLuint* TesselatedMesh::glPolygonArray( const ViewState & vs ) const
{
  tesselate( vs );
  return tesselatedMesh()->glAPI()->glPolygonArray( vs );
}


const Material *TesselatedMesh::glMaterial() const
{
  return &material();
}


Material & TesselatedMesh::GetMaterial()
{
  return tesselatedMesh()->GetMaterial();
}


const Material & TesselatedMesh::material() const
{
  const GLComponent     *g = glGeometry();
  if( g && g != this )
    return *g->glMaterial();
  else
    return MObject::material();
}


bool TesselatedMesh::render( PrimList & prim, const ViewState & state )
{
  tesselate( state );

  bool ok = tesselatedMesh()->render( prim, state );
  return ok;
}


namespace
{

  void tessVertex( GLvoid* vertex, void* client )
  {
    TesselatedMesh::Private *d
      = reinterpret_cast<TesselatedMesh::Private *>( client );

    GLdouble *coords = (GLdouble *) vertex;
    Point3df pos = Point3df( (float) coords[0], (float) coords[1],
                             (float) coords[2] );
    d->vertices->push_back( pos );
    ++d->count;
    if( d->count == 3 )
      switch( d->polytype )
      {
      case GL_TRIANGLES:
        {
          d->count = 0;
          unsigned n = d->vertices->size();
          d->polygons->push_back( AimsVector<uint, 3>( n-3, n-2, n-1 ) );
        }
        break;
      case GL_TRIANGLE_FAN:
        {
          d->count = 2;
          unsigned n = d->vertices->size() - 1;
          d->polygons->push_back( AimsVector<uint, 3>( d->index0, d->index1,
                                                       n ) );
          d->index1 = n;
        }
        break;
      case GL_TRIANGLE_STRIP:
        {
          d->count = 2;
          unsigned n = d->vertices->size() - 1;
          d->polygons->push_back( AimsVector<uint, 3>( d->index0, d->index1,
                                                       n ) );
          if( d->odd )
          {
            d->index0 = n;
            d->index1 = n-1;
          }
          else
          {
            d->index0 = n-1;
            d->index1 = n;
          }
          d->odd = !d->odd;
        }
        break;
      default:
        cout << "unknown polygon code: " << d->polytype << endl;
      }
    else
    {
      if( d->count == 1 )
        d->index0 = d->vertices->size() - 1;
      else
        d->index1 = d->vertices->size() - 1;
    }

    // glVertex3dv( (GLdouble *) vertex );
  }


  void tessBegin( GLenum polytype, void* client )
  {
    TesselatedMesh::Private *d
      = reinterpret_cast<TesselatedMesh::Private *>( client );
    if( d->vertices->size() == 0 )
    {
      d->vertices->reserve( d->nvert );
      d->vertices->insert( d->vertices->end(), d->addedVertices.begin(),
                           d->addedVertices.end() );
      d->addedVertices.clear();
    }
    d->polytype = polytype;
    d->count = 0;
    d->odd = true;

    // glBegin( polytype );
  }


  void tessEnd( void* /*client*/ )
  {
    // glEnd();
  }


  void tessError( GLenum errcode, void* /*client*/ )
  {
    // TesselatedMesh* tmesh = reinterpret_cast<TesselatedMesh *>( client );
    cout << "Tesselation error: " << gluErrorString( errcode ) << endl;
  }


  void tessCombine( GLdouble coords[3], GLdouble* vdata[4], GLfloat weight[4],
    GLdouble **outdata, void* client )
  {
    TesselatedMesh::Private *d
      = reinterpret_cast<TesselatedMesh::Private *>( client );

    GLdouble *vertex = (GLdouble *) malloc( 6 * sizeof( GLdouble ) );
    vertex[0] = coords[0];
    vertex[1] = coords[1];
    vertex[2] = coords[2];
    for( unsigned i=3; i<6; ++i )
    {
      GLdouble & v = vertex[i];
      if( vdata[0] )
        v = weight[0] * vdata[0][i];
      else
        v = 0;
      if( vdata[1] )
        v += weight[1] * vdata[1][i];
      if( vdata[2] )
        v += weight[2] * vdata[2][i];
      if( vdata[3] )
        v += weight[3] * vdata[3][i];
    }
    *outdata = vertex;
    d->addedVertices.push_back( Point3df( (float) coords[0], (float) coords[1],
                                          (float) coords[2] ) );
  }


  void tessPolyVertex( GLvoid* vertex, void* client )
  {
    TesselatedMesh::Private *d
      = reinterpret_cast<TesselatedMesh::Private *>( client );
    GLdouble *v = (GLdouble *) vertex;
    gluTessVertex( d->tesselator, v, v );
    // glVertex3dv( (GLdouble *) vertex );
  }


  void tessPolyBegin( GLenum /*polytype*/, void* client )
  {
    TesselatedMesh::Private *d
      = reinterpret_cast<TesselatedMesh::Private *>( client );
    gluTessBeginContour( d->tesselator );
    // glBegin( polytype );
  }


  void tessPolyEnd( void* client )
  {
    TesselatedMesh::Private *d
      = reinterpret_cast<TesselatedMesh::Private *>( client );
    gluTessEndContour( d->tesselator );
    // glEnd();
  }


  void tessPolyError( GLenum errcode, void* /*client*/ )
  {
    cout << "Poly Tesselation error: " << gluErrorString( errcode ) << endl;
  }


  void tessPolyCombine( GLdouble coords[3], GLdouble* vdata[4],
                        GLfloat weight[4], GLdouble **outdata,
                        void* /*client*/ )
  {
    GLdouble *vertex = (GLdouble *) malloc( 6 * sizeof( GLdouble ) );
    vertex[0] = coords[0];
    vertex[1] = coords[1];
    vertex[2] = coords[2];
    for( unsigned i=3; i<6; ++i )
    {
      GLdouble & v = vertex[i];
      if( vdata[0] )
        v = weight[0] * vdata[0][i];
      else
        v = 0;
      if( vdata[1] )
        v += weight[1] * vdata[1][i];
      if( vdata[2] )
        v += weight[2] * vdata[2][i];
      if( vdata[3] )
        v += weight[3] * vdata[3][i];
    }
    *outdata = vertex;
  }


  bool tryConnectCurve( list<list<GLuint> > & orderedlist,
                        map<GLuint, AimsVector<GLuint,2> > neigh,
                        const GLfloat* vertices )
  {
    bool connected = false;
    const Point3df *pvert = reinterpret_cast< const Point3df *>( vertices );
    const float eps = 1e-6;

    list<GLuint> & ordered = orderedlist.back();
    if( ordered.empty() )
      return true;

    list<GLuint>::reverse_iterator ior = ordered.rbegin(), eor = ordered.rend();
    GLuint i = *ior;
    GLuint ni = (unsigned) -1;
    ++ior;
    if( ior != eor )
      ni = *ior; // next in curve
    AimsVector<GLuint,2> & nx = neigh[i];
    GLuint j = nx[0];
    bool startside = true; // next check the starting side

    if( j == (unsigned) -1 || j == ni )
    {
      j = nx[1];
      if( j == ni )
        j = (unsigned) -1;
    }
    list<list<GLuint> >::iterator il, el = orderedlist.end();
    --el; // stop before ordered, the last curve in list
    if( j != (unsigned) -1 )
      for( il=orderedlist.begin(); il!=el; ++il )
        if( !il->empty() )
        {
          GLuint b = il->back();
          if( b == j )
          {
            // insert at end of *il, in reverse order
            il->insert( il->end(), ordered.rbegin(), ordered.rend() );
            connected = true;
            startside = false; // next check the ending end of il
            break;
          }
          b = il->front();
          if( b == j )
          {
            // insert at beginning of *il
            il->insert( il->begin(), ordered.begin(), ordered.end() );
            connected = true;
            break;
          }
        }

    // try using 3D positions
    if( !connected )
    {
      Point3df pi = pvert[i];
      for( il=orderedlist.begin(); il!=el; ++il )
        if( !il->empty() )
        {
          GLuint b = il->back();
          if( ( pvert[b] - pi ).norm2() < eps )
          {
            // insert at end of *il, in reverse order, skip i
            list<GLuint>::reverse_iterator ii = ordered.rbegin();
            ++ii;
            il->insert( il->end(), ii, ordered.rend() );
            connected = true;
            startside = false; // next check the ending end of il
            break;
          }
          b = il->front();
          if( ( pvert[b] - pi ).norm2() < eps )
          {
            // insert at beginning of *il, skip i
            list<GLuint>::iterator ii = ordered.end();
            --ii;
            il->insert( il->begin(), ordered.begin(), ii );
            connected = true;
            break;
          }
        }
    }

    // try other end
    if( !connected )
      il = el; // point to ordered

    list<GLuint> & curve = *il;
    list<list<GLuint> >::iterator ilc = il;
    bool connected2 = false;

    list<GLuint>::iterator is = curve.begin();
    if( !startside )
    {
      is = curve.end();
      --is;
    }
    GLuint i2 = *is;
    AimsVector<GLuint,2> & nx0 = neigh[ i2 ];
    GLuint ni2 = (unsigned) -1;
    if( curve.size() >= 2 )
    {
      if( startside )
        ++is;
      else
        --is;
      ni2 = *is;
    }

    j = nx0[0];
    if( j == ni2 || j == (unsigned) -1 )
    {
      j = nx0[1];
      if( j == ni2 )
        j = (unsigned) -1;
    }

    if( j != (unsigned) -1 )
      for( il=orderedlist.begin(); il!=el; ++il )
      {
        if( il == ilc )
          continue;
        if( !il->empty() )
        {
          GLuint b = il->back();
          if( b == j )
          {
            // insert at end of *il
            if( startside )
              il->insert( il->end(), curve.begin(), curve.end() );
            else
              il->insert( il->end(), curve.rbegin(), curve.rend() );
            if( connected )
              orderedlist.erase( ilc );
            connected = true;
            connected2 = true;
            break;
          }
          b = il->front();
          if( b == j )
          {
            // insert at beginning of *il, in reverse order
            if( startside )
              il->insert( il->begin(), curve.rbegin(), curve.rend() );
            else
              il->insert( il->begin(), curve.begin(), curve.end() );
            if( connected )
              orderedlist.erase( ilc );
            connected = true;
            connected2 = true;
            break;
          }
        }
      }

    // try using positions
    if( !connected2 )
    {
      Point3df pi = pvert[i2];
      for( il=orderedlist.begin(); il!=el; ++il )
      {
        if( il == ilc )
          continue;
        if( !il->empty() )
        {
          GLuint b = il->back();
          if( ( pvert[b] - pi ).norm2() < eps )
          {
            // insert at end of *il, skip i2
            if( startside )
            {
              list<GLuint>::iterator ii = curve.begin();
              ++ii;
              il->insert( il->end(), ii, curve.end() );
            }
            else
            {
              list<GLuint>::reverse_iterator ii = curve.rbegin();
              ++ii;
              il->insert( il->end(), ii, curve.rend() );
            }
            if( connected )
              orderedlist.erase( ilc );
            connected = true;
            break;
          }
          b = il->front();
          if( ( pvert[b] - pi ).norm2() < eps )
          {
            // insert at beginning of *il, in reverse order, skip i2
            if( startside )
            {
              list<GLuint>::reverse_iterator ii = curve.rend();
              --ii;
              il->insert( il->begin(), curve.rbegin(), ii );
            }
            else
            {
              list<GLuint>::iterator ii = curve.end();
              --ii;
              il->insert( il->begin(), curve.begin(), ii );
            }
            if( connected )
              orderedlist.erase( ilc );
            connected = true;
            break;
          }
        }
      }
    }

    return connected;
  }


  unsigned reorderPolygon( unsigned npol, const GLuint* poly,
                           list<list<GLuint> > & orderedlist,
                           const GLfloat* vertices )
  {
    if( npol == 0 )
      return 0;

    map<GLuint, AimsVector<GLuint,2> > neigh;
    map<GLuint, AimsVector<GLuint,2> >::iterator in, jn, en = neigh.end();
    const GLuint *iv, *ev = poly + npol*2;
    GLuint i, j;

    for( iv=poly; iv!=ev; ++iv )
    {
      i = *iv;
      ++iv;
      j = *iv;

      in = neigh.find( i );
      if( in == en )
        in = neigh.insert( make_pair( i, AimsVector<GLuint, 2>( -1, -1 ) )
          ).first;
      AimsVector<GLuint,2> & v = in->second;
      if( v[0] == (unsigned) -1 )
        v[0] = j;
      else
        v[1] = j;
      in = neigh.find( j );
      if( in == en )
        in = neigh.insert( make_pair( j, AimsVector<GLuint, 2>( -1, -1 ) )
          ).first;
      AimsVector<GLuint,2> & w = in->second;
      if( w[0] == (unsigned) -1 )
        w[0] = i;
      else
        w[1] = i;
    }

    unsigned n = neigh.size(), ndone = 0, nv = 0;
    list<GLuint> todo;
    set<GLuint> done;
    set<GLuint>::iterator notdone = done.end();
    in = neigh.begin();
    bool newcurve;
    while( ndone != n )
    {
      for( ; done.find( in->first ) != notdone; ++in )
      {}
      todo.push_back( in->first );
      newcurve = true;
      while( !todo.empty() )
      {
        i = todo.front();
        todo.pop_front();
        done.insert( i );
        if( newcurve )
        {
          newcurve = false;
          orderedlist.push_back( list<GLuint>() );
        }
        list<GLuint> & ordered = orderedlist.back();
        ++ndone;
        ++nv;
        AimsVector<GLuint,2> & nx = neigh[i];
        ordered.push_back( i );
        j = nx[1];
        if( j != (unsigned) -1 )
        {
          if( done.find( j ) == notdone )
          {
            todo.push_back( j );
            continue;
          }
        }
        bool ngb = (j != (unsigned) -1);
        j = nx[0];
        if( j != (unsigned) -1 )
        {
          if( done.find( j ) == notdone )
          {
            todo.push_back( j );
            continue;
          }
        }
        else if( !ngb )
        {
          cout << "vertex " << i << " without neighbour\n";
          ordered.pop_back();
          --nv;
        }
      }
      // end of curve. Try connect it to another existing one
      if( tryConnectCurve( orderedlist, neigh, vertices ) )
      {
        list<list<GLuint> >::iterator last = orderedlist.end();
        --last;
        orderedlist.erase( last );
      }
    }
    /*
    cout << "curves: " << orderedlist.size() << "\nextremities:" << endl;
    list<list<GLuint> >::iterator ic, ec = orderedlist.end();
    const Point3df *pt = reinterpret_cast<const Point3df *>( vertices );
    for( ic=orderedlist.begin(); ic!=ec; ++ic )
      cout << pt[ic->front()] << "  / " << ic->front() << "  -  "
        <<  pt[ic->back()] << "  / " << ic->back() << "  : " << ic->size()
        << endl;
    */
    return nv;
  }

}


void TesselatedMesh::tesselate( const ViewState & vs ) const
{
  if( !GLComponent::glHasChanged( glGEOMETRY ) )
    return;

  if( !d->tesselator )
  {
    d->tesselator = gluNewTess();
    gluTessCallback( d->tesselator, GLU_TESS_VERTEX_DATA,
                     (void (*)()) tessVertex );
    gluTessCallback( d->tesselator, GLU_TESS_BEGIN_DATA,
                     (void (*)()) tessBegin );
    gluTessCallback( d->tesselator, GLU_TESS_END_DATA,
                     (void (*)()) tessEnd );
    gluTessCallback( d->tesselator, GLU_TESS_ERROR_DATA,
                     (void (*)()) tessError );
    gluTessCallback( d->tesselator, GLU_TESS_COMBINE_DATA,
                     (void (*)()) tessCombine );
    gluTessProperty( d->tesselator, GLU_TESS_WINDING_RULE,
                     GLU_TESS_WINDING_ODD );
    d->polytess = gluNewTess();
    gluTessCallback( d->polytess, GLU_TESS_VERTEX_DATA,
                     (void (*)()) tessPolyVertex );
    gluTessCallback( d->polytess, GLU_TESS_BEGIN_DATA,
                     (void (*)()) tessPolyBegin );
    gluTessCallback( d->polytess, GLU_TESS_END_DATA,
                     (void (*)()) tessPolyEnd );
    gluTessCallback( d->polytess, GLU_TESS_ERROR_DATA,
                     (void (*)()) tessPolyError );
    gluTessCallback( d->polytess, GLU_TESS_COMBINE_DATA,
                     (void (*)()) tessPolyCombine );
    gluTessProperty( d->polytess, GLU_TESS_BOUNDARY_ONLY, GL_TRUE );
    gluTessProperty( d->polytess, GLU_TESS_WINDING_RULE,
                     GLU_TESS_WINDING_ODD );
    const ASurface<2> *surf = dynamic_cast<const ASurface<2> *>( firstPolygon() );
    if( surf && surf->isPlanar() )
    {
      const GLComponent *gl = surf->glAPI();
      if( gl )
      {
        const GLfloat* narr = gl->glNormalArray( vs );
        if( narr )
        {
          gluTessNormal( d->tesselator, narr[0], narr[1], narr[2] );
          gluTessNormal( d->polytess, narr[0], narr[1], narr[2] );
        }
      }
    }
  }
  iterator io = begin(), eo = end();
  unsigned nvert = 0, v = 0;

  list<list<list<GLuint> > > ordered;
  // count / order vertices on all polygons
  for( ++io; io!=eo; ++io )
  {
    GLComponent *gl = (*io)->glAPI();
    if( gl )
    {
      ordered.push_back( list<list<GLuint> >() );
      nvert += reorderPolygon( gl->glNumPolygon( vs ),
                               gl->glPolygonArray( vs ), ordered.back(),
                               gl->glVertexArray( vs ) );
      // cout << "polygon: " << ordered.back().size() << " curves, " << nvert << " vertices" << endl;
    }
  }
  vector<GLdouble> vertices;
  vertices.reserve( nvert * 3 );
  list<list<list<GLuint> > >::iterator iov = ordered.begin();
  ATriangulated *asurf = const_cast<ATriangulated *>(
    static_cast<const ATriangulated *>( tesselatedMesh() ) );
  rc_ptr<AimsSurfaceTriangle> surf = asurf->surface();
  d->vertices = &surf->vertex();
  d->vertices->clear();
  d->polygons = &surf->polygon();
  d->polygons->clear();
  surf->normal().clear();
  d->nvert = nvert;
  d->polygons->reserve( (unsigned) ( 1.2 * nvert ) );

  gluTessBeginPolygon( d->polytess, d );
  gluTessBeginPolygon( d->tesselator, d );
  for( io=begin(), ++io; io!=eo; ++io )
  {
    GLComponent *gl = (*io)->glAPI();
    if( !gl )
      continue;
    list<list<GLuint> > & overt = *iov;
    list<list<GLuint> >::iterator iiov, eiov = overt.end();
    const GLfloat* vert = gl->glVertexArray( vs );

    list<GLuint>::iterator iord, eord;
    for( iiov=overt.begin(); iiov!=eiov; ++iiov )
    {
      gluTessBeginContour( d->polytess );
      // we have to convert coords to double and keep them all in memory
      for( iord=iiov->begin(), eord=iiov->end(); iord!=eord; ++iord )
      {
        unsigned ind = 3 * *iord;
        vertices.push_back( vert[ ind + 0 ] );
        vertices.push_back( vert[ ind + 1 ] );
        vertices.push_back( vert[ ind + 2 ] );
        gluTessVertex( d->polytess, &vertices[v], &vertices[v] );
        v += 3;
      }
      gluTessEndContour( d->polytess );
    }
    ++iov;
  }
  gluTessEndPolygon( d->polytess );
  gluTessEndPolygon( d->tesselator );
  // cout << "tesselate done\n";
  // recreate normals, fast
  if( d->polygons->size() != 0 )
  {
    unsigned ip, np = d->polygons->size();
    Point3df pvec;
    for( ip=0; ip!=np; ++ip )
    {
      AimsVector<uint, 3> & pol = (*d->polygons)[ip];
      Point3df v1 = (*d->vertices)[ pol[1] ] - (*d->vertices)[ pol[0] ];
      Point3df v2 = (*d->vertices)[ pol[2] ] - (*d->vertices)[ pol[1] ];
      pvec = vectProduct( v1, v2 );
      if( pvec.norm2() >= 1e-6 * v1.norm2() * v2.norm2() )
        break; // non-flat triangle
    }
    pvec.normalize();
    unsigned i, n=d->vertices->size();
    vector<Point3df> & norm = surf->normal();
    norm.reserve( n );
    for( i=0; i<n; ++i )
      norm.push_back( pvec );
  }
  asurf->UpdateMinAndMax();
  asurf->glSetChanged( glGEOMETRY );
  asurf->setChanged();
  asurf->notifyObservers( const_cast<TesselatedMesh *>( this ) );

  // cleanup private struct
  d->vertices = 0;
  d->polygons = 0;
  d->addedVertices.clear();
  d->count = 0;
  d->tesselator = 0;
  d->polytess = 0;
  d->nvert = 0;
  d->polytype = 0;
  d->index0 = 0;
  d->index1 = 0;

  glClearHasChangedFlags();
  clearHasChangedFlags();
}


void TesselatedMesh::update( const Observable* observable, void* arg )
{
  // cout << "TesselatedMesh::update " << this << " " << size() << endl;
  iterator io = begin(), eo = end();
  for( ++io; io!=eo; ++io )
    if( observable == *io )
    {
      GLComponent *gl = (*io)->glAPI();
      if( gl && gl->glHasChanged( glGEOMETRY ) )
      {
        // cout << "polygon changed\n";
        GLComponent::glSetChanged( glGEOMETRY );
      }
    }
  GLObjectVector::update( observable, arg );
}


Tree* TesselatedMesh::optionTree() const
{
  static Tree*  _optionTree = 0;

  if( !_optionTree )
    {
      //Tree    *t, *t2;
      _optionTree = new Tree( true, "option tree" );

      Tree      *t = new Tree( true, "Referential" );
      _optionTree->insert( t );
      Tree      *t2 = new Tree( true, "Load" );
      t2->setProperty( "callback", &ObjectActions::referentialLoad );
      t->insert( t2 );
    }
  return( _optionTree );
}


void TesselatedMesh::SetMaterial( const Material & mat )
{
  AObject::SetMaterial( mat );
  AObject       *cm = tesselatedMesh();
  if( cm )
    cm->SetMaterial( mat );
}


AObject* TesselatedMesh::fallbackReferentialInheritance() const
{
  return const_cast<AObject *>( firstPolygon() );
}


