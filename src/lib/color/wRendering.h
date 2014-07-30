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


#ifndef ANA_COLOR_WRENDERING_H
#define ANA_COLOR_WRENDERING_H

#include <anatomist/observer/Observer.h>
#include <anatomist/color/Material.h>
#include <anatomist/surface/Shader.h>
#include <anatomist/ui/ui_color_rendering.h>
#include <qslider.h>
#include <set>


namespace anatomist
{
  class AObject;
}


/**	Rendering aspect (shading/lighting model, effects) tuning window
 */
class RenderingWindow : public QWidget, public Ui::RenderingWindow, public anatomist::Observer
{
  Q_OBJECT

public:
  RenderingWindow( const std::set<anatomist::AObject *> &, QWidget* parent = 0, 
		  const char *name = 0, Qt::WindowFlags f = 0 );
  virtual ~RenderingWindow();

  const std::set<anatomist::AObject*>& objects() const { return _parents; }
  const anatomist::Material& getMaterial() const { return _material; }
  const anatomist::Shader& getShader() const { return _shader; }

  void update( const anatomist::Observable* observable, void* arg );
  void updateObjectsRendering();
  void updateInterface();
  void runCommand();

protected slots:
  void chooseObject();
  void objectsChosen( const std::set<anatomist::AObject *> & );

  void enableShadersClicked( int x );
  void renderModeChanged( int x );
  void renderPropertyChanged( int x );
  void lightingModelChanged( int x );
  void interpolationModelChanged( int x );
  void coloringModelChanged( int x );
  void selectionModeChanged( int x );
  void lineWidthChanged();
  void unlitColorClicked();

  void reloadClicked(void);

protected:
  void updateObjectsShading();
  void removeObjectsShading();
  virtual void unregisterObservable( anatomist::Observable* );

  std::set<anatomist::AObject *>	_parents;
  anatomist::Material			_material;
  anatomist::Shader			_shader;

private:
  struct Private;
  Private	*_privdata;
};


#endif
