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


#ifndef ANA_OBJECT_TEXTUREPANEL_H
#define ANA_OBJECT_TEXTUREPANEL_H


#include <anatomist/observer/Observer.h>
#include <qwidget.h>
#include <set>


namespace anatomist
{
  class AObject;
}


/* Texture properties panel for textured objects
 */
class QTexturePanel : public QWidget, public anatomist::Observer
{ 
  Q_OBJECT

public:
  enum Part
  {
    Mode, 
    Rate, 
    Filtering, 
    Generation, 
    Interpolation //, 
  };

  QTexturePanel( const std::set<anatomist::AObject *> &, 
                 QWidget* parent, const char *name = 0 );
  virtual ~QTexturePanel();

  void setActiveTexture( unsigned tex );
  unsigned activeTexture() const;
  void setObjects( const std::set<anatomist::AObject *> & );
  const std::set<anatomist::AObject *> & objects() const;
  virtual void update( const anatomist::Observable* observable, void* arg );
  void setVisibility( Part p, bool x );
  bool isVisible( Part p ) const;
  void updateObjects();
  void setAllowedTextureModes( const std::vector<int> & );

protected:
  void updateWindow();
  void runCommand();
  void updateAutoTexParams();
  virtual void unregisterObservable( anatomist::Observable* );

protected slots:
  void modeChanged( int );
  void filteringChanged( int );
  void generationChanged( int );
  void rateChanged( int );
  void rgbInterpolation( bool );
  void generationParamsDialog();

private:
  struct Private;
  Private	*d;
};

#endif

