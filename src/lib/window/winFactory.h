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


#ifndef ANA_WINDOW_WINFACTORY
#define ANA_WINDOW_WINFACTORY


#include <cartobase/object/object.h>


namespace anatomist
{

  class AWindow;


  /**	Window creator: (string) type -> new window.
	All static...
  */
  class AWindowFactory
  {
  public:
    typedef AWindow* (*WinCreator)( void *dock, carto::Object );

    //AWindowFactory();
    //virtual ~AWindowFactory();

    static AWindow* createWindow( const std::string & type, void *dock = 0, 
                                  carto::Object params = carto::none() );
    static AWindow* createWindow( int type, void *dock = 0, 
                                  carto::Object params = carto::none() );
    static bool initTypes();
    static std::string typeString( int type, int subtype = 0 );
    static int typeID( const std::string & type );
    static bool exist( int type );
    static bool exist( const std::string & type );
    static int registerType( const std::string & type, WinCreator creator );
    static std::set<std::string> types();
    static const std::map<std::string, int> & typeID() { return( TypeID ); }
    static const std::map<int, std::string> & typeNames()
    { return( TypeNames ); }
    static const std::map<int, WinCreator> & creators() { return( Creators ); }

  protected:
    static AWindow* createAxial( void *, carto::Object );
    static AWindow* createSagittal( void *, carto::Object );
    static AWindow* createCoronal( void *, carto::Object );
    static AWindow* create3D( void *, carto::Object );

    static std::map<int, std::string>	TypeNames;
    static std::map<std::string, int>	TypeID;
    static std::map<int, WinCreator>	Creators;

  private:
  };

}


#endif
