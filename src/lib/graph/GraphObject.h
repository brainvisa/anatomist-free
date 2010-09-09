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


#ifndef ANA_GRAPH_GRAPHOBJECT_H
#define ANA_GRAPH_GRAPHOBJECT_H


#include <anatomist/mobject/globjectlist.h>
#include <anatomist/graph/attribAObject.h>


namespace anatomist
{

  class AGraph;


  /**	Anatomist graph object: a list that manages destruction of its 
	sub-elements
  */
  class AGraphObject : public GLObjectList, public AttributedAObject
  {
  public:
    ///	Modes d'affichage des sous-objets
    enum ShowType
    {
      ///	Affiche les triangulations seulement
      TRIANG, 
      ///	Affiche les buckets seulement
      BUCKET, 
      ///	Affiche tous les objets 3D
      ALL, 
      ///	Affiche seulement le 1er objet 3D de la liste
      FIRST
    };

    AGraphObject( carto::GenericObject* go );
    virtual ~AGraphObject();

    /**@name	Identification handling functions */
    //@{
    virtual int MType() const { return AObject::GRAPHOBJECT; }
    //@}

    /**@name	Visualization */
    //@{
    virtual GLPrimitives glMainGLL( const ViewState & );
    ///	Visu mode (default is TRIANG)
    static ShowType showType() { return( _showType ); }
    ///	Control the visualization mode
    static void setShowType( ShowType type ) { _showType = type; }
    //@}

    virtual const Material & material() const;
    virtual Material & GetMaterial();
    virtual void SetMaterial( const Material & mat );
    virtual void SetMaterialOrDefault( const AGraph* agr, 
				       const Material & mat );

    virtual carto::GenericObject* attributed() { return( _gobject.get() ); }
    virtual const carto::GenericObject* attributed() const
    { return( _gobject.get() ); }
    virtual void internalUpdate();

  protected:
    carto::rc_ptr<carto::GenericObject>	_gobject;

  private:
    ///	Mode d'affichage
    static ShowType	_showType;
  };

}


namespace carto
{
#define _TMP_ std::map<anatomist::AObject *, std::string>
  DECLARE_GENERIC_OBJECT_TYPE( _TMP_ )
#undef _TMP_
}

#endif
