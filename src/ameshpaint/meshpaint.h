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

  void paintBrush()
  {
    popAllButtonPaintToolBar();
    paintBrushAction->setChecked(true);
    changeMode(3);
  }

  void shortPath()
  {
    popAllButtonPaintToolBar();
    string iconname = Settings::globalPath() + "/icons/meshPaint/shortest.png";
    pathButton->setIcon(QIcon(iconname.c_str()));
    pathButton->setChecked(true);
    cout << "shortPath\n";
    _mode = 4;
    changeMode(4);
  }

  void sulciPath()
  {
    popAllButtonPaintToolBar();
    string iconname = Settings::globalPath() + "/icons/meshPaint/sulci.png";
    pathButton->setIcon(QIcon(iconname.c_str()));
    pathButton->setChecked(true);
    cout << "sulciPath\n";
    _mode = 5;
    changeMode(5);
  }

  void gyriPath()
  {
    popAllButtonPaintToolBar();
    string iconname = Settings::globalPath() + "/icons/meshPaint/gyri.png";
    pathButton->setIcon(QIcon(iconname.c_str()));
    pathButton->setChecked(true);
    cout << "gyriPath\n";
    _mode = 6;
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
    changeMode(_mode);
  }

  void selection()
  {
    popAllButtonPaintToolBar();
    selectionAction->setChecked(true);
    changeMode(7);
  }

  void clear()
  {
    popAllButtonPaintToolBar();
    clearAction->setChecked(true);
    changeMode(8);
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
  QSpinBox *constraintPathSpinBox;
  QLabel *constraintPathSpinBoxLabel;

  QToolButton *pathButton;

public :
  QComboBox *constraintList;

private :
  void popAllButtonPaintToolBar ();
  void createActions();
  void createToolBars();

  int _mode;

  QAction *trackballAction;

  QAction *colorPickerAction;
  QAction *selectionAction;
  QAction *pathAction;
  QAction *shortPathAction;
  QAction *sulciPathAction;
  QAction *gyriPathAction;

  QAction *paintBrushAction;
  QAction *fillAction;
  QAction *clearAction;

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
