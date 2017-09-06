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


#ifndef ANA_OBJECT_OREADER_H
#define ANA_OBJECT_OREADER_H

#include <anatomist/object/Object.h>
#include <qobject.h>

class AimsFinder;


namespace anatomist
{

  class AObject;


  /**	Object reader.
	Replacement for static AObject::Load functions. Can be replaced by a 
	derived class to include support for new objects. 
	In addition, a registration procedure allows to add or replace 
	readers functions for a given filename extension.
  */
  class ObjectReader
  {
  public:
    typedef std::vector<std::pair<AObject*,bool> > PostRegisterList;
    typedef std::list<AObject *> (*LoadFunction)( const std::string & filename,
                                      PostRegisterList & subObjectsToRegister,
                                      carto::Object options );

    class LoadFunctionClass
    {
    public:
      virtual ~LoadFunctionClass();
      virtual std::list<AObject *> load( const std::string & filename,
                             PostRegisterList & subObjectsToRegister,
                             carto::Object options ) = 0;
    };

    ObjectReader();
    virtual ~ObjectReader();

    /// singleton access
    static ObjectReader* reader();

    /**	Register a new extension and a loader function
    */
    static void registerLoader( const std::string & extension,
                                LoadFunction newFunc );
    /** same as the above function but takes a class loader instead.
        \param newFunc will be owned by ObjectReader
    */
    static void registerLoader( const std::string & extension,
                                LoadFunctionClass *newFunc );

    virtual std::list<AObject *> load( const std::string & filename,
                           PostRegisterList & subObjectsToRegister,
                           bool notifyFail = true,
                           carto::Object options = carto::none(),
                           void* clientid=0 ) const;
    virtual bool reload( AObject* object, bool notifyFail = true, 
                         bool onlyoutdated = false ) const;
    virtual AObject* readAims( const std::string & filename,
                               PostRegisterList & subObjectsToRegister,
                               carto::Object options = carto::none() ) const;
    static std::string allSupportedFileExtensions();
    static std::string anatomistSupportedFileExtensions();
    static std::string supportedFileExtensions( const std::string & objtype );
    static std::string
      supportedFileExtensions( const std::set<std::string> & objtypes );
    static std::set<std::string>
      supportedFileExtensionsSet( const std::string & objtype );
    static std::set<std::string>
      anatomistSupportedFileExtensionsSet();

  protected:
    std::list<AObject *> load_internal( const std::string & filename,
                            PostRegisterList & subObjectsToRegister,
                            carto::Object options ) const;

  private:
    ///	extention -> reader function map
    typedef std::multimap<std::string, carto::rc_ptr<LoadFunctionClass> >
        _storagetype;
    static std::list<AObject*> readMObject(
        const std::string & filename,
        std::vector<std::pair<AObject *, bool> > &, carto::Object options );
    static _storagetype & _loaders();
  };

}


#endif
