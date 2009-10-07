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


#ifndef ANATOMIST_REFERENCE_TRANSFSET_H
#define ANATOMIST_REFERENCE_TRANSFSET_H


#include <map>
#include <set>


namespace anatomist
{

  class Transformation;
  class Referential;
  class Observer;


  /**	List of all transformations. Singleton */
  class ATransformSet
  {
  public:
    ATransformSet();
    /// Deletes all registered transformations as well
    virtual ~ATransformSet();

    unsigned size() const;
    Transformation* transformation( const Referential* src, 
				    const Referential* dst );
    const Transformation* transformation( const Referential* src, 
					  const Referential* dst ) const;
    std::set<Transformation *> allTransformations() const;
    void registerTransformation( Transformation* t );
    void unregisterTransformation( Transformation* t );
    std::set<Transformation *> transformationsWith( const Referential* ref ) 
      const;
    void deleteTransformationsWith( const Referential* ref );
    bool hasTransformation( const Transformation* t ) const;
    void registerObserver( const Referential* src, const Referential* dst, 
                           Observer* o );
    void unregisterObserver( const Referential* src, const Referential* dst, 
                             Observer* o );
    void setTransformationChanged( const Referential* src, 
                                   const Referential* dst );
    void notifyTransformationChanged( const Referential* src, 
                                      const Referential* dst, 
                                      void* arg = 0, bool setchanged = false );

    static ATransformSet* instance();
    std::set<Referential *> connectedComponent( Referential* r ) const;

  private:
    struct Private;

    void completeTransformations( Transformation* t );
    void propagate( Referential* ref, Referential* r2, 
		    const std::set<Referential *> & others );
    void deleteGeneratedConnections( Referential* r1, Referential* r2 );

    Private	*d;
  };

}

#endif
