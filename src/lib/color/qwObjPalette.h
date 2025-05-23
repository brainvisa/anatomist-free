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


#ifndef ANAQT_COLOR_QWOBJPALETTE_H
#define ANAQT_COLOR_QWOBJPALETTE_H


#include <anatomist/color/wObjPalette.h>
#include <cartobase/smart/rcptr.h>
#include <qwidget.h>
#include <qaction.h>

class QListWidgetItem;

namespace anatomist
{
  class AObjectPalette;
  class APalette;
  class APaletteExtensionAction;
}


class QAPaletteWin : public QWidget, public anatomist::APaletteWin
{
  Q_OBJECT

public:
  QAPaletteWin( const std::set<anatomist::AObject *> & obj );
  virtual ~QAPaletteWin();

  virtual void update( const anatomist::Observable* obs, void* arg );

  static anatomist::APaletteWin* 
  createPalWin( const std::set<anatomist::AObject* > & obj );
  /** Actions can be added here, they will show on a toolbar.
      Actions will call their extensionTriggered() method when used.
      Signals are not used due to difficulties of python bindings for custom 
      template types (std::set<anatomist::AObject *>)
  */
  static void addExtensionAction( anatomist::APaletteExtensionAction* action );

signals:

protected slots:
  void palette1Changed( const std::string & );
  void palette2Changed( const std::string & );
  void responsiveToggled( bool val );
  void updateClicked();
  void dimChanged( int );
  void mixMethodChanged( int );
  void palette1DMappingMethodChanged( int );
  void enablePalette2( bool );
  void mixFactorChanged( int );
  void min1EditChanged();
  void max1EditChanged();
  void min2EditChanged();
  void max2EditChanged();
  void resetValues1();
  void resetValues2();
  void resetBounds1();
  void resetBounds2();
  void chooseObject();
  void objectsChosen( const std::set<anatomist::AObject *> & );
  void extensionActionTriggered( QAction* action );
  void zeroCentered1Changed( int state );
  void zeroCentered2Changed( int state );
  void palette1RangeChanged( float, float );
  void palette2RangeChanged( float, float );
  void cleatMin1();
  void cleatMax1();
  void cleatMin2();
  void cleatMax2();
  void min1Changed( float );
  void max1Changed( float );
  void min2Changed( float );
  void max2Changed( float );

protected:
  struct DimBox;

  virtual void closeEvent( QCloseEvent* );
  void updateInterface();

  virtual void unregisterObservable( anatomist::Observable* );

  void fillPalette1();
  void fillPalette2();
  anatomist::AObjectPalette* objPalette();
  virtual QWidget* makeDimBox( const QString & title, QWidget* parent, 
                               DimBox* dbox, bool secondary );
  virtual void setValues( DimBox* dimBox, float min, float max, 
                          float objMin, float objMax, bool zeroCentered );
  virtual void setValues1();
  virtual void setValues2();
  virtual void updateObjects();
  virtual void updateObjPal();
  void fillMixMethods();
  void fillPalette1DMappingMethods() ;
  void runCommand();
  void fillToolBar();

private:
  struct Private;
  Private	*d;
};


namespace anatomist
{

  class APaletteExtensionAction : public QAction
  {
    Q_OBJECT

  public:
    friend class ::QAPaletteWin;

    APaletteExtensionAction( QObject* parent );
    APaletteExtensionAction( const QString & text, QObject * parent );
    APaletteExtensionAction( const QIcon & icon, const QString & text, 
                             QObject * parent );
    virtual ~APaletteExtensionAction();

    virtual void extensionTriggered( 
      const std::set<anatomist::AObject *> & ) = 0;
  };


  namespace internal
  {

    // static class
    class PaletteWinExtensionActions : public QObject
    {
      Q_OBJECT

    public:
      virtual ~PaletteWinExtensionActions() { _instance() = 0; }

      static PaletteWinExtensionActions *instance();

    private:
      PaletteWinExtensionActions( QObject * parent ) : QObject( parent ) {}
      static PaletteWinExtensionActions *& _instance();
    };

  }

}

#endif
