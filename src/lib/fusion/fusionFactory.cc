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


#include <anatomist/fusion/fusionFactory.h>
#include <anatomist/fusion/defFusionMethods.h>
#include <anatomist/surface/fusiontexsurf.h>
#include <anatomist/fusion/fusionChooser.h>
#include <anatomist/application/Anatomist.h>



using namespace anatomist;
using namespace std;


FusionFactory		*FusionFactory::_theFactory = 0;
set<FusionMethod *>	FusionFactory::_methods;

static FusionFactory	*_theFusionFactory = new FusionFactory;



FusionMethod::~FusionMethod()
{
}


bool FusionMethod::orderingMatters() const
{
  return true;
}


//

FusionFactory::FusionFactory()
{
  delete _theFactory;
  _theFactory = this;

  if( _methods.empty() )	// base methods not registered yet ?
    {
      registerMethod( new Fusion2dMethod );
      registerMethod( new Fusion3dMethod );
      registerMethod( new PlanarFusion3dMethod );
      registerMethod( new FusionTexSurfMethod );
      registerMethod( new FusionTextureMethod );
      registerMethod( new FusionMultiTextureMethod );
      registerMethod( new FusionCutMeshMethod );
      registerMethod( new FusionSliceMethod );
      registerMethod( new FusionRGBAVolumeMethod );
      registerMethod( new FusionClipMethod );
      registerMethod( new FusionTesselationMethod );
      registerMethod( new Fusion2DMeshMethod );
      registerMethod( new ConnectivityMatrixFusionMethod );
      registerMethod( new VectorFieldFusionMethod );
    }
}


FusionFactory::~FusionFactory()
{
  _theFactory = 0;

  set<FusionMethod *>::iterator	im, em = _methods.end();
  for( im=_methods.begin(); im!=em; ++im )
    delete *im;
}


bool FusionFactory::registerMethod( FusionMethod* method )
{
  set<FusionMethod *>::const_iterator	im, fm=_methods.end();
  string	id = method->ID();

  for( im=_methods.begin(); im!=fm; ++im )
    if( (*im)->ID() == id )
      return( false );	// already registered

  _methods.insert( method );
  return( true );
}


bool FusionFactory::canFusion( const set<AObject *> & objects )
{
  if( objects.size() == 0 )
    return false;

  set<FusionMethod *>::const_iterator	im, fm=_methods.end();

  for( im=_methods.begin(); im!=fm; ++im )
    if( (*im)->canFusion( objects ) != 0 )
      return( true );
  return( false );
}


FusionMethod* FusionFactory::chooseMethod( const set<AObject *> & objects )
{
  vector<AObject *>	obj;
  obj.reserve( objects.size() );
  set<AObject *>::const_iterator	io, eo = objects.end();
  for( io=objects.begin(); io!=eo; ++io )
    obj.push_back( *io );
  return chooseMethod( obj, true );
}


FusionMethod* FusionFactory::chooseMethod( vector<AObject *> & objects,
                                           bool ordering )
{
  if( objects.size() == 0 )
    return 0;

  multimap<int, FusionMethod *> sm;
  set<FusionMethod *>::const_iterator im, fm=_methods.end();
  set<AObject *>	objs;

  objs.insert( objects.begin(), objects.end() );
  for( im=_methods.begin(); im!=fm; ++im )
  {
    int p = (*im)->canFusion( objs );
    if( p != 0 )
    {
      sm.insert( make_pair( p, *im ) );
      ordering |= (*im)->orderingMatters();
    }
  }

  if( sm.empty() )
    return 0;	// not found

  if( sm.size() == 1 && ( !ordering || objects.size() == 1 ) )
    return sm.begin()->second;

  vector<AObject *>	*obj = 0;
  if( ordering )
  {
    if( objects.size() != 1 )
      obj = &objects;
    else
      ordering = false;
  }

  // several fusions avalable
  FusionChooser	fc( sm, theAnatomist->getQWidgetAncestor(), 0, true, Qt::Window, obj );
  if( !fc.exec() )
    return( 0 );	// cancelled

  if( ordering )
    // reorder objects
    objects = fc.objects();
  return fc.selectedMethod();
}


multimap<int, string> FusionFactory::allowedMethods( const set<AObject *>
  & objs ) const
{
  multimap<int, string> meths;
  set<FusionMethod *>::const_iterator	im, fm=_methods.end();

  for( im=_methods.begin(); im!=fm; ++im )
  {
    int p = (*im)->canFusion( objs );
    if( p != 0 )
      meths.insert( make_pair( p, (*im)->ID() ) );
  }

  return meths;
}


FusionMethod* FusionFactory::method( const string & name ) const
{
  set<FusionMethod *>::const_iterator	im, em = _methods.end();

  for( im=_methods.begin(); im!=em && (*im)->ID()!=name; ++im ) {}
  if( im != em )
    return( *im );
  else
    return( 0 );
}


set<string> FusionFactory::methods()
{
  set<string> meths;
  set<FusionMethod *>::const_iterator im, em = _methods.end();
  for( im=_methods.begin(); im!=em; ++im )
    meths.insert( (*im)->ID() );
  return meths;
}

