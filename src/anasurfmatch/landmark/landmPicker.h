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


#ifndef ANATOMIST_LANDMARK_LANDMPICKER_H
#define ANATOMIST_LANDMARK_LANDMPICKER_H


#include <anatomist/observer/Observer.h>
#include <aims/vector/vector.h>
#include <qwidget.h>
#include <set>


namespace anatomist
{
  class AObject;
  class GLComponent;
  //class APointCollector;

  struct ALandmarkPicker_privateData;
}


/** Tool for selecting landmarks on a surface
 */
class ALandmarkPicker : public QWidget, public anatomist::Observer
{
  Q_OBJECT

public:
  ALandmarkPicker( const std::set<anatomist::AObject *> & obj );
  virtual ~ALandmarkPicker();

  virtual void update( const anatomist::Observable* obs, void* arg );
  ///	Callback function to start it from the GUI
  static void startInterface( const std::set<anatomist::AObject *> & obj );

public slots:
  void pickPoint();

protected:
  /**	Finds nearest vertex from given point in object, using euclidian 
	distance
	@param	obj	object to look in
	@param	pt	3D point
	@param	index	(return) index of the nearest vertex of obj
	@return		minimum distance
  */
  float nearestEuclidian( anatomist::GLComponent* obj, const Point3df & pt, 
			  unsigned & index, Point3df & vpos );
  /**	Finds nearest vertex in all observed objects according to current 
	distance mode */
  anatomist::AObject* nearestVertex( const Point3df & pt, unsigned & index, 
				     float & dist, Point3df & vpos );
  void createLandmark( const Point3df & pt );
  virtual void unregisterObservable( anatomist::Observable* );

  std::set<anatomist::AObject *>		_obj;
  anatomist::ALandmarkPicker_privateData	*_privdata;

private:
  //static void clickPointCbk( APointCollector* caller, void *clientdata );
};


#endif
