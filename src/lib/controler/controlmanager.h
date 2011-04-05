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


#ifndef CONTROL_MANAGER_H
#define CONTROL_MANAGER_H



#include <string>
#include <list>
#include <set>
#include <map>

namespace anatomist
{

  struct ControlMapEntry
  {
    ControlMapEntry(std::string v, std::string o) : view(v), object(o) {}
    std::string view ;
    std::string object ;
  };
  
  struct LessControlMap
  {
    bool operator()( ControlMapEntry entry1, ControlMapEntry entry2 ) const
    {
      if ( entry1.view == entry2.view )
	return entry1.object < entry2.object ;
      return entry1.view < entry2.view ;
    }
  } ;
  
  typedef std::map< std::string, std::set< std::string > > ViewControls ;
  typedef std::map< std::string, std::set< std::string > > ObjectControls ;
  
  class ControlManager
  {
  public:
    ~ControlManager();
    
    static ControlManager* instance() ;
    
    typedef std::map< ControlMapEntry, std::set<std::string>, 
		      LessControlMap >::iterator iterator ;
    typedef std::map< ControlMapEntry, std::set<std::string>, 
		      LessControlMap >::const_iterator const_iterator ;
    
    std::set< std::string > 
    availableControlList( const std::string& view, 
			  const std::list<std::string>& objects ) const ;
    std::set< std::string > 
    availableControlList( const std::string& view, 
			  const std::string& object ) const ;
    std::set< std::string > 
    activableControlList( const std::string& view, 
			  const std::list<std::string>& selectedObjects ) 
      const ;
        
    bool insertView( const std::string& view, 
		     const ViewControls& viewControls ) ;
    bool removeView( const std::string& view ) ;
    
    bool insertObject( const std::string& object, 
		       const ObjectControls& objectControls ) ;
    bool removeObject( const std::string& object ) ;
    
    void addControl( const std::string& view, const std::string& object, 
		     const std::set< std::string > & controls ) ;
    void addControl( const std::string& view, const std::string& object, 
		     const std::string& control ) ;
    
    bool removeControl( const std::string& view, const std::string& object, 
			const std::string& control ) ;
    
    bool removeControlList( const std::string& view, 
			    const std::string& object ) ;
    void print() const  ;
  private:
    static ControlManager* _instance ;
    std::map< ControlMapEntry, std::set<std::string>, 
	      LessControlMap > myControlTable ;
  };
}
  
#endif
