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


#ifndef ANA_REFERENCE_REFERENTIAL_H
#define ANA_REFERENCE_REFERENTIAL_H


//--- header files ------------------------------------------------------------

#include <anatomist/object/Object.h>
#include <anatomist/window/Window.h>
#include <aims/data/pheader.h>
#include <cartobase/uuid/uuid.h>


//--- class declarations ------------------------------------------------------

namespace anatomist
{

  /**	Referential: marker for transformations, with an associated color
   */
  class Referential
  { 
  public:
    Referential();
    Referential( const std::string & filename );
    Referential( std::set<AObject*>& );
    Referential( std::set<AWindow*>& );
    Referential( const Referential& ref );
    virtual ~Referential();

    Referential & operator = ( const Referential & );
    
    /// objects which are currently in this referential
    std::set<AObject*> AnaObj( void ) const { return( _anaObj ); }
    /// windows which are currently in this referential
    std::set<AWindow*> AnaWin( void ) const { return( _anaWin ); }
    AimsRGB Color() const { return( _color ); }
    void setColor( const AimsRGB & col );
    /// Unique index, 0 is always the central referential
    int index() const { return( _indCol ); }
    /// Ajoute un objet dans la liste des objets.
    void AddObject(AObject *);
    /// Ajoute une fenetre dans la liste des fenetres.
    void AddWindow(AWindow *);
    /// Enleve un objet de la liste des objets.
    void RemoveObject(AObject *);
    /// Enleve une fenetre de la liste des fenetres.
    void RemoveWindow(AWindow *);
    bool destroying() const { return _destroying; }
    carto::UUID uuid() const;
    const aims::PythonHeader & header() const { return *_header; }
    aims::PythonHeader & header() { return *_header; }
    bool load( const std::string & filename );
    std::string filename() const;
    bool isDirect() const;
    /** a hidden referential does not appear in the GUI. This function is
    merely a shortcut to the header property "hidden". */
    bool hidden() const;

    static Referential* referentialOfUUID( const carto::UUID & uuid );
    static Referential* referentialOfUUID( const std::string & uuid );
    static Referential* acPcReferential();
    static Referential* mniTemplateReferential();
    static Referential* giftiTalairachReferential();
    /// tries to find a ref in the referentials property list
    static Referential* referentialOfNameOrUUID( const AObject* obj,
                                                 const std::string & refname );
    /// WARNING: a referential may not have a unique name.
    static Referential* referentialOfName( const std::string & refname );
    static void clearUnusedReferentials();
    /** merge 2 unconnected referential or connected by identity.
        r2 will be deleted, objects/windows in ref r2 will be moved to r1.
        If r2 is a non-destructible referential (AC/PC...) then r1 will be
        deleted.

        \return false if r1 and r2 are connected by a non-identity transform.
    */
    static bool mergeReferentials( Referential* r1, Referential *r2 );
    static void mergeIdenticalReferentials();

  protected:
    std::set<AObject*> _anaObj;
    std::set<AWindow*> _anaWin;
    AimsRGB _color;
    /// color index
    int _indCol;
    bool _destroying;
    aims::PythonHeader *_header;
    /// Retourne une nouvelle couleur.
    AimsRGB NewColor();
    std::string _filename;
  };

}

namespace carto
{
  DECLARE_GENERIC_OBJECT_TYPE( anatomist::Referential * )
  DECLARE_GENERIC_OBJECT_TYPE( std::set<anatomist::Referential *> )
  DECLARE_GENERIC_OBJECT_TYPE( std::vector<anatomist::Referential *> )
  DECLARE_GENERIC_OBJECT_TYPE( std::list<anatomist::Referential *> )
}

#endif
