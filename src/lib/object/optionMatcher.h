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


#ifndef ANA_OBJECT_OPTIONMATCHER_H
#define ANA_OBJECT_OPTIONMATCHER_H


#include <set>

class Tree;

namespace anatomist
{

  class AObject;


  /**	Option menus registration mechanism (all static).
	Makes the intersection of the option menu trees for a set of objects. 
	The menu tree is the option menu for this object: a tree of strings 
	(syntactic attribute for each node), and each leaf must have a 
	callback function of type OptionFunction stored in the attribute 
	"callback".
	Typically, each object class has a static tree, a pointer to which 
	is returned by the optionTree() function. In special cases the tree 
	can be instance-dependent.
  */
  class OptionMatcher
  {
  public:
    ///
    typedef void (*OptionFunction)( const std::set<AObject *> & );

    ///	Fills the return tree tr with the common options of the set of objects
    static void commonOptions( const std::set<AObject *> &, Tree & tr );
    /**	Makes the intersection between tr and ot, result in tr.
	ot may be null, if so tr is empty after intersection
    */
    static void intersect( Tree & tr, const Tree* ot );
    /**	Copies 'tin' into 'tout', with only syntactic attribute and
        "objectmenucallback" or "callback" attribute */
    static void copyTree( Tree & tout, const Tree & tin );

  protected:

  private:
  };

}


#endif
