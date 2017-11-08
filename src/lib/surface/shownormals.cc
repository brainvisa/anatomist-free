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

#include <anatomist/surface/shownormals.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/surface/shownormalsgui.h>
#include <anatomist/control/qObjTree.h>
#include <anatomist/application/settings.h>
#include <anatomist/object/actions.h>
#include <algorithm>

using namespace anatomist;
using namespace carto;
using namespace std;


float ANormalsMesh::_default_length = 2.;

ANormalsMesh::ANormalsMesh( const vector<AObject *> & ameshes )
  : ObjectVector(), _length( _default_length ), _nmesh( 0 )
{
  _type = registerObjectType( "ANormalsMesh" );

  if( QObjectTree::TypeNames.find( _type ) == QObjectTree::TypeNames.end() )
  {
    string str = Settings::findResourceFile( "icons/list_normals.xpm" );
    if( !QObjectTree::TypeIcons[ _type ].load( str.c_str() ) )
    {
      QObjectTree::TypeIcons.erase( _type );
      cerr << "Icon " << str.c_str() << " not found\n";
    }

    QObjectTree::TypeNames[ _type ] = "Normals";
  }

  vector<AObject *>::const_iterator io, eo = ameshes.end();

  for( io=ameshes.begin(); io!=eo; ++io )
  {
    ASurface<3> * surf = dynamic_cast<ASurface<3> *>( *io );
    if( surf )
    {
      _ameshes.push_back( surf );
      insert( rc_ptr<AObject>( *io ) );
    }
  }
  ASurface<2> *normalmesh = new ASurface<2>;
  normalmesh->setSurface( new AimsTimeSurface<2, Void> );
  normalmesh->setName( theAnatomist->makeObjectName( "normals" ) );
  Material & mat = normalmesh->GetMaterial();
  mat.SetDiffuse( 0, 0, 0.5, 1. );

  _nmesh = normalmesh;
  insert( rc_ptr<AObject>( normalmesh ) );
  theAnatomist->registerObject( normalmesh, false );
  theAnatomist->releaseObject( normalmesh );

  rebuild();
}


ANormalsMesh::~ANormalsMesh()
{
}

void ANormalsMesh::rebuild()
{
//   cout << "rebuild\n";
  ASurface<2> *nmesh = normalMesh();
  vector<Point3df> &vert = nmesh->surface()->vertex();
  vector<AimsVector<uint, 2> > & poly = nmesh->surface()->polygon();

  vert.clear();
  poly.clear();

  if( _ameshes.empty() )
    return;

  vector<ASurface<3> *>::const_iterator io, eo = _ameshes.end();
  size_t np = 0;

  for( io=_ameshes.begin(); io!=eo; ++io )
  {
    np += (*io)->surface()->vertex().size();
  }

  vert.reserve( np * 2 );
  poly.reserve( np );

  vector<Point3df>::iterator ip, in;
  size_t i;
  AimsTimeSurface<3, Void> *mesh = 0;

  for( io=_ameshes.begin(), mesh=(*io)->surface().get(),
       ip=mesh->vertex().begin(), in=mesh->normal().begin(), i=0; i<np;
       ++ip, ++in, ++i )
  {
    if( ip == mesh->vertex().end() )
    {
      ++io;
      mesh = (*io)->surface().get();
      ip = mesh->vertex().begin();
      in = mesh->normal().begin();
    }
    vert.push_back( *ip );
    vert.push_back( *ip + *in * _length );
    poly.push_back( AimsVector<uint, 2>( i*2, i*2 + 1 ) );
  }

  if( !_ameshes.empty() )
  {
    setReferentialInheritance( *_ameshes.begin() );
    nmesh->setReferentialInheritance( *_ameshes.begin() );
  }

  nmesh->glSetChanged( GLComponent::glGEOMETRY );
}


void ANormalsMesh::setLength( float length )
{
  if( length != _length )
  {
    _length = length;
    normalMesh()->glSetChanged( GLComponent::glGEOMETRY );
    setChanged();
    rebuild(); // could be postponed
  }
}


bool ANormalsMesh::render( PrimList & plist, const ViewState & vs )
{
  vector<ASurface<3> *>::iterator io, eo = _ameshes.end();
  for( io=_ameshes.begin(); io!=eo; ++io )
    (*io)->render( plist, vs );
  normalMesh()->render( plist, vs );
  return true;
}


void ANormalsMesh::update( const Observable* observable, void* arg )
{
//   cout << "ANormalsMesh::update\n";
  const AObject *obj = dynamic_cast<const AObject *>( observable );
  if( obj )
  {
    if( std::find( _ameshes.begin(), _ameshes.end(), obj ) != _ameshes.end() )
    {
      rebuild();
      normalMesh()->notifyObservers( this );
    }
    else if( obj == normalMesh() )
    {
      setChanged();
      notifyObservers( this );
    }
  }
}


ObjectMenu* ANormalsMesh::optionMenu() const
{
  rc_ptr<ObjectMenu> om = getObjectMenu( objectFullTypeName() );
  if( !om )
  {
    om.reset( new ObjectMenu );
    vector<string>  vs;
    vs.reserve(1);
    vs.push_back( QT_TRANSLATE_NOOP( "QSelectMenu", "File" ) );
    om->insertItem( vs, QT_TRANSLATE_NOOP( "QSelectMenu", "Save" ),
                    &ObjectActions::saveStatic );
    om->insertItem( vs, QT_TRANSLATE_NOOP( "QSelectMenu",
                    "Rename object"),
                    &ObjectActions::renameObject );
    vs[0] = QT_TRANSLATE_NOOP( "QSelectMenu", "Referential" );
    om->insertItem( vs, QT_TRANSLATE_NOOP( "QSelectMenu", "Load" ),
                    &ObjectActions::referentialLoad );
    vs[0] = QT_TRANSLATE_NOOP( "QSelectMenu", "Fusion" );
    om->insertItem(vs, QT_TRANSLATE_NOOP( "QSelectMenu",
                                          "Edit normals properties"),
                   &editNormalsProperties );
    setObjectMenu( objectFullTypeName(), om );
  }
  return AObject::optionMenu();
}


Tree* ANormalsMesh::optionTree() const
{
  return AObject::optionTree();
}


void ANormalsMesh::editNormalsProperties( const set<AObject *> & objects )
{
  NormalsSettingsPanel *np = new NormalsSettingsPanel( objects );
  np->show();
}


// ---

NormalsFusionMethod::~NormalsFusionMethod()
{
}


int NormalsFusionMethod::canFusion( const std::set<AObject *> & objects )
{
  set<AObject *>::const_iterator io, eo = objects.end();
  for( io=objects.begin(); io!=eo; ++io )
    if( !dynamic_cast<ASurface<3> *>( *io ) )
      return 0;
  return 1;
}


AObject* NormalsFusionMethod::fusion( const std::vector<AObject *> & objects )
{
  ANormalsMesh *anorm = new ANormalsMesh( objects );
  return anorm;
}


std::string NormalsFusionMethod::generatedObjectType() const
{
  return "ANormalsMesh";
}


