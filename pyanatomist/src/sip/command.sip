
%ModuleCode
#if SIP_VERSION >= 0x040700
#include <anatomist/commands/cCreateWindow.h>
#include <anatomist/commands/cAddObject.h>
#include <anatomist/commands/cRemoveObject.h>
#include <anatomist/commands/cFusionObjects.h>
#include <anatomist/commands/cSetObjectPalette.h>
#include <anatomist/commands/cAssignReferential.h>
#include <anatomist/commands/cLoadTransformation.h>
#include <anatomist/commands/cNewPalette.h>
#include <anatomist/commands/cGroupObjects.h>
#include <anatomist/commands/cObjectInfo.h>
#include <anatomist/commands/cLoadObject.h>
#include <anatomist/commands/cGenerateTexture.h>
#include <anatomist/commands/cExtractTexture.h>
#endif
%End


namespace anatomist
{

  class Command
  {
%ConvertToSubClassCode
  if( dynamic_cast<anatomist::RegularCommand *>( sipCpp ) )
    if( dynamic_cast<anatomist::CreateWindowCommand *>( sipCpp ) )
      sipClass = sipClass_anatomist_CreateWindowCommand;
    else if( dynamic_cast<anatomist::AddObjectCommand *>( sipCpp ) )
      sipClass = sipClass_anatomist_AddObjectCommand;
    else if( dynamic_cast<anatomist::RemoveObjectCommand *>( sipCpp ) )
      sipClass = sipClass_anatomist_RemoveObjectCommand;
    else if( dynamic_cast<anatomist::FusionObjectsCommand *>( sipCpp ) )
      sipClass = sipClass_anatomist_FusionObjectsCommand;
    else if( dynamic_cast<anatomist::SetObjectPaletteCommand *>( sipCpp ) )
      sipClass = sipClass_anatomist_SetObjectPaletteCommand;
    else if( dynamic_cast<anatomist::AssignReferentialCommand *>( sipCpp ) )
      sipClass = sipClass_anatomist_AssignReferentialCommand;
    else if( dynamic_cast<anatomist::LoadTransformationCommand *>( sipCpp ) )
      sipClass = sipClass_anatomist_LoadTransformationCommand;
    else if( dynamic_cast<anatomist::NewPaletteCommand *>( sipCpp ) )
      sipClass = sipClass_anatomist_NewPaletteCommand;
    else if( dynamic_cast<anatomist::GroupObjectsCommand *>( sipCpp ) )
      sipClass = sipClass_anatomist_GroupObjectsCommand;
    else if( dynamic_cast<anatomist::ObjectInfoCommand *>( sipCpp ) )
      sipClass = sipClass_anatomist_ObjectInfoCommand;
    else
      sipClass = sipClass_anatomist_RegularCommand;
  else if( dynamic_cast<anatomist::WaitCommand *>( sipCpp ) )
    if( dynamic_cast<anatomist::LoadObjectCommand *>( sipCpp ) )
      sipClass = sipClass_anatomist_LoadObjectCommand;
    else
      sipClass = sipClass_anatomist_WaitCommand;
  else if( dynamic_cast<anatomist::GenerateTextureCommand *>( sipCpp ) )
    sipClass = sipClass_anatomist_GenerateTextureCommand;
  else if( dynamic_cast<anatomist::ExtractTextureCommand *>( sipCpp ) )
    sipClass = sipClass_anatomist_ExtractTextureCommand;
  else
    sipClass = 0;
%End

%TypeHeaderCode
#include <anatomist/processor/Command.h>
%End

%TypeCode
#include <pyanatomist/serializingcontext.h>
#include <graph/tree/tree.h>
%End

  public:
    virtual void execute(); 
    virtual std::string name() const = 0;
    carto::Object write( anatomist::CommandContext * = 0 );
%MethodCode
  anatomist::CommandContext  *cc = a0;
  if( !cc )
    cc = &anatomist::CommandContext::defaultContext();
  anatomist::SerializingContext  ser( *cc );
  Tree  t( true, sipCpp->name() );
  sipCpp->write( t, &ser );
  if( t.childrenSize() == 1 )
  {
    Tree  *t2 = new Tree( true, sipCpp->name() );
    t2->copyProperties( carto::Object::reference(
      ( (Tree *) *t.begin() )
        ->value<carto::AttributedObject::ContentType>() ) );
    sipRes = new carto::Object( t2 );
  }
  ser.updateUnserializer();
%End

  protected:
    Command();
    virtual void doit() = 0;
    virtual ~Command();

  private:
    Command( const anatomist::Command & );
  };


  class Processor
  {
%TypeHeaderCode
#include <anatomist/processor/Processor.h>
#include <anatomist/processor/context.h>
#include <anatomist/processor/Registry.h>
#include <graph/tree/tree.h>
#include <cartobase/object/pythonreader.h>
#include <cartobase/stream/sstream.h>
%End

  public:
    Processor();
    void execute( anatomist::Command * /Transfer/ ) /PyName=_execute/;

    anatomist::Command* 
    execute( const std::string &, const std::string & ="", 
             anatomist::CommandContext* = 0 ) /PyName=_execute/;
    bool execWhileIdle() const;
    void allowExecWhileIdle( bool );

  private:
    Processor( const anatomist::Processor & );
  };


  class CommandContext : carto::MutexedObject
  {
%TypeHeaderCode
#include <anatomist/processor/context.h>
#include <anatomist/processor/unserializer.h>
%End

  public:
    CommandContext();

    static anatomist::CommandContext & defaultContext();
    anatomist::OutputEventFilter *evfilter;
    
    rc_ptr_Unserializer unserial;

    SIP_PYOBJECT object( int ) const;
%MethodCode
    if( !sipCpp->unserial.get() )
    {
      sipIsErr = 1;
      PyErr_SetString( PyExc_RuntimeError, "no ID map" );
    }
    else
    {
      void	*ptr = sipCpp->unserial->pointer( a0 );
      if( !ptr )
      {
        sipIsErr = 1;
        PyErr_SetString( PyExc_RuntimeError, "ID not found" );
      }
      else
      {
        std::string t = sipCpp->unserial->type( ptr );
        if( t == "AObject" )
        {
          sipRes =
            sipConvertFromInstance( ptr, sipClass_anatomist_AObject, 0 );
        }
        else if( t == "AWindow" )
        {
          sipRes =
            sipConvertFromInstance( ptr, sipClass_anatomist_AWindow, 0 );
        }
        else if( t == "Referential" )
        {
          sipRes =
            sipConvertFromInstance( ptr, sipClass_anatomist_Referential, 0 );
        }
        else if( t == "Transformation" )
        {
          sipRes =
            sipConvertFromInstance( ptr, sipClass_anatomist_Transformation,
                                    0 );
        }
        else
        {
          sipIsErr = 1;
          PyErr_SetString( PyExc_TypeError,
                            ( std::string( "type " ) + t
                              + " unknown or not handled" ).c_str() );
        }
      }
    }
%End

    int id( anatomist::AObject* ) const;
%MethodCode
    if( !sipCpp->unserial.get() )
      {
        sipIsErr = 1;
        PyErr_SetString( PyExc_RuntimeError, "no ID map" );
      }
    else if( sipCpp->unserial->type( a0 ) != "AObject" )
      {
        sipIsErr = 1;
        PyErr_SetString( PyExc_TypeError, 
                         ( std::string( "Pointer for AObject has been " 
                                        "registered as a different type: " )
                           + sipCpp->unserial->type( a0 ) ).c_str() );
      }
    else
      try
        {
          sipRes = sipCpp->unserial->id( a0, "AObject" );
        }
      catch( std::exception & e )
        {
          sipIsErr = 1;
          PyErr_SetString( PyExc_RuntimeError, e.what() );
        }
%End

    int id( anatomist::AWindow* ) const;
%MethodCode
    if( !sipCpp->unserial.get() )
      {
        sipIsErr = 1;
        PyErr_SetString( PyExc_RuntimeError, "no ID map" );
      }
    else if( sipCpp->unserial->type( a0 ) != "AWindow" )
      {
        sipIsErr = 1;
        PyErr_SetString( PyExc_TypeError, 
                         ( std::string( "Pointer for AWindow has been " 
                                        "registered as a different type: " )
                           + sipCpp->unserial->type( a0 ) ).c_str() );
      }
    else
      try
        {
          sipRes = sipCpp->unserial->id( a0, "AWindow" );
        }
      catch( std::exception & e )
        {
          sipIsErr = 1;
          PyErr_SetString( PyExc_RuntimeError, e.what() );
        }
%End

    int id( anatomist::Referential* ) const;
%MethodCode
    if( !sipCpp->unserial.get() )
    {
      sipIsErr = 1;
      PyErr_SetString( PyExc_RuntimeError, "no ID map" );
    }
    else if( sipCpp->unserial->type( a0 ) != "Referential" )
    {
      sipIsErr = 1;
      PyErr_SetString( PyExc_TypeError,
                        ( std::string( "Pointer for Referential has been "
                                      "registered as a different type: " )
                          + sipCpp->unserial->type( a0 ) ).c_str() );
    }
    else
      try
      {
        sipRes = sipCpp->unserial->id( a0, "Referential" );
      }
      catch( std::exception & e )
      {
        sipIsErr = 1;
        PyErr_SetString( PyExc_RuntimeError, e.what() );
      }
%End

    int id( anatomist::Transformation* ) const;
%MethodCode
    if( !sipCpp->unserial.get() )
    {
      sipIsErr = 1;
      PyErr_SetString( PyExc_RuntimeError, "no ID map" );
    }
    else if( sipCpp->unserial->type( a0 ) != "Transformation" )
    {
      sipIsErr = 1;
      PyErr_SetString( PyExc_TypeError,
                        ( std::string( "Pointer for Transformation has been "
                                      "registered as a different type: " )
                          + sipCpp->unserial->type( a0 ) ).c_str() );
    }
    else
      try
      {
        sipRes = sipCpp->unserial->id( a0, "Transformation" );
      }
      catch( std::exception & e )
      {
        sipIsErr = 1;
        PyErr_SetString( PyExc_RuntimeError, e.what() );
      }
%End


    int makeID( anatomist::AObject* );
%MethodCode
   if( !sipCpp->unserial.get() )
      {
        sipIsErr = 1;
        PyErr_SetString( PyExc_RuntimeError, "no ID map" );
      }
    else
      {
        sipRes = sipCpp->unserial->makeID( a0, "AObject" );
      }    
%End

    int makeID( anatomist::Referential* );
%MethodCode
  if( !sipCpp->unserial.get() )
  {
    sipIsErr = 1;
    PyErr_SetString( PyExc_RuntimeError, "no ID map" );
  }
  else
  {
    sipRes = sipCpp->unserial->makeID( a0, "Referential" );
  }
%End

    int makeID( anatomist::Transformation* );
%MethodCode
  if( !sipCpp->unserial.get() )
  {
    sipIsErr = 1;
    PyErr_SetString( PyExc_RuntimeError, "no ID map" );
  }
  else
  {
    sipRes = sipCpp->unserial->makeID( a0, "Transformation" );
  }
%End

    int makeID( anatomist::AWindow* );
%MethodCode
   if( !sipCpp->unserial.get() )
      {
        sipIsErr = 1;
        PyErr_SetString( PyExc_RuntimeError, "no ID map" );
      }
    else
      {
        sipRes = sipCpp->unserial->makeID( a0, "AWindow" );
      }    
%End

    int freeID() const;
%MethodCode
   if( !sipCpp->unserial.get() )
      {
        sipIsErr = 1;
        PyErr_SetString( PyExc_RuntimeError, "no ID map" );
      }
    else
      {
        sipRes = sipCpp->unserial->freeID();
      }
%End

     SIP_PYTUPLE ids() const;
%MethodCode
   if( !sipCpp->unserial.get() )
      {
        sipIsErr = 1;
        PyErr_SetString( PyExc_RuntimeError, "no ID map" );
      }
    else
      {
        const std::map<int, void *>	& ids = sipCpp->unserial->ids();
        std::map<int, void *>::const_iterator i, e = ids.end();
        PyObject	*tup = PyTuple_New( ids.size() );
        int		n = 0;
        for( i=ids.begin(); i!=e; ++i, ++n )
          PyTuple_SetItem( tup, n, PyInt_FromLong( i->first ) );
        sipRes = tup;
      }
%End
  };


  class RegularCommand : anatomist::Command
  {
%TypeHeaderCode
#include <anatomist/processor/Command.h>
%End

  public:
    virtual ~RegularCommand();

  protected:
    RegularCommand();

  private:
    RegularCommand( const anatomist::RegularCommand & );
  };


  class WaitCommand : anatomist::Command
  {
%TypeHeaderCode
#include <anatomist/processor/Command.h>
%End

  public: 
    virtual ~WaitCommand();

  protected:
    WaitCommand();

  private:
    WaitCommand( const anatomist::WaitCommand & );
  };


  class CreateWindowCommand : anatomist::RegularCommand
  {
%TypeHeaderCode
#include <anatomist/commands/cCreateWindow.h>
#if SIP_VERSION < 0x040700
#include "sipaimssipvector_S32.h"
#endif
%End

  public:
    CreateWindowCommand( const std::string &, int = -1, 
                         anatomist::CommandContext * = 0,
                         const vector_S32& = vector_S32(),
                         int = 0, QWidget * = 0, int = 2,
                         carto::Object = carto::none() );

    virtual ~CreateWindowCommand();
    virtual std::string name() const;
    anatomist::AWindow *createdWindow() const;
    QWidget *block() const;

  protected:
    virtual void doit();

  private:
    CreateWindowCommand( const anatomist::CreateWindowCommand & );
  };


  class LoadObjectCommand : anatomist::WaitCommand
  {
%TypeHeaderCode
#include <anatomist/commands/cLoadObject.h>
%End

  public:
    LoadObjectCommand( const std::string &, int = -1, 
                       const std::string & = std::string( "" ), bool = false,
                       carto::Object = carto::none(),
                       anatomist::CommandContext * = 0 );
    virtual ~LoadObjectCommand();
    virtual std::string name() const;
    anatomist::AObject *loadedObject();

  protected:
    virtual void doit();

  private:
    LoadObjectCommand( const anatomist::LoadObjectCommand & );
  };


  class AddObjectCommand : anatomist::RegularCommand
  {
%TypeHeaderCode
#include <anatomist/commands/cAddObject.h>
#if SIP_VERSION < 0x040700
#include "sipanatomistsipset_AObjectPtr.h"
#include "sipanatomistsipset_AWindowPtr.h"
#endif
%End

%TypeCode
#if SIP_VERSION >= 0x040700
#ifndef PYAIMSSIP_SET_AObjectPtr_DEFINED
#define PYAIMSSIP_SET_AObjectPtr_DEFINED
typedef std::set<anatomist::AObject * > set_AObjectPtr;
#endif
#ifndef PYAIMSSIP_SET_AWindowPtr_DEFINED
#define PYAIMSSIP_SET_AWindowPtr_DEFINED
typedef std::set<anatomist::AWindow * > set_AWindowPtr;
#endif
#endif
%End

  public:
 /*    AddObjectCommand( anatomist::AObject *, anatomist::AWindow * ) 
      [(const std::set<anatomist::AObject *> &, 
        const std::set<anatomist::AWindow *>)];
%MethodCode
    std::set<anatomist::AObject *>	so;
    std::set<anatomist::AWindow *>	sw;
    so.insert( a0 );
    sw.insert( a1 );
    Py_BEGIN_ALLOW_THREADS
    sipCpp = new sipanatomist_AddObjectCommand( so, sw );
    Py_END_ALLOW_THREADS
%End
 */

	AddObjectCommand( const set_AObjectPtr &,
        const set_AWindowPtr,
        bool addchildren = false, bool addgraphnodes = false,
        bool addgraphrels = false);
/*
    AddObjectCommand( SIP_PYLIST, SIP_PYLIST ) 
      [(const set_AObjectPtr &,
        const set_AWindowPtr)];
%MethodCode
    set_AObjectPtr	so;
    set_AWindowPtr	sw;
    if( !PyList_Check( a0 ) || !PyList_Check( a1 ) )
      sipIsErr = 1;
    else
      {
        int	i, no = PyList_GET_SIZE( a0 ), nw = PyList_GET_SIZE( a1 );
        PyObject		*item;
        anatomist::AObject	*ao;
        anatomist::AWindow	*aw;
        int			iserr = 0;
        for( i=0; i<no; ++i )
          {
            item = PyList_GetItem( a0, i );
            if( item )
              {
                ao = reinterpret_cast<anatomist::AObject *>
                  ( sipForceConvertTo_anatomist_AObject( item, &iserr ) );
                if( !ao || iserr )
                  {
                    sipIsErr = 1;
                    break;
                  }
                so.insert( ao );
              }
          }
        if( !sipIsErr )
          {
            for( i=0; i<nw; ++i )
              {
                item = PyList_GetItem( a1, i );
                if( item )
                  {
                    aw = reinterpret_cast<anatomist::AWindow *>
                      ( sipForceConvertTo_anatomist_AWindow( item, &iserr ) );
                    if( !aw || iserr )
                      {
                        sipIsErr = 1;
                        break;
                      }
                    sw.insert( aw );
                  }
              }
          }
        if( !sipIsErr )
          {
            Py_BEGIN_ALLOW_THREADS
              sipCpp = new sipanatomist_AddObjectCommand( so, sw );
            Py_END_ALLOW_THREADS
          }
      }
%End
*/
    virtual ~AddObjectCommand();
    virtual std::string name() const;

  protected:
    virtual void doit();

  private:
    AddObjectCommand( const anatomist::AddObjectCommand & );
  };


  class RemoveObjectCommand : anatomist::RegularCommand
  {
%TypeHeaderCode
#include <anatomist/commands/cRemoveObject.h>
#if SIP_VERSION < 0x040700
#include "sipanatomistsipset_AObjectPtr.h"
#include "sipanatomistsipset_AWindowPtr.h"
#endif
%End

  public:

    RemoveObjectCommand(const set_AObjectPtr &,
        const set_AWindowPtr);

    virtual ~RemoveObjectCommand();
    virtual std::string name() const;

  protected:
    virtual void doit();

  private:
    RemoveObjectCommand( const anatomist::RemoveObjectCommand & );

  };


  class FusionObjectsCommand : anatomist::RegularCommand
  {
%TypeHeaderCode
#include <anatomist/commands/cFusionObjects.h>
#if SIP_VERSION < 0x040700
#include "sipanatomistsipanatomistAObject.h"
#endif
%End

  public:
    FusionObjectsCommand( SIP_PYLIST, const std::string & ="", int = -1, 
        bool = false, anatomist::CommandContext* = 0 ) 
      [( const std::vector<anatomist::AObject *> &, const std::string &="", 
         int=-1, bool=false, anatomist::CommandContext*=0 )];
%MethodCode
    std::vector<anatomist::AObject *>	so;
    if( !PyList_Check( a0 ) )
      sipIsErr = 1;
    else
      {
        int	i, no = PyList_GET_SIZE( a0 );
        PyObject		*item;
        anatomist::AObject	*ao;
        int			iserr = 0;
        so.reserve( no );
        for( i=0; i<no; ++i )
          {
            item = PyList_GetItem( a0, i );
            if( item )
              {
                ao = reinterpret_cast<anatomist::AObject *>
                  ( sipForceConvertTo_anatomist_AObject( item, &iserr ) );
                if( !ao || iserr )
                  {
                    sipIsErr = 1;
                    break;
                  }
                so.push_back( ao );
              }
          }
        if( !sipIsErr )
          {
            Py_BEGIN_ALLOW_THREADS
              std::cout << "FusionObjects: " << so.size() << ", " << *a1 << std::endl;
              sipCpp = new sipanatomist_FusionObjectsCommand( so, *a1, a2, 
                                                              a3, a4 );
            Py_END_ALLOW_THREADS
          }
      }
%End

    virtual ~FusionObjectsCommand();

    virtual std::string name() const;
    anatomist::AObject* createdObject() const;

  protected:
    virtual void doit();
  };


  class SetObjectPaletteCommand : anatomist::RegularCommand
  {
%TypeHeaderCode
#include <anatomist/commands/cSetObjectPalette.h>
#if SIP_VERSION < 0x040700
#include "sipanatomistsipset_AObjectPtr.h"
#endif
%End

%TypeCode
#if SIP_VERSION >= 0x040700
#ifndef PYAIMSSIP_SET_AObjectPtr_DEFINED
#define PYAIMSSIP_SET_AObjectPtr_DEFINED
typedef std::set<anatomist::AObject * > set_AObjectPtr;
#endif
#endif
%End

  public:
    SetObjectPaletteCommand( SIP_PYLIST, const std::string &, 
                             bool = false, float = 0, bool = false, float = 1, 
                             const std::string & ="", bool = false, float = 0, 
                             bool = false, float = 1, const std::string & ="", 
                             bool = false, float = 0.5, 
                             const std::string & ="" )
      [ (const set_AObjectPtr &, const std::string &,
         bool=false, float=0, bool=false, float=1, const std::string &="", 
         bool=false, float=0, bool=false, float=1, const std::string &="", 
         bool=false, float=0.5, const std::string &="" ) ];
%MethodCode
    std::set<anatomist::AObject *>	so;
    if( !PyList_Check( a0 ) )
      sipIsErr = 1;
    else
      {
        int	i, no = PyList_GET_SIZE( a0 );
        PyObject		*item;
        anatomist::AObject	*ao;
        int			iserr = 0;
        for( i=0; i<no; ++i )
          {
            item = PyList_GetItem( a0, i );
            if( item )
              {
                ao = reinterpret_cast<anatomist::AObject *>
                  ( sipForceConvertTo_anatomist_AObject( item, &iserr ) );
                if( !ao || iserr )
                  {
                    sipIsErr = 1;
                    break;
                  }
                so.insert( ao );
              }
          }
        if( !sipIsErr )
          {
            Py_BEGIN_ALLOW_THREADS
              // work around SIP bug on default string args
              std::string aa6 = "", aa11 = "", aa14 = "";
              if( !a6 ) a6 = &aa6;
              if( !a11 ) a11 = &aa11;
              if( !a14 ) a14 = &aa14;
              sipCpp = new sipanatomist_SetObjectPaletteCommand
                ( so, *a1, a2, a3, a4, a5, *a6, a7, a8, a9, a10, *a11, a12,
                  a13, *a14 );
            Py_END_ALLOW_THREADS
          }
      }
%End

    virtual ~SetObjectPaletteCommand();

    virtual std::string name() const;

  protected:
    virtual void doit();
  };


  class AssignReferentialCommand : anatomist::RegularCommand
  {
%TypeHeaderCode
#include <anatomist/commands/cAssignReferential.h>
#if SIP_VERSION < 0x040700
#include "sipanatomistsipset_AObjectPtr.h"
#include "sipanatomistsipset_AWindowPtr.h"
#endif
%End

  public:
    AssignReferentialCommand( anatomist::Referential*,
                              const set_AObjectPtr &,
                              const set_AWindowPtr &,
                              int = -1, anatomist::CommandContext* = 0,
                              const std::string & = std::string( "" ) );
    virtual ~AssignReferentialCommand();

    virtual std::string name() const;
    anatomist::Referential *ref() const;

  protected:
    virtual void doit();
  };


  class LoadTransformationCommand : anatomist::RegularCommand
  {
%TypeHeaderCode
#include <anatomist/commands/cLoadTransformation.h>
#if SIP_VERSION < 0x040700
#include "sipaimssipvector_FLOAT.h"
#endif
%End

  public:
    LoadTransformationCommand( const std::string &, anatomist::Referential*,
                               anatomist::Referential*, int = -1,
                               anatomist::CommandContext* = 0 );
    LoadTransformationCommand( const vector_FLOAT,
                               anatomist::Referential*,
                               anatomist::Referential*, int = -1,
                               anatomist::CommandContext* = 0 );
    virtual ~LoadTransformationCommand();

    virtual std::string name() const;
    anatomist::Transformation *trans() const;

  protected:
    virtual void doit();
  };


  class NewPaletteCommand : anatomist::RegularCommand
  {
%TypeHeaderCode
#include <anatomist/commands/cNewPalette.h>
%End

  public:
    NewPaletteCommand( const std::string & );
    virtual ~NewPaletteCommand();

    virtual std::string name() const;

  protected:
    virtual void doit();
  };


  class GroupObjectsCommand : anatomist::RegularCommand
  {
%TypeHeaderCode
#include <anatomist/commands/cGroupObjects.h>
#if SIP_VERSION < 0x040700
#include "sipanatomistsipset_AObjectPtr.h"
#endif
%End

%TypeCode
#include <anatomist/mobject/ObjectList.h>
%End

  public:
    GroupObjectsCommand( const set_AObjectPtr &,
                         int = -1, anatomist::CommandContext* = 0 );
    virtual ~GroupObjectsCommand();

    virtual std::string name() const;
    anatomist::MObject* groupObject() const;

  protected:
    virtual void doit();
  };


  class GenerateTextureCommand : anatomist::Command
  {
%TypeHeaderCode
#include <anatomist/commands/cGenerateTexture.h>
%End

  public:
    GenerateTextureCommand( anatomist::AObject *, int = -1, int = -1,
                            anatomist::CommandContext* = 0 );
    virtual ~GenerateTextureCommand();

    virtual std::string name() const;
    anatomist::AObject* createdObject();

  protected:
    virtual void doit();
  };


  class ExtractTextureCommand : anatomist::Command
  {
%TypeHeaderCode
#include <anatomist/commands/cExtractTexture.h>
%End

  public:
    ExtractTextureCommand( anatomist::AObject *, int = -1, int = -1,
                           anatomist::CommandContext* = 0 );
    virtual ~ExtractTextureCommand();

    virtual std::string name() const;
    anatomist::AObject* createdObject();

  protected:
    virtual void doit();
  };


  class ObjectInfoCommand : anatomist::RegularCommand
  {
%TypeHeaderCode
#include <anatomist/commands/cObjectInfo.h>
#ifndef PYAIMSSIP_VECTOR_S32_DEFINED
#define PYAIMSSIP_VECTOR_S32_DEFINED
typedef std::vector<int> vector_S32;
#endif
#ifndef PYAIMSSIP_SET_STRING_DEFINED
#define PYAIMSSIP_SET_STRING_DEFINED
typedef std::set<std::string> set_STRING;
#endif
%End

  public:
    ObjectInfoCommand( const std::string & filename,
                       anatomist::CommandContext* context,
                       const vector_S32 & objectIDs, bool nameChildren,
                       bool nameref = false, 
                       const std::string & requestid = "",
                       const set_STRING & objfilenames
                           = set_STRING() );
    virtual ~ObjectInfoCommand();

    virtual std::string name() const;
    carto::Object result();
  };

};

