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


#ifndef ANATOMIST_LANDMARK_GRAPHLANDMARK_H
#define ANATOMIST_LANDMARK_GRAPHLANDMARK_H

#ifndef QT_CELAN_NAMESPACE
#define QT_CELAN_NAMESPACE
#endif
#include <qwidget.h>
#include <anatomist/observer/Observer.h>
#include <aims/vector/vector.h>
#include <set>

class Tree;

namespace anatomist
{
  class AObject;
  class AGraph;
  //class APointCollector;
  struct GraphLandmarkPicker_privateData;
}


//namespace anatomist
//{

///	Landmarks organized into a graph, associated with a 2D object
class GraphLandmarkPicker : public QWidget, public anatomist::Observer
{
  Q_OBJECT

public:
  GraphLandmarkPicker( anatomist::AGraph* ag, QWidget* parent = 0, 
                       const char* name = 0,
                       Qt::WindowFlags f = Qt::WindowFlags() );
  virtual ~GraphLandmarkPicker();

  static void addGraphLandmarkOptions( Tree* tr );
  static void startInterface( const std::set<anatomist::AObject *> & );

  virtual void update( const anatomist::Observable* obs, void* arg );

public slots:
  void pickLandmark();

protected:
  void createLandmark( const Point3df & pt );
  virtual void unregisterObservable( anatomist::Observable* );

private:
  //static void clickPointCbk( APointCollector* caller, void *clientdata );

  anatomist::AGraph				*_graph;
  anatomist::GraphLandmarkPicker_privateData	*pdat;
};

//}


#endif
