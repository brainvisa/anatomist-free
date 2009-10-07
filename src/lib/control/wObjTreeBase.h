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


#ifndef ANA_CONTROL_WOBJTREEBASE
#define ANA_CONTROL_WOBJTREEBASE


#include <set>


namespace anatomist
{

  class AObject;
  class MObject;


  /**	Abstract base class for object tree widget interface in control window
   */
  class ObjectTreeBase
  {
  public:
    ///	corresponds to Xt callbacks (XtCallbackProc type)
    typedef void (*Callback)( void*, void*, void* );

    virtual ~ObjectTreeBase();

    /**@name	Objects registration */
    //@{
    ///	adds the object at the base level of this tree
    virtual void RegisterObject( AObject* obj ) = 0;
    virtual void UnregisterObject( AObject* obj ) = 0;
    virtual void NotifyObjectChange( AObject* obj ) = 0;
    /**	adds the object in a sub-tree. \\
	(the corresponding sub-tree has to be searched for first)
	@param	mobj	immediate parent of obj
	@param	obj	object to map
    */
    virtual void RegisterSubObject( MObject *mobj, AObject *obj ) = 0;
    virtual void UnregisterSubObject( MObject *mobj, AObject *obj ) = 0;
    //@}

    virtual std::set<AObject *> *SelectedObjects() const = 0;
    virtual AObject* ObjectOfNumber( unsigned pos ) const = 0;
    virtual void SelectObject( AObject *obj ) = 0;
    virtual bool isObjectSelected( AObject* obj ) const = 0;
    virtual void UnselectAll() = 0;
    virtual void AddCallback( Callback callback, void* clientData ) 
      = 0;
    virtual void RemoveCallback( Callback callback, void* clientData ) 
      = 0;

    ///	Are reference colors markers visible ?
    virtual bool ViewingRefColors() const = 0;
    virtual void ToggleRefColorsView() = 0;
    virtual void DisplayRefColors() = 0;
    virtual void UndisplayRefColors() = 0;

  protected:
    ///	Protected constructor
    ObjectTreeBase();

  private:
  };

}


#endif
