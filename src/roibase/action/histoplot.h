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

#ifndef HISTO_PLOT_H
#define HISTO_PLOT_H

#include <anatomist/observer/Observer.h>
#include <aims/qtcompat/qvbox.h>
#include <vector>
#include <map>
#include <string>

class RoiHistoPlot : public QVBox, public anatomist::Observer 
{
  Q_OBJECT
  
 public:
  RoiHistoPlot( QWidget * parent, int nbOfBins ) ;
  virtual ~RoiHistoPlot() ;
    
  int nbOfBins( ) const { return myNbOfBins ; }
  void setNbOfBins( int nbOfBins ) { myNbOfBins = nbOfBins ; }
  
  void printHistos( const std::string& filename ) ;
  virtual void update( const anatomist::Observable *, void * ) ;
  
  void activate() ;
  void deactivate() ;
  
 public slots :
  void nbOfBinsChanged( int ) ;
  void lowChanged( float low ) ;
  void highChanged( float high ) ;
  void showGraphHisto() ;
  void saveHistos() ;
  void showImageHisto() ;
  void showHistoChange( ) ;
  void ignoreUnderLowChicked( ) ;
 private:
  struct Private;

  // Try and think to reparent plot curves.
  std::vector<float> 
  getImageHisto( const std::string& image, int& nbOfPoints, 
		 float& binSize, int time = 0 ) ;
  std::map< std::string, std::vector<float> > 
  getGraphHisto( const std::string& image, const std::string& graph, 
		 std::map< std::string, int>& nbOfPoints, 
		 std::map< std::string, float>& meanValue, 
		 std::map< std::string, float>& stdDeviation,
		 float& binSize, 
		 int time = 0 ) ;
  std::vector<float> 
  getRegionHisto( const std::string& image, const std::string& graph,
		  const std::string& region, int& nbOfPoints,
		  float& meanValue, float& stdDev,
		  float& binSize, int time = 0 ) ;
  
  std::map< std::string, std::vector<float> > myImageHistos ;
  std::map< std::string, int > myNbOfPoints ;


  std::string myImage ;
  std::string myGraph ;
  float myIgnoreForMax ;
  int myNbOfBins ;
  double myHistoMax ;
  double myHighLevel ;
  double myLowLevel ;
  bool myActivate ;
  bool myShowImageHisto ;

  Private * _private ;
} ;


#endif
