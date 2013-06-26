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

#include <anatomist/commands/cDeleteElement.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/processor/Registry.h>
#include <anatomist/processor/unserializer.h>
#include <anatomist/window/Window.h>
#include <anatomist/reference/Referential.h>
#include <anatomist/reference/Transformation.h>
#include <anatomist/reference/transfSet.h>
#include <anatomist/processor/context.h>
#include <cartobase/object/syntax.h>
#include <graph/tree/tree.h>
#include <qwidget.h>
#include <qapplication.h>
#include <aims/qtcompat/qwidgetlist.h>
#include <vector>

using namespace anatomist;
using namespace carto;
using namespace std;

//-----------------------------------------------------------------------------

DeleteElementCommand::DeleteElementCommand( const vector<int> & elements, 
					    CommandContext* context ) 
  : RegularCommand(), SerializingCommand( context ), _elem( elements )
{
}


DeleteElementCommand::~DeleteElementCommand()
{
}


bool DeleteElementCommand::initSyntax()
{
  SyntaxSet	ss;
  Syntax	& s = ss[ "DeleteElement" ];

  s[ "elements" ].type = "int_vector";
  s[ "elements" ].needed = true;
  Registry::instance()->add( "DeleteElement", &read, ss );
  return( true );
}


void
DeleteElementCommand::doit()
{
  vector<int>::iterator i;
  void			*ptr;
  string		type;

  for( i=_elem.begin(); i!=_elem.end(); ++i )
  {
    ptr = context()->unserial->pointer( *i );
    if( !ptr )
      cerr << "DeleteElementCommand : Element ID " << *i 
            << " not registered\n";
    else
    {
      type = context()->unserial->type( ptr );
      if( type == "AObject" )
      {
        AObject	*o = (AObject *) ptr;
        if( theAnatomist->hasObject( o ) )
          theAnatomist->destroyObject( o );
      }
      else if( type == "AWindow" )
      {
        AWindow	*w = (AWindow *) ptr;
        if( theAnatomist->hasWindow( w ) )
          w->tryDelete();
      }
      else if( type == "Referential" )
      {
        Referential	*r = (Referential *) ptr;
        if( r != theAnatomist->centralReferential() )
        {
          set<Referential *> refs = theAnatomist->getReferentials();
          if( refs.find( r ) != refs.end() )
            delete r;
        }
      }
      else if( type == "Transformation" )
      {
        Transformation	*t = (Transformation *) ptr;
        if( ATransformSet::instance()->hasTransformation( t ) )
          {
            delete t;

            set<AWindow *> 
              win = theAnatomist->getWindows();
            set<AWindow*>::iterator	iw, fw = win.end();

            for( iw=win.begin(); iw!=fw; ++iw )
              (*iw)->SetRefreshFlag();
          }
      }
      else if( type == "Widget" )
      {
        QWidget		*w =  (QWidget *) ptr;
        QWidgetList	wl4 = qApp->topLevelWidgets();
        QWidget		*w2 = 0;
        int		i, n = wl4.size();
        for( i=0; i < n && (w2=wl4.at(i)) != w; ++i );
        if( w != w2 )
        {
          wl4 = qApp->allWidgets();
          for( i=0, n=wl4.size(); i < n && (w2=wl4.at(i)) != w; ++i );
        }
        if( w == w2 )
        {
          // Call the close method of the widget because it could be impossible to close it if there are still references on it
          bool closeOk=w->close();
          // if the close method deletes the widget, we mustn't call the destructor again
          bool destructiveClose = (w->windowFlags() & Qt::WA_DeleteOnClose);
          bool deleteOnClose = w->testAttribute( Qt::WA_DeleteOnClose);
          if ((closeOk) && (!destructiveClose) && (!deleteOnClose))
            delete w;
        }
      }
      else
        cerr << "DeleteElementCommand: Unrecognized element type " << type 
              << endl;
    }
  }

  theAnatomist->UpdateInterface();
  theAnatomist->Refresh();
}


Command * DeleteElementCommand::read( const Tree & com, 
				      CommandContext* context )
{
  vector<int>	elem;

  if( !com.getProperty( "elements", elem ) )
    return( 0 );

  return( new DeleteElementCommand( elem, context ) );
}


void DeleteElementCommand::write( Tree & com, Serializer* ) const
{
  Tree	*t = new Tree( true, name() );

  t->setProperty( "elements", _elem );
  com.insert( t );
}
