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


#ifndef ANAQT_HIERARCHY_HIERARCHY_H
#define ANAQT_HIERARCHY_HIERARCHY_H


#include <anatomist/object/Object.h>
#include <anatomist/graph/attribAObject.h>
#include <cartobase/object/syntax.h>
#include <graph/tree/tree.h>


namespace anatomist
{
  class AGraph;

  /**	Nomenclature hierarchy object
   */
  class Hierarchy : public AObject, public AttributedAObject
  {
  public:
    Hierarchy( Tree* tr );
    Hierarchy( carto::rc_ptr<Tree> tr );
    virtual ~Hierarchy();

    carto::rc_ptr<Tree> tree() { return( _tree ); };

    static AObject* loadHierarchy( const std::string & filename, 
                                   carto::Object options );

    ///	not a 2D object
    virtual bool Is2DObject() { return( false ); }
    ///	... neither a 3D...
    virtual bool Is3DObject() { return( false ); }
    static int classType() { return( _classType ); }
    virtual Tree* optionTree() const;

    virtual carto::GenericObject* attributed();
    virtual bool save( const std::string & filename );
    virtual void internalUpdate();

    Tree* findNamedNode( const std::string & name,
                         const std::list<Tree*>** parents = 0 ) const;

    static void namesUnder( Tree* tr, std::set<std::string> & names );
    /** finds a Hierarchy matching for edition of the given object
        (which should be a graph or graph element)
        @param ag if not null, will be set to the parent graph of obj
    */
    static Hierarchy* findMatchingNomenclature( const AObject* obj,
        const AGraph **ag = 0 );

  protected:
    carto::rc_ptr<Tree>	_tree;
    static Tree	*_optionTree;

  private:
    struct Private;
    struct PrivateStatic;
    Private *d;
    static PrivateStatic *staticStruct();
    void slotPropertyChanged( const carto::Object& sender,
                              const std::string& propertyName,
                              const carto::Object& oldValue );

    ///	ensures the object class is registered in Anatomist
    static int registerClass();
    static int		_classType;
    ///	Syntax rules
    static carto::SyntaxSet	*_syntax;
  };

}


#endif
