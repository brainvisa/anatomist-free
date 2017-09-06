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
#include <anatomist/ui/ui_vectorfield.h>


namespace anatomist
{
  class AObject;
  class VectorField;
}


class VectorFieldEditionWindow : public QWidget, public Ui::VectorField,
  public anatomist::Observer
{
  Q_OBJECT

public:
  VectorFieldEditionWindow( const std::set<anatomist::AObject *> &,
                            QWidget* parent = 0, const char *name = 0,
                            Qt::WindowFlags f = 0 );
  virtual ~VectorFieldEditionWindow();

  const std::set<anatomist::AObject*>& objects() const { return _parents; }

  void update( const anatomist::Observable* observable, void* arg );
  void updateInterface();
  anatomist::VectorField* vectorField() const;

protected slots:
  void chooseObject();
  void objectsChosen( const std::set<anatomist::AObject *> & );

  void scalingChanged();
  void xVolumeChanged( int );
  void yVolumeChanged( int );
  void zVolumeChanged( int );
  void xSpace0Changed( int );
  void xSpace1Changed( int );
  void xSpace2Changed( int );
  void ySpace0Changed( int );
  void ySpace1Changed( int );
  void ySpace2Changed( int );
  void zSpace0Changed( int );
  void zSpace1Changed( int );
  void zSpace2Changed( int );
  void setFixedCoord( int chan, int coord, int value );

protected:
  virtual void unregisterObservable( anatomist::Observable* );
  void setVolume( int chan, int index );
  void setSpaceDim( int chan, int dim, int index );

  std::set<anatomist::AObject *>	_parents;

private:
  struct Private;
  Private	*d;
};



namespace anatomist
{
  namespace internal
  {

    class VectorFieldCoordSpinBox : public QSpinBox
    {
      Q_OBJECT

    public:
      VectorFieldCoordSpinBox( int channel, int coord, QWidget* parent = 0 )
        : QSpinBox( parent ), channel( channel ), coord( coord )
      {
        connect( this, SIGNAL( valueChanged( int ) ),
                this, SLOT( changeValue( int ) ) );
      }
      virtual ~VectorFieldCoordSpinBox();

    signals:
      void valueChanged( int, int, int );

    protected slots:
      void changeValue( int value )
      {
        emit valueChanged( channel, coord, value );
      }

    private:
      int channel;
      int coord;
    };

  }
}



#endif

