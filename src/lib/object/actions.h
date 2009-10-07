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


#ifndef ANA_OBJECT_ACTIONS_H
#define ANA_OBJECT_ACTIONS_H


#include <cartobase/object/object.h>

namespace anatomist
{

  class AObject;
  class ObjectMenuCallback;


  /**	Base callback functions for actions on the objects
   */
  class ObjectActions
  {
  public:
    typedef void (*ActionObjectCallback)( const std::set<AObject *> & );
    typedef void (*ActionUntypedCallback)( void * );

    static void fileReload( const std::set<AObject *> & obj );
    static void colorPalette( const std::set<AObject *> & obj );
    static void colorMaterial( const std::set<AObject *> & obj );
    static void referentialLoad( const std::set<AObject *> & obj );
    static void fusion2DControl( const std::set<AObject *> & obj );
    static void fusion3DControl( const std::set<AObject *> & obj );
    static void textureControl( const std::set<AObject *> & obj );
    static void saveStatic( const std::set<AObject *> & obj ) ;
    static std::string specificSaveStatic( const std::set<AObject *> & obj, 
					   const std::string& filter, 
					   const std::string & caption ) ;
    static void saveTexture( const std::set<AObject *> & obj ) ;
    static void extractTexture( const std::set<AObject *> & obj ) ;
    static std::string specificSaveTexture( const std::set<AObject *> & obj, 
                                            const std::string& filter, 
                                            const std::string & caption ) ;
    static void renameObject( const std::set<AObject *> & obj );
    static void generateTexture1D( const std::set<AObject *> & obj );
    static void generateTexture2D( const std::set<AObject *> & obj );
    static void displayGraphChildren( const std::set<AObject *> & obj );
    static void displayGraphRelations( const std::set<AObject *> & obj );
    static void loadGraphSubObjects( const std::set<AObject *> & obj );

    static bool askName( std::string & newname, const std::string & type = "", 
			 const std::string & origname = "" );
    static void setAutomaticReferential( const std::set<AObject *> & obj );
    static void graphLabelToName( const std::set<AObject *> & obj );
    static void graphUseLabel( const std::set<AObject *> & obj );
    static void graphUseName( const std::set<AObject *> & obj );
    static void graphUseDefaultLabelProperty( const std::set<AObject *>
        & obj );

    /// returns a callback object for the reload action
    static ObjectMenuCallback* fileReloadMenuCallback();
    static ObjectMenuCallback* colorPaletteMenuCallback();
    static ObjectMenuCallback* colorMaterialMenuCallback();
    static ObjectMenuCallback* referentialLoadMenuCallback();
    static ObjectMenuCallback* fusion2DControlMenuCallback();
    static ObjectMenuCallback* fusion3DControlMenuCallback();
    static ObjectMenuCallback* textureControlMenuCallback();
    static ObjectMenuCallback* saveStaticMenuCallback();
    static ObjectMenuCallback* saveTextureMenuCallback();
    static ObjectMenuCallback* extractTextureMenuCallback();
    static ObjectMenuCallback* renameObjectMenuCallback();
    static ObjectMenuCallback* generateTexture1DMenuCallback();
    static ObjectMenuCallback* generateTexture2DMenuCallback();
    static ObjectMenuCallback* displayGraphChildrenMenuCallback();
    static ObjectMenuCallback* setAutomaticReferentialMenuCallback();
    static ObjectMenuCallback* graphLabelToNameMenuCallback();
    static ObjectMenuCallback* graphUseLabelMenuCallback();
    static ObjectMenuCallback* graphUseNameMenuCallback();
    static ObjectMenuCallback* graphUseDefaultLabelPropertyMenuCallback();
  };

}

namespace carto
{
  DECLARE_GENERIC_OBJECT_TYPE( anatomist::ObjectActions::ActionObjectCallback )
  DECLARE_GENERIC_OBJECT_TYPE( anatomist::ObjectActions::ActionUntypedCallback )
}

#endif
