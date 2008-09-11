
namespace anatomist
{

  class Observable : carto::SharedObject
  {
%TypeHeaderCode
#include <anatomist/observer/Observer.h>
#include <anatomist/observer/Observable.h>
%End

%TypeCode
#include <anatomist/graph/Graph.h>
#include <anatomist/graph/GraphObject.h>
#include <anatomist/window3D/window3D.h>
%End

%ConvertToSubClassCode
  sipClass = 0;
  if( dynamic_cast<anatomist::AObject *>( sipCpp ) )
  {
    if (dynamic_cast<anatomist::ASurface_3 *>(sipCpp))
      sipClass = sipClass_anatomist_ASurface_3;
    else if (dynamic_cast<anatomist::MObject *>(sipCpp))
    {
      if (dynamic_cast<anatomist::AGraph *>(sipCpp))
        sipClass = sipClass_anatomist_AGraph;
      else if (dynamic_cast<anatomist::AGraphObject *>(sipCpp))
        sipClass = sipClass_anatomist_AGraphObject;
      else if (dynamic_cast<anatomist::ObjectList *>(sipCpp))
        sipClass = sipClass_anatomist_ObjectList;
      else
        sipClass = sipClass_anatomist_MObject;
    }
    else if (dynamic_cast<anatomist::SliceableObject *>(sipCpp))
      sipClass = sipClass_anatomist_SliceableObject;
  }
  else if( dynamic_cast<AWindow3D *>( sipCpp ) )
  {
    *sipCppRet = static_cast<AWindow3D *>( sipCpp );
    sipClass = sipClass_AWindow3D;
  }
%End

  public:
    Observable();
    virtual ~Observable();
    void addObserver(anatomist::Observer*);
    void deleteObserver(anatomist::Observer*);
    void deleteObservers();
    int countObservers() const;
    virtual void notifyObservers(void* = 0);
    virtual void notifyUnregisterObservers();
    bool hasChanged() const;
    bool obsHasChanged( int ) const;
    bool obsHasChanged( const std::string & ) const;
    void setChanged() const;

  protected:
    void clearChanged() const;
    void obsSetChanged( int, bool = true ) const;
    void obsSetChanged( const std::string &, bool = true ) const;
  };

  class Observer
  {
%TypeHeaderCode
#include <anatomist/observer/Observer.h>
#include <anatomist/observer/Observable.h>
%End

%TypeCode
#include <anatomist/graph/Graph.h>
#include <anatomist/graph/GraphObject.h>
#include <anatomist/window/Window.h>
%End

%ConvertToSubClassCode
  sipClass = 0;
  if( dynamic_cast<anatomist::AObject *>( sipCpp ) )
  {
    *sipCppRet = static_cast<anatomist::AObject *>( sipCpp );
    if (dynamic_cast<anatomist::ASurface_3 *>(sipCpp))
      sipClass = sipClass_anatomist_ASurface_3;
    else if (dynamic_cast<anatomist::MObject *>(sipCpp))
    {
      if (dynamic_cast<anatomist::AGraph *>(sipCpp))
        sipClass = sipClass_anatomist_AGraph;
      else if (dynamic_cast<anatomist::AGraphObject *>(sipCpp))
        sipClass = sipClass_anatomist_AGraphObject;
      else if (dynamic_cast<anatomist::ObjectList *>(sipCpp))
        sipClass = sipClass_anatomist_ObjectList;
      else
        sipClass = sipClass_anatomist_MObject;
    }
    else if (dynamic_cast<anatomist::SliceableObject *>(sipCpp))
      sipClass = sipClass_anatomist_SliceableObject;
  }
  else if( dynamic_cast<anatomist::AWindow *>( sipCpp ) )
  {
    *sipCppRet = static_cast<anatomist::AWindow *>( sipCpp );
    if( dynamic_cast<QAWindow *>(sipCpp) )
    {
      *sipCppRet = static_cast<QAWindow *>( sipCpp );
      if( dynamic_cast<ControlledWindow *>( sipCpp ) )
        if( dynamic_cast<AWindow3D *>( sipCpp ) )
          sipClass = sipClass_AWindow3D;
        else
          sipClass = sipClass_ControlledWindow;
      else
        sipClass = sipClass_QAWindow;
    }
  }
%End

  public:
    Observer();
    virtual ~Observer();
    virtual void update( const anatomist::Observable*, void* ) = 0;

  protected:
    virtual void cleanupObserver();
    virtual void registerObservable( anatomist::Observable* );
    virtual void unregisterObservable( anatomist::Observable* );
  };

};

