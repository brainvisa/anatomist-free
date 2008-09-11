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

#include <iostream>
#include "pyanatomist.h"
#include <anatomist/processor/Processor.h>
#include <exception>
#include <anatomist/commands/cCreateWindow.h>
#include <anatomist/processor/Processor.h>
#include <anatomist/window/qwindow.h>
#include <anatomist/application/settings.h>
#include <anatomist/control/wControl.h>
#include <anatomist/volume/Volume.h>
#include <anatomist/bucket/Bucket.h>
#include <anatomist/graph/Graph.h>
#include <anatomist/object/objectConverter.h>
#include <anatomist/reference/transfSet.h>
#include <anatomist/surface/texture.h>
#include <aims/def/path.h>
#include <cartobase/smart/rcptrtrick.h>
#include <qapplication.h>

using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;

void unexpectedExceptionHandler()
{
  cerr << "PyAnatomist fatal error" << endl;
}


AnatomistSip::AnatomistSip( const vector<string> & argv )
{
  set_unexpected( unexpectedExceptionHandler );
  try
    {
      if( !anatomist::theProcessor )
        {
          // cerr << "PyAnatomist: new anatomist::Processor;" << endl;
          new anatomist::Processor;
        }
      if( !theAnatomist )
        {
          // cerr << "PyAnatomist: new anatomist::Anatomist" << endl;

          static vector<const char *>  falseArgv;
          static const char anatv0[] = "anatomist";

          if( falseArgv.empty() )
          {
            if( argv.empty() )
            {
              falseArgv.reserve( 2 );
              falseArgv.push_back( anatv0 );
            }
            else
              falseArgv.reserve( argv.size() + 1 );
            unsigned i, n = argv.size();
            for( i=0; i<n; ++i )
            { cout << "arg: " << argv[i] << endl;
              falseArgv.push_back( argv[i].c_str() );
            }
            falseArgv.push_back( 0 );
          }

          int	falseargc = falseArgv.size() - 1;
          if( !qApp )
          {
            // cerr << "build a QApplication" << endl;
            new QApplication( falseargc, (char **) &falseArgv[0] );
          }
          new anatomist::Anatomist( falseargc, &falseArgv[0],
                                    "PyAnatomist GUI" );
          // cerr << "PyAnatomist: _anatomist.initialize();" << endl;
          theAnatomist->initialize();
          // cerr << "PyAnatomist: new QSelectFactory" << endl;
          new QSelectFactory;
        }
      // cerr << "PyAnatomist: all done" << endl;
    }
  catch( exception &e )
    {
      cerr << "PyAnatomist error: " << e.what() << endl;
    }
  catch( ... )
    {
      cerr << "PyAnatomist unknown error" << endl;
    }
}


QWidget *AnatomistSip::createWindow( const QString &type, QWidget *parent )
{
  anatomist::CreateWindowCommand *c 
    = new anatomist::CreateWindowCommand( type.utf8().data() );

  anatomist::theProcessor->execute( c );
  QWidget	*w = dynamic_cast<QWidget *>( c->createdWindow() );
  if( w && parent )
    w->reparent( parent, 0, QPoint( 0, 0 ) );

  return w;
}


void AnatomistSip::releaseWindow( AWindow* win )
{
  theAnatomist->releaseWindow( win );
}


void AnatomistSip::releaseObject( AObject* obj )
{
  theAnatomist->releaseObject( obj );
}


QString AnatomistSip::anatomistSharedPath()
{
  return anatomist::Settings::globalPath().c_str();
}


QString AnatomistSip::anatomistHomePath()
{
  return anatomist::Settings::localPath().c_str();
}


ControlWindow* AnatomistSip::getControlWindow()
{
  return theAnatomist->getControlWindow();
}

std::set<anatomist::AWindow*> AnatomistSip::getWindowsInGroup( int group )
{
  return theAnatomist->getWindowsInGroup( group );
}

void AnatomistSip::setObjectName( anatomist::AObject* obj,
                                  const std::string & name )
{
  obj->setName( name );
}

anatomist::Processor* AnatomistSip::theProcessor()
{
  return anatomist::theProcessor;
}

std::set<anatomist::AObject *> AnatomistSip::getObjects () const
{
	return theAnatomist->getObjects();
}

std::set<anatomist::AWindow *> AnatomistSip::getWindows () const
{
	return theAnatomist->getWindows();
}


std::set<anatomist::Referential *> AnatomistSip::getReferentials() const
{
  return theAnatomist->getReferentials();
}


std::set<anatomist::Transformation *> AnatomistSip::getTransformations() const
{
  return ATransformSet::instance()->allTransformations();
}


anatomist::PaletteList & AnatomistSip::palettes()
{
  return theAnatomist->palettes();
}


Point3df AnatomistSip::lastPosition( const Referential *toref ) const
{
  return theAnatomist->lastPosition( toref );
}


int AnatomistSip::userLevel() const
{
  return theAnatomist->userLevel();
}


void AnatomistSip::setUserLevel( int x )
{
  theAnatomist->setUserLevel( x );
}

// ------------------------

AimsData_U8* AObjectConverter::aimsData_U8( anatomist::AObject* obj )
{
  return ObjectConverter<AimsData_U8>::ana2aims( obj );
}


AimsData_S16* AObjectConverter::aimsData_S16( anatomist::AObject* obj )
{
  return ObjectConverter<AimsData_S16>::ana2aims( obj );
}


AimsData_U16* AObjectConverter::aimsData_U16( anatomist::AObject* obj )
{
  return ObjectConverter<AimsData_U16>::ana2aims( obj );
}


AimsData_S32* AObjectConverter::aimsData_S32( anatomist::AObject* obj )
{
  return ObjectConverter<AimsData_S32>::ana2aims( obj );
}


AimsData_U32* AObjectConverter::aimsData_U32( anatomist::AObject* obj )
{
  return ObjectConverter<AimsData_U32>::ana2aims( obj );
}


AimsData_FLOAT* AObjectConverter::aimsData_FLOAT( anatomist::AObject* obj )
{
  return ObjectConverter<AimsData_FLOAT>::ana2aims( obj );
}


AimsData_DOUBLE* AObjectConverter::aimsData_DOUBLE( anatomist::AObject* obj )
{
  return ObjectConverter<AimsData_DOUBLE>::ana2aims( obj );
}


AimsData_RGB* AObjectConverter::aimsData_RGB( anatomist::AObject* obj )
{
  return ObjectConverter<AimsData_RGB>::ana2aims( obj );
}


AimsData_RGBA* AObjectConverter::aimsData_RGBA( anatomist::AObject* obj )
{
  return ObjectConverter<AimsData_RGBA>::ana2aims( obj );
}


AimsTimeSurface<2,Void>* 
AObjectConverter::aimsSurface2( anatomist::AObject* obj )
{
  return ObjectConverter<AimsTimeSurface<2,Void> >::ana2aims( obj );
}


AimsTimeSurface<3,Void>* 
AObjectConverter::aimsSurface3( anatomist::AObject* obj )
{
  return ObjectConverter<AimsTimeSurface<3,Void> >::ana2aims( obj );
}


AimsTimeSurface<4,Void>* 
AObjectConverter::aimsSurface4( anatomist::AObject* obj )
{
  return ObjectConverter<AimsTimeSurface<4,Void> >::ana2aims( obj );
}


BucketMap<Void> *
AObjectConverter::aimsBucketMap_VOID( anatomist::AObject* obj )
{
  return ObjectConverter<BucketMap<Void> >::ana2aims( obj );
}


TimeTexture<float> *
AObjectConverter::aimsTexture_FLOAT( anatomist::AObject * obj )
{
  return ObjectConverter<TimeTexture<float> >::ana2aims( obj );
}


TimeTexture<Point2df> *
AObjectConverter::aimsTexture_POINT2DF( anatomist::AObject * obj )
{
  return ObjectConverter<TimeTexture<Point2df> >::ana2aims( obj );
}


Graph * AObjectConverter::aimsGraph( anatomist::AObject * obj )
{
  return ObjectConverter<Graph>::ana2aims( obj );
}


AObject* AObjectConverter::anatomist( AimsData_U8* aims )
{
  AObject	*ao = new AVolume<uint8_t>( *aims );
  ao->setName( theAnatomist->makeObjectName( "Volume_U8" ) );
  theAnatomist->registerObject( ao );
  ao->SetExtrema();
  ao->adjustPalette();
  return ao;
}


AObject* AObjectConverter::anatomist( AimsData_S16* aims )
{
  AObject	*ao = new AVolume<int16_t>( *aims );
  ao->setName( theAnatomist->makeObjectName( "Volume_S16" ) );
  theAnatomist->registerObject( ao );
  ao->SetExtrema();
  ao->adjustPalette();
  return ao;
}


AObject* AObjectConverter::anatomist( AimsData_U16* aims )
{
  AObject	*ao = new AVolume<uint16_t>( *aims );
  ao->setName( theAnatomist->makeObjectName( "Volume_U16" ) );
  theAnatomist->registerObject( ao );
  ao->SetExtrema();
  ao->adjustPalette();
  return ao;
}


AObject* AObjectConverter::anatomist( AimsData_S32* aims )
{
  AObject	*ao = new AVolume<int32_t>( *aims );
  ao->setName( theAnatomist->makeObjectName( "Volume_S32" ) );
  theAnatomist->registerObject( ao );
  ao->SetExtrema();
  ao->adjustPalette();
  return ao;
}


AObject* AObjectConverter::anatomist( AimsData_U32* aims )
{
  AObject	*ao = new AVolume<uint32_t>( *aims );
  ao->setName( theAnatomist->makeObjectName( "Volume_U32" ) );
  theAnatomist->registerObject( ao );
  ao->SetExtrema();
  ao->adjustPalette();
  return ao;
}


AObject* AObjectConverter::anatomist( AimsData_FLOAT* aims )
{
  AObject	*ao = new AVolume<float>( *aims );
  ao->setName( theAnatomist->makeObjectName( "Volume_FLOAT" ) );
  theAnatomist->registerObject( ao );
  ao->SetExtrema();
  ao->adjustPalette();
  return ao;
}


AObject* AObjectConverter::anatomist( AimsData_DOUBLE* aims )
{
  AObject	*ao = new AVolume<double>( *aims );
  ao->setName( theAnatomist->makeObjectName( "Volume_DOUBLE" ) );
  theAnatomist->registerObject( ao );
  ao->SetExtrema();
  ao->adjustPalette();
  return ao;
}


AObject* AObjectConverter::anatomist( AimsData_RGB* aims )
{
  AObject	*ao = new AVolume<AimsRGB>( *aims );
  ao->setName( theAnatomist->makeObjectName( "Volume_RGB" ) );
  theAnatomist->registerObject( ao );
  ao->SetExtrema();
  ao->adjustPalette();
  return ao;
}


AObject* AObjectConverter::anatomist( AimsData_RGBA* aims )
{
  AObject	*ao = new AVolume<AimsRGBA>( *aims );
  ao->setName( theAnatomist->makeObjectName( "Volume_RGBA" ) );
  theAnatomist->registerObject( ao );
  ao->SetExtrema();
  ao->adjustPalette();
  return ao;
}


AObject* AObjectConverter::anatomist( AimsSurfaceTriangle* aims )
{
  ASurface<3>	*ao = new ASurface<3>;
  rc_ptr<AimsSurfaceTriangle>	ref( aims );
  // increment the counter so that Anatomist never destroys the surface
  // WARNING: memory leak here: the ref counter will never be deleted
  rc_ptr_trick::releaseOwnership( ref );
  ao->setSurface( ref );
  ao->setName( theAnatomist->makeObjectName( "Mesh" ) );
  theAnatomist->registerObject( ao );
  return ao;
}


AObject* AObjectConverter::anatomist( AimsTimeSurface<2, Void>* aims )
{
  ASurface<2>	*ao = new ASurface<2>;
  rc_ptr<AimsTimeSurface<2, Void> > 
    *ref = new rc_ptr<AimsTimeSurface<2, Void> >( aims );
  // WARNING memory leak: ref is never destroyed !
  ao->setSurface( *ref );
  ao->setName( theAnatomist->makeObjectName( "Segments" ) );
  theAnatomist->registerObject( ao );
  return ao;
}


AObject* AObjectConverter::anatomist( AimsTimeSurface<4, Void>* aims )
{
  ASurface<4>	*ao = new ASurface<4>;
  rc_ptr<AimsTimeSurface<4, Void> > 
    *ref = new rc_ptr<AimsTimeSurface<4, Void> >( aims );
  // WARNING memory leak: ref is never destroyed !
  ao->setSurface( *ref );
  ao->setName( theAnatomist->makeObjectName( "Mesh4" ) );
  theAnatomist->registerObject( ao );
  return ao;
}


AObject* AObjectConverter::anatomist( BucketMap<Void> *aims )
{
  Bucket *bck = new Bucket;
  bck->setBucket( *aims );
  AObject	*ao = bck;
  ao->setName( theAnatomist->makeObjectName( "Bucket" ) );
  theAnatomist->registerObject( ao );
  ao->SetExtrema();
  return ao;
}


AObject* AObjectConverter::anatomist( Graph *aims )
{
  AGraph	*ao = new AGraph( aims, "" );
  ao->setName( theAnatomist->makeObjectName( "Graph" ) );
  theAnatomist->registerObject( ao );
  ao->SetExtrema();
  return ao;
}


AObject* AObjectConverter::anatomist( TimeTexture<float> *aims )
{
  ATexture        *ao = new ATexture;
  ao->setName( theAnatomist->makeObjectName( "Texture1d" ) );
  ao->setTexture( rc_ptr<TimeTexture<float> >( aims ) );
  theAnatomist->registerObject( ao );
  ao->SetExtrema();
  return ao;
}


AObject* AObjectConverter::anatomist( TimeTexture<Point2df> *aims )
{
  ATexture        *ao = new ATexture;
  ao->setName( theAnatomist->makeObjectName( "Texture2d" ) );
  ao->setTexture( rc_ptr<TimeTexture<Point2df> >( aims ) );
  theAnatomist->registerObject( ao );
  ao->SetExtrema();
  return ao;
}


