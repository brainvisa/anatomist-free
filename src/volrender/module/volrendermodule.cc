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


#include <anatomist/module/volrendermodule.h>
#include <anatomist/module/volrenderfusionmethod.h>
#include <anatomist/object/volrender.h>
#include <anatomist/object/actions.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/volume/Volume.h>
#include <graph/tree/tree.h>
#include <qtranslator.h>
#include <iostream>

using namespace anatomist;
using namespace std;


static bool initVolRenderModule()
{
  VolRenderModule	*a = new VolRenderModule;
  a->init();
  return( true );
}

static bool garbage = initVolRenderModule();


VolRenderModule::VolRenderModule() : Module()
{
}


VolRenderModule::~VolRenderModule()
{
}


string VolRenderModule::name() const
{
  return( QT_TRANSLATE_NOOP( "ControlWindow", "VolRender" ) );
}


string VolRenderModule::description() const
{
  return( QT_TRANSLATE_NOOP( "ControlWindow", "Volume rendering package" ) );
}


void VolRenderModule::objectsDeclaration()
{
  FusionFactory::registerMethod( new VolRenderFusionMethod );
}


void VolRenderModule::objectPropertiesDeclaration()
{
  AVolume<short>	vol;
  Tree			*tr = vol.optionTree();

  Tree			*t2, *t3;

  t2 = new Tree( true, "Volume rendering" );
  tr->insert( t2 );

  t3 = new Tree( true, "Create associated volume rendering object" ) ;

  t3->setProperty( "callback", &VolRenderModule::createVolRender );

  t2->insert( t3 );
}


void VolRenderModule::createVolRender( const set<AObject *> & obj )
{
  set<AObject *>::const_iterator	io, eo = obj.end();
  VolRender				*vr;

  for( io=obj.begin(); io!=eo; ++io )
  {
    vr = new VolRender( *io );

    if( vr )
    {
      vr->setName( theAnatomist->makeObjectName( (*io)->name() + "_VR" ) );
      theAnatomist->registerObject( vr );
    }
  }
}
