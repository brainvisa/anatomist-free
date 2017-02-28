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


// must be included first (interaction pb between Xt & Qt ?)
#include <anatomist/control/qObjTree.h>

#include <anatomist/interpoler/interpoler.h>
#include <anatomist/interpoler/interpolMethod.h>
#include <anatomist/landmark/landmPicker.h>
#include <anatomist/surface/triangulated.h>
#include <anatomist/surface/texsurface.h>
#include <anatomist/surface/texture.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/application/settings.h>
#include <anatomist/object/actions.h>
#include <anatomist/misc/error.h>
#include <anatomist/primitive/primitive.h>
#include <anatomist/window/viewstate.h>
#include <anatomist/reference/transformobserver.h>
#include <aims/mesh/meshinterpoler.h>
#include <graph/tree/tree.h>
#include <qpixmap.h>
#include <stdio.h>


using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;


int	AInterpoler::_classType = AInterpoler::registerClass();
Tree	*AInterpoler::_optionTree = 0;


struct AInterpoler::Private
{
  typedef vector<GLfloat> CoordVec;
  Private();

  mutable MeshInterpoler *interpol;
  mutable map<unsigned, vector<CoordVec> >	texCoords;
  bool					neighboursHasChanged;
};


AInterpoler::Private::Private()
  : interpol( 0 ), neighboursHasChanged( true )
{
}


int AInterpoler::registerClass()
{
  int		type = registerObjectType( "INTERPOLER" );
  FusionMethod	*m = new AInterpolerMethod;

  if( !FusionFactory::registerMethod( m ) )
    delete m;

  return type;
}


//


AInterpoler::AInterpoler( AObject* o1, AObject* o2 )
  : GLObjectVector(), d( new Private )
{
  _type = _classType;

  GLComponent	*go2 = o2->glAPI();
  GLComponent	*go1 = o1->glAPI();
  ViewState	s( 0 );

  if( !go1 || !go2 )
    throw invalid_argument( "Interpoler object built on incorrect objects" );
  if( go2->glNumTextures() == 0 )
    {
      GLComponent	*c = go1;
      go1 = go2;
      go2 = c;
      AObject	*o = o1;
      o1 = o2;
      o2 = o;
    }
  if( go1->glNumVertex( s ) == 0 || go2->glNumVertex( s ) == 0 || 
      go2->glNumTextures() == 0 )
    cerr << "Warning: interpolation on incomplete or empty objects\n";

  insert( o1 );
  insert( o2 );

  if( QObjectTree::TypeNames.find( _type ) == QObjectTree::TypeNames.end() )
    {
      string str = Settings::findResourceFile( "icons/list_interpoler.xpm" );
      if( !QObjectTree::TypeIcons[ _type ].load( str.c_str() ) )
      {
        QObjectTree::TypeIcons.erase( _type );
        cerr << "Icon " << str.c_str() << " not found\n";
      }

      QObjectTree::TypeNames[ _type ] = "Interpoler";
    }

  glAddTextures( go2->glNumTextures() );
  //computeNeighbours( 0 );
}


AInterpoler::~AInterpoler()
{
  cleanup();
  delete d->interpol;
  delete d;
}


Tree* AInterpoler::optionTree() const
{
  if( !_optionTree )
    {
      Tree	*t, *t2;
      _optionTree = new Tree( true, "option tree" );
      t = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu", "File" ) );
      _optionTree->insert( t );
      t2 = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu", "Save" ) );
      t2->setProperty( "callback", &ObjectActions::saveStatic );
      t->insert( t2 );
      t2 = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu",
					      "Rename object" ) );
      t2->setProperty( "callback", &ObjectActions::renameObject );
      t->insert( t2 );
      t2 = new Tree( true, "Export texture" );
      t2->setProperty( "callback", &ObjectActions::saveTexture );
      t->insert( t2 );

      t = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu", "Landmarks" ) );
      _optionTree->insert( t );
      t2 = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu", 
					      "Pick landmarks" ) );
      t2->setProperty( "callback", &ALandmarkPicker::startInterface );
      t->insert( t2 );
    }
  return( _optionTree );
}


GLComponent* AInterpoler::glGeometry()
{
  iterator	i = begin();
  return (*i)->glAPI();
}


const GLComponent* AInterpoler::glGeometry() const
{
  iterator	i = begin();
  return (*i)->glAPI();
}


GLComponent* AInterpoler::texSurf()
{
  iterator	i = begin();
  ++i;
  return (*i)->glAPI();
}


const GLComponent* AInterpoler::texSurf() const
{
  iterator	i = begin();
  ++i;
  return (*i)->glAPI();
}


GLComponent* AInterpoler::orgGeom()
{
  return texSurf();
}


const GLComponent* AInterpoler::orgGeom() const
{
  return texSurf();
}


GLComponent* AInterpoler::glTexture()
{
  return glAPI();
}


const GLComponent* AInterpoler::glTexture() const
{
  return glAPI();
}


unsigned AInterpoler::glTexCoordSize( const ViewState & s, unsigned ) const
{
  return glGeometry()->glNumVertex( s );
}


const GLfloat* AInterpoler::glTexCoordArray( const ViewState & s, 
                                             unsigned tex ) const
{
  const_cast<AInterpoler *>( this )->refreshTexCoordArray( s );
  unsigned			step = (unsigned) ( s.time / TimeStep() );
  return &d->texCoords[ step ][ tex ][0];
}


void AInterpoler::refreshTexCoordArray( const ViewState & state )
{
  //cout << "AInterpoler::refreshTexCoordArray 1.\n";

  if( hasNeighboursChanged() )
    {
      delete d->interpol;
      d->interpol = 0;
      d->texCoords.clear();
      computeNeighbours( state.time );
      clearNeighboursChanged();
      glSetChanged( glBODY );
    }

  if( glHasChanged( glBODY ) )
    { cout << "clearing tex coords\n";
    d->texCoords.clear();
    }

  unsigned			step = (unsigned) ( state.time / TimeStep() );
  unsigned			tex, ntex = texSurf()->glNumTextures( state );
  vector<Private::CoordVec>	& cvec = d->texCoords[ step ];

  if( !cvec.empty() )
    return;	// cache OK

  // cout << "AInterpoler::refreshTexCoordArray 2.\n";

  vector<unsigned>		dim( ntex );
  vector<const GLfloat *>	orgTex( ntex );
  const GLComponent		*so = orgGeom();
  const GLComponent		*sd = glGeometry();
  const Point3df		*po 
    = (const Point3df *) so->glVertexArray( state );
  unsigned			n = sd->glNumVertex( state );
  const Point3df		*pd 
    = (const Point3df *) sd->glVertexArray( state );
  const AimsVector<uint,3>	*polyo 
    = (const AimsVector<uint,3> *) so->glPolygonArray( state );
  unsigned			i, t;
  vector<GLfloat *>		ptr( ntex );
  const GLfloat			*ptro1, *ptro2, *ptro3;
  unsigned			tri, j1, j2, j3;
  float				d1, d2, d3, x1, x2, a, b, l1, l2;
  Point3df			vec1, vec2, vec3, vec4;
  bool				xxfar;
  vector<unsigned>		texsize( ntex );

  if( !hasChanged() && cvec.size() == ntex )
    return;

  if( cvec.size() != ntex )
    cvec = vector<Private::CoordVec>( ntex );

  for( tex=0; tex<ntex; ++tex )
    {
      orgTex[ tex ] = texSurf()->glTexCoordArray( state, tex );
      dim[ tex ] = glDimTex( state, tex );
      texsize[ tex ] = n * dim[ tex ];
      if( cvec[ tex ].size() != texsize[ tex ] )
        cvec[ tex ] = Private::CoordVec( texsize[ tex ] );
      ptr[ tex ] = &cvec[tex][0];
      if( dim[tex] == 2 )
        d->interpol->resampleTexture(
          reinterpret_cast<const Point2df *>( orgTex[ tex ] ),
          reinterpret_cast<Point2df *>( ptr[ tex ] ), 0 );
      else
        d->interpol->resampleTexture( orgTex[ tex ], ptr[ tex ] );
    }

  glSetChanged( glBODY, false );
}


void AInterpoler::computeNeighbours( const ViewState & state )
{
  //cout << "AInterpoler::computeNeighbours\n";
  theAnatomist->setCursor( Anatomist::Working );

  const GLComponent		*s1 = orgGeom();
  const GLComponent		*s2 = glGeometry();
  const Point3df		*vert1 
    = (const Point3df *) s1->glVertexArray( state );
  const Point3df		*vert2 
    = (const Point3df *) s2->glVertexArray( state );
  const AimsVector<uint,3>	*tri1 
    = (const AimsVector<uint,3> *) s1->glPolygonArray( state );
  unsigned			i, t, n2 = s2->glNumVertex( state );
  unsigned			nt1 = s1->glNumPolygon( state );

  delete d->interpol;
  d->interpol = new MeshInterpoler( vert1, tri1, nt1, vert2,
                                    (const AimsVector<uint,3> *)
                                      s2->glPolygonArray( state ), n2,
                                    s2->glNumPolygon( state ) );
  d->interpol->project();

  theAnatomist->setCursor( Anatomist::Normal );
}


void AInterpoler::update( const Observable* observable, void* arg )
{
  //cout << "AInterpoler::update: " << this << ", obs: " << observable << endl;

  AObject::update( observable, arg );

  const AObject	*ao = dynamic_cast<const AObject *>( observable );
  if( ao )
    {
      const GLComponent	*c = ao->glAPI();
      const GLComponent	*surf = glGeometry();
      if( c == surf )
        {
          // cout << "Interpoler geometry modified\n";
          if( ao->obsHasChanged( glGEOMETRY ) )
            {
              glSetChanged( glGEOMETRY );
              setNeighboursChanged();
            }
          if( ao->obsHasChanged( glMATERIAL ) )
            glSetChanged( glMATERIAL );
        }
      else if( c == texSurf() )
        {
          // cout << "Interpoler texSurf modified\n";
          if( ao->obsHasChanged( glGEOMETRY ) )
            {
              glSetChanged( glGEOMETRY );
              setNeighboursChanged();
            }
          if( ao->obsHasChanged( glBODY ) )
            glSetChanged( glBODY );
          if( ao->obsHasChanged( glTEXENV ) )
            glSetChanged( glTEXENV );
          if( ao->obsHasChanged( glTEXIMAGE ) )
            glSetChanged( glTEXIMAGE );
        }
    }
  else
    {
      const TransformationObserver 
        *to = dynamic_cast<const TransformationObserver *>( observable );
      if( !to )
        return;

      // cout << "Transformation changed in Interpoler\n";
      setNeighboursChanged();
      glSetChanged( glGEOMETRY );
      glSetTexImageChanged( true, 0 );
    }
  notifyObservers( (void*) this );
}


void AInterpoler::setNeighboursChanged()
{
  d->neighboursHasChanged = true;
  setChanged();
}


void AInterpoler::clearNeighboursChanged()
{
  d->neighboursHasChanged = false;
}


bool AInterpoler::hasNeighboursChanged() const
{
  return d->neighboursHasChanged;
}
