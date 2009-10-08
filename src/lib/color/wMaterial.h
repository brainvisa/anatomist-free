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


#ifndef ANA_COLOR_WMATERIAL_H
#define ANA_COLOR_WMATERIAL_H

#include <anatomist/observer/Observer.h>
#include <anatomist/color/Material.h>
#include <qslider.h>
#include <set>


namespace anatomist
{
  class AObject;
}


/**	Material aspect (color / transparency) tuning window
 */
class MaterialWindow : public QWidget, public anatomist::Observer
{
  Q_OBJECT

public:
  MaterialWindow( const std::set<anatomist::AObject *> &, QWidget* parent = 0, 
		  const char *name = 0 );
  virtual ~MaterialWindow();

  const std::set<anatomist::AObject*>& objects() const { return _parents; }
  const anatomist::Material& getMaterial() const { return _material; }

  void update( const anatomist::Observable* observable, void* arg );
  void updateObjects();
  void updateInterface();
  void runCommand();

public slots:
  /// called by the QTabBar change
  void enableTab( int );
  void enableAutoUpdate( bool );

protected slots:
  void valueChanged( int panel, int color, int value );
  void shininessChanged( int value );
  void chooseObject();
  void objectsChosen( const std::set<anatomist::AObject *> & );
  void renderModeChanged( int );
  void renderPropertyChanged( int );

protected:
  virtual void unregisterObservable( anatomist::Observable* );
  void drawContents();
  ///	internal: draws a RGBA widgets group
  QWidget* buildRgbaBox( QWidget* parent, int num );
  ///	internal: updates the sliders / labels positions for one RGBA box
  void updatePanel( unsigned panel );

  std::set<anatomist::AObject *>	_parents;
  anatomist::Material			_material;

private:
  struct Private;
  Private	*_privdata;
};


/**	A private class: slider with specific data storage (for callbacks 
	client data), no use for it anywhere else... 
	It should be entirely in the .cc, but we need to improve the Makefile 
	to compile MOCs in .cc */
class QANumSlider : public QSlider
{
  Q_OBJECT

public:
  QANumSlider( int num1, int num2, int minValue, int maxValue, int pageStep, 
               int value, Qt::Orientation, QWidget * parent, 
               const char * name=0 );
  virtual ~QANumSlider() {}

  int		_num1;
  int		_num2;

signals:
  void valueChanged( int num1, int num2, int value );

protected slots:
  void transformChange( int );
};


#endif
