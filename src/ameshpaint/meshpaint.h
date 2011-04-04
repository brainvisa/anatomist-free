#ifndef MESHPAINT_H
#define MESHPAINT_H

#include <QMainWindow>
#include <QtGui>
#include <cstdlib>
#include <iostream>
#include <string>
#include <sstream>
#include <string.h>
#include <aims/mesh/surfaceOperation.h>
#include <anatomist/application/globalConfig.h>
#include <anatomist/application/settings.h>
#include <aims/io/process.h>
#include <aims/io/finder.h>

#include <aims/def/path.h>
#include <cartobase/config/version.h>
#include <cartobase/config/paths.h>
#include <cartobase/stream/fileutil.h>

#include "glwidget.h"

using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;

class MeshPaint : public QMainWindow
{
  Q_OBJECT

public:
  MeshPaint();
  ~MeshPaint();

protected :
  void resizeEvent(QResizeEvent *event )
  {
	  event->accept();
  }
  virtual void DisplayConstraintList (void){};
  virtual void fillRegionOrPath(void){};
  virtual void changeMode(int mode){};
  virtual void saveTexture(void){};
  virtual void clearAll(void){};

private slots:

  void updateConstraintList () {DisplayConstraintList();}

  void trackball()
  {
    popAllButtonPaintToolBar();
    trackballAction->setChecked(true);
  	changeMode(1);
  }

  void colorPicker()
  {
    popAllButtonPaintToolBar();
    colorPickerAction->setChecked(true);
    changeMode(2);
  }

  void geodesicDistance()
  {
    popAllButtonPaintToolBar();
    geodesicDistanceAction->setChecked(true);
    changeMode(12);
  }

  void brush()
  {
    popAllButtonPaintToolBar();
    brushButton->setChecked(true);
    changeMode(_modeBrush);
  }

  void paintBrush()
  {
    popAllButtonPaintToolBar();
    string iconname = Settings::findResourceFile(
      "icons/meshPaint/stylo.png" );
    brushButton->setIcon(QIcon(iconname.c_str()));
    brushButton->setChecked(true);
    cout << "paint brush\n";
    _modeBrush = 9;
    changeMode(_modeBrush);
  }

  void magicBrush()
  {
    popAllButtonPaintToolBar();
    string iconname = Settings::findResourceFile(
      "icons/meshPaint/magic_pencil.png" );
    brushButton->setIcon(QIcon(iconname.c_str()));
    brushButton->setChecked(true);
    cout << "magic brush\n";
    _modeBrush = 10;
    changeMode(_modeBrush);
  }

  void erase()
  {
    popAllButtonPaintToolBar();
    string iconname = Settings::findResourceFile(
      "icons/meshPaint/erase.png" );
    brushButton->setIcon(QIcon(iconname.c_str()));
    brushButton->setChecked(true);
    cout << "eraser\n";
    _modeBrush = 8;
    changeMode(_modeBrush);
  }

  void shortPath()
  {
    popAllButtonPaintToolBar();
    string iconname = Settings::findResourceFile(
      "icons/meshPaint/shortest.png" );
    pathButton->setIcon(QIcon(iconname.c_str()));
    pathButton->setChecked(true);
    cout << "shortPath\n";
    _modePath = 4;
    changeMode(4);
  }

  void sulciPath()
  {
    popAllButtonPaintToolBar();
    string iconname = Settings::findResourceFile(
      "icons/meshPaint/sulci.png" );
    pathButton->setIcon(QIcon(iconname.c_str()));
    pathButton->setChecked(true);
    cout << "sulciPath\n";
    _modePath = 5;
    changeMode(5);
  }

  void gyriPath()
  {
    popAllButtonPaintToolBar();
    string iconname = Settings::findResourceFile( "icons/meshPaint/gyri.png" );
    pathButton->setIcon(QIcon(iconname.c_str()));
    pathButton->setChecked(true);
    cout << "gyriPath\n";
    _modePath = 6;
    changeMode(6);
  }

  void filling()
  {
    fillRegionOrPath();
  }

  void path()
  {
    popAllButtonPaintToolBar();
    pathButton->setChecked(true);
    changeMode(_modePath);
  }

  void selection()
  {
    popAllButtonPaintToolBar();
    selectionAction->setChecked(true);
    changeMode(7);
  }

  void clear()
  {
    clearAll();
  }

  void save()
  {
    saveTexture();
  }

protected :
  QToolBar *paintToolBar;
  QToolBar *infosToolBar;

  QSpinBox *toleranceSpinBox;
  QLabel *toleranceSpinBoxLabel;
  QDoubleSpinBox *constraintPathSpinBox;
  QLabel *constraintPathSpinBoxLabel;
  QDoubleSpinBox *sigmoPathSpinBox;
  QLabel *sigmoPathSpinBoxLabel;

  QToolButton *pathButton;
  QToolButton *brushButton;

  QSpinBox *distanceSpinBox;
  QLabel *distanceSpinBoxLabel;

public :
  QComboBox *constraintList;

private :
  void popAllButtonPaintToolBar ();
  void createActions();
  void createToolBars();

  int _modePath;
  int _modeBrush;

  QAction *trackballAction;

  QAction *colorPickerAction;
  QAction *selectionAction;

  QAction *geodesicDistanceAction;

  //QAction *pathAction;
  QAction *shortPathAction;
  QAction *sulciPathAction;
  QAction *gyriPathAction;

  //QAction *brushAction;
  QAction *paintBrushAction;
  QAction *magicBrushAction;
  QAction *fillAction;
  QAction *clearAction;
  QAction *eraseAction;
  QAction *saveAction;
};

template<typename T>
class myMeshPaint : public MeshPaint
{
public:
  myMeshPaint(string adressTexIn,string adressMeshIn,string adressTexCurvIn,string adressTexOut,string colorMap, string dataType);
  ~myMeshPaint();

  void fillRegionOrPath(void);
  void changeMode(int mode);
  void saveTexture(void);
  void clearAll(void);
  void keyPressEvent( QKeyEvent* event );

protected :
  void DisplayConstraintList ();

public :
  QWidget *textureSpinBox;
  QSpinBox *IDPolygonSpinBox;
  QSpinBox *IDVertexSpinBox;

private :
  string _adressTexIn;
  string _adressMeshIn;
  string _adressTexCurvIn;
  string _adressTexOut;
  string _colorMap;
  string _dataType;

  myGLWidget<T> *glWidget;
};

#endif // MESHPAINT_H
