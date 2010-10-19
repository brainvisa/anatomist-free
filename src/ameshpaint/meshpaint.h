#ifndef MESHPAINT_H
#define MESHPAINT_H

#include <QMainWindow>
#include <QtGui>
#include <cstdlib>
#include <iostream>
#include <string.h>
#include <aims/mesh/surfaceOperation.h>
#include <anatomist/application/globalConfig.h>
#include <anatomist/application/settings.h>
#include <aims/io/process.h>
#include <aims/io/finder.h>

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
	  trackballAction->setChecked(true);
	  paintBrushAction->setChecked(false);
	  colorPickerAction->setChecked(false);
  }

  virtual void changeMode(int mode){};
  virtual void saveTexture(void){};

private slots:

  void trackball()
  {
    paintBrushAction->setChecked(false);
  	colorPickerAction->setChecked(false);
  	pathButton->setChecked(false);
    clearAction->setChecked(false);
    fillAction->setChecked(false);
  	changeMode(1);
  }

  void colorPicker()
  {
    paintBrushAction->setChecked(false);
    trackballAction->setChecked(false);
    pathButton->setChecked(false);
    clearAction->setChecked(false);
    fillAction->setChecked(false);
    changeMode(2);
  }

  void paintBrush()
  {
    colorPickerAction->setChecked(false);
    trackballAction->setChecked(false);
    pathButton->setChecked(false);
    clearAction->setChecked(false);
    fillAction->setChecked(false);
    changeMode(3);
  }

  void shortPath()
  {
    colorPickerAction->setChecked(false);
    trackballAction->setChecked(false);
    paintBrushAction->setChecked(false);
    clearAction->setChecked(false);
    fillAction->setChecked(false);
    string iconname = Settings::globalPath() + "/icons/meshPaint/shortest.png";
    pathButton->setIcon(QIcon(iconname.c_str()));
    pathButton->setChecked(true);
    cout << "shortPath\n";
    _mode = 4;
    changeMode(_mode);
  }

  void sulciPath()
  {
    colorPickerAction->setChecked(false);
    trackballAction->setChecked(false);
    paintBrushAction->setChecked(false);
    clearAction->setChecked(false);
    fillAction->setChecked(false);
    string iconname = Settings::globalPath() + "/icons/meshPaint/sulci.png";
    pathButton->setIcon(QIcon(iconname.c_str()));
    pathButton->setChecked(true);
    cout << "sulciPath\n";
    _mode = 5;
    changeMode(_mode);
  }

  void gyriPath()
  {
    colorPickerAction->setChecked(false);
    trackballAction->setChecked(false);
    paintBrushAction->setChecked(false);
    clearAction->setChecked(false);
    fillAction->setChecked(false);
    string iconname = Settings::globalPath() + "/icons/meshPaint/gyri.png";
    pathButton->setIcon(QIcon(iconname.c_str()));
    pathButton->setChecked(true);
    cout << "gyriPath\n";
    _mode = 6;
    changeMode(_mode);
  }

  void fill()
  {
    colorPickerAction->setChecked(false);
    trackballAction->setChecked(false);
    paintBrushAction->setChecked(false);
    clearAction->setChecked(false);
    pathButton->setChecked(false);
    cout << "fill\n";
    changeMode(7);
  }

  void path()
  {
    colorPickerAction->setChecked(false);
    trackballAction->setChecked(false);
    paintBrushAction->setChecked(false);
    fillAction->setChecked(false);
    clearAction->setChecked(false);
    changeMode(_mode);
  }

  void clear()
  {
    colorPickerAction->setChecked(false);
    trackballAction->setChecked(false);
    paintBrushAction->setChecked(false);
    fillAction->setChecked(false);
    pathButton->setChecked(false);
    changeMode(8);
  }

  void save()
  {
    saveTexture();
  }

protected :
  QToolBar *paintToolBar;
  QToolBar *infosToolBar;

private :
  void createActions();
  void createToolBars();

  int _mode;

  QAction *colorPickerAction;
  QAction *paintBrushAction;
  QAction *trackballAction;
  QAction *clearAction;

  QToolButton *pathButton;

  QAction *pathAction;
  QAction *shortPathAction;
  QAction *sulciPathAction;
  QAction *gyriPathAction;

  QAction *fillAction;

  QAction *saveAction;
};

template<typename T>
class myMeshPaint : public MeshPaint
{
public:
  myMeshPaint(string adressTexIn,string adressMeshIn,string adressTexCurvIn,string adressTexOut,string colorMap, string dataType);
  ~myMeshPaint();

  void changeMode(int mode);
  void saveTexture(void);
  void keyPressEvent( QKeyEvent* event );

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
