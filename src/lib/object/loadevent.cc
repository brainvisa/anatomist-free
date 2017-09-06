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

#include <anatomist/object/loadevent.h>
#include <anatomist/application/Anatomist.h>
#include <qapplication.h>

using namespace anatomist;
using namespace carto;


QEvent::Type AObjectLoadEvent::eventType()
{
  static Type type = QEvent::None;
  if( type == QEvent::None )
    type = (QEvent::Type) registerEventType();
  return type;
}


AObjectLoadEvent::AObjectLoadEvent( AObject* newobject,
                                    const ObjectReader::PostRegisterList &
                                    subObjects,
                                    Object options, void* clientid,
                                    bool last
                                  )
  : QEvent( eventType() ), _object( newobject ), _subObjects( subObjects ),
    _options( options ), _clientid( clientid ), _last( last )
{
}


AObjectLoadEvent::~AObjectLoadEvent()
{
}


AObject* AObjectLoadEvent::newObject()
{
  return _object;
}


Object AObjectLoadEvent::loadOptions()
{
  return _options;
}


const ObjectReader::PostRegisterList & AObjectLoadEvent::subObjects()
{
  return _subObjects;
}


void* AObjectLoadEvent::clientid()
{
  return _clientid;
}


bool AObjectLoadEvent::isLast() const
{
  return _last;
}


ObjectReaderNotifier::ObjectReaderNotifier( QObject* parent )
  : QObject( parent )
{
}


ObjectReaderNotifier::~ObjectReaderNotifier()
{
}


ObjectReaderNotifier* ObjectReaderNotifier::notifier()
{
  static ObjectReaderNotifier *notifier = 0;
  if( !notifier )
    notifier = new ObjectReaderNotifier( qApp );
  return notifier;
}


bool ObjectReaderNotifier::event( QEvent *e )
{
  if( e->type() == AObjectLoadEvent::eventType() )
  {
    e->accept();
    AObjectLoadEvent *lev = static_cast<AObjectLoadEvent *>( e );
    AObject *object = lev->newObject();
    if( object )
    {
      bool  visible = true;
      Object options = lev->loadOptions();
      if( options )
      {
        try
        {
          Object x = options->getProperty( "hidden" );
          if( x && (bool) x->getScalar() )
            visible = false;
        }
        catch( ... )
        {
        }
      }
      theAnatomist->registerObject( object, visible );
      // register sub-objects also created (if any)
      ObjectReader::PostRegisterList::const_iterator ipr,
        epr = lev->subObjects().end();
      for( ipr=lev->subObjects().begin(); ipr!=epr; ++ipr )
        theAnatomist->registerObject( ipr->first, ipr->second );

      object->notifyObservers( (void *) this );
    }
    emit( objectLoaded( lev->newObject(), lev->subObjects(),
                        lev->clientid(), lev->isLast() ) );
    return true;
  }
  else
  {
    e->ignore();
    return false;
  }
}

