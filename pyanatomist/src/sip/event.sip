
namespace anatomist
{

  class OutputEvent
  {
%TypeHeaderCode
#include <anatomist/processor/event.h>
%End

  public:
    OutputEvent( const std::string &, carto::Object, 
		 bool /*, 
		 const set_STRING & = set_STRING()*/ );
    virtual ~OutputEvent();
    virtual void send();
    std::string eventType() const;
    carto::Object contents() const;
  };


  class EventHandler
  {
%TypeHeaderCode
#include <pyanatomist/event.h>
typedef carto::rc_ptr<anatomist::EventHandler > rc_ptr_EventHandler;
// DEBUG
#include <iostream>
%End

  public:
    EventHandler();
    virtual ~EventHandler();
    virtual void doit( const anatomist::OutputEvent & ) = 0;

    static void registerHandler( const std::string &, rc_ptr_EventHandler );
    static void unregisterHandler( const std::string &, 
                                   rc_ptr_EventHandler );
    static void registerHandler( const std::string &,
                                 anatomist::EventHandler* /Transfer/ );
%MethodCode
    std::cout << "registerHandler: " << a1 << std::endl;
    rc_ptr_EventHandler	*rc = new rc_ptr_EventHandler( a1 );
    anatomist::EventHandler::registerHandler( *a0, *rc );
%End
    static void unregisterHandler( const std::string &,
                                   anatomist::EventHandler* /Transfer/ );
%MethodCode
    std::cout << "unregisterHandler: " << a1 << std::endl;
    rc_ptr_EventHandler	*rc = new rc_ptr_EventHandler( a1 );
    anatomist::EventHandler::unregisterHandler( *a0, *rc );
%End
};

  class OutputEventFilter
  {
%TypeHeaderCode
#include <anatomist/processor/event.h>
%End

    public:
    OutputEventFilter();
    virtual ~OutputEventFilter();

    void filter(const std::string &);
    void unfilter(const std::string &);
    void setDefaultIsFiltering(bool);
  };

};

