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

#include <anatomist/commands/cExternalReference.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/processor/Registry.h>
#include <anatomist/processor/unserializer.h>
#include <anatomist/window/Window.h>
#include <anatomist/object/Object.h>
/*
#include <anatomist/reference/Referential.h>
#include <anatomist/reference/Transformation.h>
#include <anatomist/reference/transfSet.h>
*/
#include <anatomist/processor/context.h>
#include <cartobase/object/syntax.h>
#include <graph/tree/tree.h>
#include <cartobase/smart/rcptrtrick.h>
#include <vector>

using namespace anatomist;
using namespace carto;
using namespace std;


ExternalReferenceCommand::ExternalReferenceCommand(
    const vector<int> & elements, ActionType actype,
    CommandContext* context )
  : RegularCommand(), SerializingCommand( context ), _elem( elements ),
    _actiontype( actype )
{
}


ExternalReferenceCommand::~ExternalReferenceCommand()
{
}


bool ExternalReferenceCommand::initSyntax()
{
  SyntaxSet	ss;
  Syntax	& s = ss[ "ExternalReference" ];

  s[ "elements" ] = Semantic( "int_vector", true );
  s[ "action_type" ] = Semantic( "string", true );
  Registry::instance()->add( "ExternalReference", &read, ss );
  return( true );
}


namespace
{

  template <typename T>
  void manageRef( T* obj, ExternalReferenceCommand::ActionType t )
  {
    using carto::shared_ptr;

    typename shared_ptr<T>::ReferenceType rt = shared_ptr<T>::Weak;
    shared_ptr<T> sp;
    switch( t )
    {
      case ExternalReferenceCommand::TakeStrongRef:
        rt = shared_ptr<T>::Strong;
        sp.reset( rt, obj );
        ++rc_ptr_trick::refCount( sp );
        break;
      case ExternalReferenceCommand::TakeWeakSharedRef:
        rt = shared_ptr<T>::WeakShared;
        sp.reset( rt, obj );
        ++rc_ptr_trick::refCount( sp );
        ++rc_ptr_trick::weakCount( sp );
        break;
      case ExternalReferenceCommand::ReleaseStrongRef:
        rt = shared_ptr<T>::Strong;
        sp.reset( rt, obj );
        --rc_ptr_trick::refCount( sp );
        break;
      case ExternalReferenceCommand::ReleaseWeakSharedRef:
        rt = shared_ptr<T>::WeakShared;
        sp.reset( rt, obj );
        --rc_ptr_trick::weakCount( sp );
        --rc_ptr_trick::refCount( sp );
        break;
      default:
        break;
    }
  }

}


void ExternalReferenceCommand::doit()
{
  vector<int>::iterator i;
  void			*ptr;
  string		type;

  for( i=_elem.begin(); i!=_elem.end(); ++i )
  {
    ptr = context()->unserial->pointer( *i );
    if( !ptr )
      cerr << "ExternalReferenceCommand : Element ID " << *i
            << " not registered\n";
    else
    {
      type = context()->unserial->type( ptr );
      if( type == "AObject" )
      {
        AObject	*o = (AObject *) ptr;
        if( theAnatomist->hasObject( o ) )
        {
          if( _actiontype == ReleaseApplication )
            theAnatomist->releaseObject( o );
          else if( _actiontype == TakeApplication )
            theAnatomist->takeObjectRef( o );
          else
            manageRef( o, _actiontype );
        }
      }
      else if( type == "AWindow" )
      {
        AWindow	*w = (AWindow *) ptr;
        if( theAnatomist->hasWindow( w ) )
        {
          if( _actiontype == ReleaseApplication )
            theAnatomist->releaseWindow( w );
          else if( _actiontype == TakeApplication )
            theAnatomist->takeWindowRef( w );
          else
            manageRef( w, _actiontype );
        }
      }
      /*
      else if( type == "Referential" )
      {
        Referential	*r = (Referential *) ptr;
        if( r != theAnatomist->centralReferential() )
          {
            set<Referential *> refs = theAnatomist->getReferentials();
            if( refs.find( r ) != refs.end() )
              if( _actiontype == ReleaseApplication )
                theAnatomist->releaseReferential( r );
              else if( _actiontype == TakeApplication )
                theAnatomist->takeReferentialRef( r );
              else
                manageRef( r, _actiontype );
          }
      }
      else if( type == "Transformation" )
      {
        Transformation	*t = (Transformation *) ptr;
        if( ATransformSet::instance()->hasTransformation( t ) )
        {
          if( _actiontype == ReleaseApplication )
            theAnatomist->releaseTransformation( t );
          else if( _actiontype == TakeApplication )
            theAnatomist->takeOTransformationRef( t );
          else
            manageRef( t, _actiontype );

          set<AWindow *>
            win = theAnatomist->getWindows();
          set<AWindow*>::iterator	iw, fw = win.end();

          for( iw=win.begin(); iw!=fw; ++iw )
            (*iw)->SetRefreshFlag();
        }
      }
      */
      else
        cerr << "ExternalReferenceCommand: Unrecognized element type " << type
             << endl;
    }
  }
}


Command * ExternalReferenceCommand::read( const Tree & com,
                                          CommandContext* context )
{
  vector<int>	elem;
  string sactype;

  if( !com.getProperty( "elements", elem )
       || !com.getProperty( "action_type", sactype ) )
    return( 0 );
  map<string, ActionType> actypes;
  actypes[ "TakeStrongRef" ] = TakeStrongRef;
  actypes[ "TakeWeakSharedRef" ] = TakeWeakSharedRef;
  actypes[ "TakeWeakRef" ] = TakeWeakRef;
  actypes[ "ReleaseStrongRef" ] = ReleaseStrongRef;
  actypes[ "ReleaseWeakSharedRef" ] = ReleaseWeakSharedRef;
  actypes[ "ReleaseWeakRef" ] = ReleaseWeakRef;
  actypes[ "ReleaseApplication" ] = ReleaseApplication;
  actypes[ "TakeApplication" ] = TakeApplication;
  map<string, ActionType>::const_iterator i = actypes.find( sactype );
  if( i == actypes.end() )
  {
    cerr << "unknown action type for ExternalReference command: " << sactype
         << endl;
    return 0;
  }

  return( new ExternalReferenceCommand( elem, i->second, context ) );
}


void ExternalReferenceCommand::write( Tree & com, Serializer* ) const
{
  Tree	*t = new Tree( true, name() );

  static const string actypes[] =
  {
    "TakeStrongRef",
    "TakeWeakSharedRef",
    "TakeWeakRef",
    "ReleaseStrongRef",
    "ReleaseWeakSharedRef",
    "ReleaseWeakRef",
    "ReleaseApplication",
    "TakeApplication",
  };
  t->setProperty( "elements", _elem );
  t->setProperty( "action_type", actypes[ _actiontype ] );
  com.insert( t );
}

