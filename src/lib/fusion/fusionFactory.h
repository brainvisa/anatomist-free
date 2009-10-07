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


#ifndef ANA_FUSION_FUSIONFACTORY_H
#define ANA_FUSION_FUSIONFACTORY_H


#include <list>
#include <set>
#include <vector>
#include <string>


namespace anatomist
{

  class AObject;


  /**	Fusion method interface.
	Tells if it can fusion a set of objects together, and if so creates 
	the appropriate fusion object
  */
  class FusionMethod
  {
  public:
    FusionMethod() {}
    virtual ~FusionMethod();

    virtual bool canFusion( const std::set<AObject *> & ) = 0;
    ///	creates the fusion
    virtual AObject* fusion( const std::vector<AObject *> & ) = 0;
    ///	identifier for the method
    virtual std::string ID() const = 0;
    virtual bool orderingMatters() const;
  };


  /**	Handles creation of various fusion objects.
	This is an "open" mechanism with registration procedure to allow new 
	types of fusion
  */
  class FusionFactory
  {
  public:
    FusionFactory();
    virtual ~FusionFactory();

    /**	Chooses one method which can fusion these objects.
	The default factory only takes the first found (random!...)
	@return	0 if none can do so
    */
    virtual FusionMethod* chooseMethod( const std::set<AObject *> & );
    virtual FusionMethod* chooseMethod( std::vector<AObject *> &, 
                                        bool allowreorder = false );
    ///	Selects the method with ID name (if any)
    virtual FusionMethod* method( const std::string & name ) const;

    ///
    static FusionFactory* factory() { return( _theFactory ); }
    ///	returns false if the method was already registered
    static bool registerMethod( FusionMethod* method );
    /**	is there at least one method which can actually make a fusion with 
	these objects ? */
    static bool canFusion( const std::set<AObject *> & );

  protected:
    static FusionFactory		*_theFactory;
    static std::set<FusionMethod *>	_methods;

  private:
  };

}


#endif
