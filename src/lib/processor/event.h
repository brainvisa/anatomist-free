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


#ifndef ANATOMIST_PROCESSOR_EVENT_H
#define ANATOMIST_PROCESSOR_EVENT_H

#include <cartobase/object/object.h>

namespace anatomist
{

  /** This class holds events to send to output streams so that external 
      programs are notified when something happens in Anatomist */
  class OutputEvent
  {
  public:
    /**	Builds an Event
	\param evname event type
	\param contents event attributes. Attributes beginning with a \c '_' 
	(underscore), of types  \c AObject*, \c AWindow*, \c Referential*, 
	\c Transformation *, or containers types built on these types 
	(\c list, \c set, \c vector ) are converted to IDs before the event is 
	sent to an output stream.
	\param allocIDs specifies if new IDs must be generated for 'unnamed' 
	objects or if the corresponding objects are ignored
	\param discrimintantAttribs is a list of attributes that are mandatory 
	for the event to be significant: if these attributes are all empty, 
	the event is just not sent to the corresponding output stream
    */
    OutputEvent( const std::string & evname, carto::Object contents, 
		 bool allocIDs = true, 
		 const std::set<std::string> & discrimintantAttribs 
		 = std::set<std::string>() );
    virtual ~OutputEvent();
    /// sends the event to all output streams
    virtual void send();
    std::string eventType() const { return _name; }
    carto::Object contents() const { return _contents; }

  protected:
    OutputEvent();

  private:
    std::string			_name;
    bool			_allocateIDs;
    carto::Object		_contents;
    std::set<std::string>	_discrim;
  };


  class OutputEventFilter
  {
  public:
    OutputEventFilter();
    ~OutputEventFilter();

    void filter( const std::string & evtype );
    void filter( const std::set<std::string> & evtype );
    void unfilter( const std::string & evtype );
    void unfilter( const std::set<std::string> & evtype );
    const std::set<std::string> & filters() const { return _filtered; }
    void setDefaultIsFiltering( bool );
    bool isDefaultFiltering() const { return _deffiltering; }
    void clear();

  private:
    bool			_deffiltering;
    std::set<std::string>	_filtered;
  };


  /** Event handlers are called internally when the default context outputs 
      an event */
  class EventHandler
  {
  public:
    EventHandler();
    virtual ~EventHandler();
    /** this method should be defined by derived classes to do something 
        when an event of the matching type occurs */
    virtual void doit( const OutputEvent & ) = 0;

    static void registerHandler( const std::string & evtype, 
                                 carto::rc_ptr<EventHandler> );
    static void unregisterHandler( const std::string & evtype, 
                                   carto::rc_ptr<EventHandler> );
    static const std::map<std::string, std::set<carto::rc_ptr<EventHandler> > >
    & handlers() { return _handlers(); }

  private:
    static std::map<std::string, std::set<carto::rc_ptr<EventHandler> > > 
    & _handlers();
  };

}


#endif

