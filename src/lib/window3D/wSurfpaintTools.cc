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
#include <anatomist/controler/icondictionary.h>
#include <anatomist/window3D/wSurfpaintTools.h>
#include <anatomist/window3D/window3D.h>
#include <anatomist/window/glwidget.h>
#include <aims/qtcompat/qhgroupbox.h>
#include <qlayout.h>
#include <aims/qtcompat/qvgroupbox.h>
#include <qcheckbox.h>
#include <aims/qtcompat/qvbuttongroup.h>
#include <qradiobutton.h>
#include <aims/qtcompat/qhbox.h>
#include <aims/qtcompat/qbuttongroup.h>
#include <aims/qtcompat/qbutton.h>
#include <qlabel.h>
#include <qspinbox.h>
#include <qtooltip.h>
#include <iostream>

using namespace anatomist;
using namespace std;

namespace
{

  void setButtonState(QButton* b, int x)
  {
    QCheckBox *cb = dynamic_cast<QCheckBox *> (b);
    if (!cb) return;
#if QT_VERSION >= 0x040000
    switch( x )
    {
      case 0:
      cb->setCheckState( Qt::Unchecked );
      break;
      case 1:
      cb->setCheckState( Qt::Checked );
      break;
      default:
      cb->setCheckState( Qt::PartiallyChecked );
    }
#else
    switch (x)
    {
      case 0:
        cb->setChecked(false);
        break;
      case 1:
        cb->setChecked(true);
        break;
      default:
        cb->setNoChange();
    }
#endif
  }

}

Infos3DTab::Infos3DTab(string textype, QWidget *parent)
     : QWidget(parent), _textype(textype)
{
  QVBoxLayout *mainlay = new QVBoxLayout;

  QHGroupBox *infos3D = new QHGroupBox(tr("Infos 3D :"), this);

  QLabel *SpinBoxLabel = new QLabel(tr("Texture Value : "), infos3D);

  if (_textype == "FLOAT")
  {
    QDoubleSpinBox *textureFloatSpinBox = new QDoubleSpinBox(infos3D);

    textureFloatSpinBox->setSingleStep(0.001);
    textureFloatSpinBox->setDecimals(3);
    textureFloatSpinBox->setFixedHeight(30);
    textureFloatSpinBox->setFixedWidth(100);
    textureFloatSpinBox->setValue(0.000);
    textureSpinBox = static_cast<QDoubleSpinBox*> (textureFloatSpinBox);
    connect  (textureFloatSpinBox,SIGNAL(valueChanged(double)),parent,SLOT(setTextureValueFloat(double)));
  }

  if (_textype == "S16" || (_textype == "U32") || (_textype == "S32"))
  {
    QSpinBox *textureIntSpinBox = new QSpinBox(infos3D);
    textureIntSpinBox->setSingleStep(1);
    textureIntSpinBox->setFixedHeight(30);
    textureIntSpinBox->setFixedWidth(75);
    textureIntSpinBox->setValue(0);
    textureSpinBox = static_cast<QSpinBox*>(textureIntSpinBox);
    connect(textureIntSpinBox,SIGNAL(valueChanged(int)),parent,SLOT(setTextureValueInt(int)));
  }

  QLabel *IDPolygonSpinBoxLabel = new QLabel(tr("ID Polygon : "),infos3D);

  IDPolygonSpinBox = new QSpinBox(infos3D);
  IDPolygonSpinBox->setSingleStep(1);
  IDPolygonSpinBox->setFixedHeight(30);
  IDPolygonSpinBox->setFixedWidth(75);
  IDPolygonSpinBox->setValue(0);

  QLabel *IDVertexSpinBoxLabel = new QLabel(tr("ID Vertex : "),infos3D);

  IDVertexSpinBox = new QSpinBox(infos3D);
  IDVertexSpinBox->setSingleStep(1);
  IDVertexSpinBox->setFixedHeight(30);
  IDVertexSpinBox->setFixedWidth(75);
  IDVertexSpinBox->setValue(0);

  mainlay->addWidget(infos3D);
  setLayout(mainlay);
}

ConstraintsEditorTab::ConstraintsEditorTab(QWidget *parent)
     : QWidget(parent)
{

  QVBoxLayout *mainlay = new QVBoxLayout;

  QWidget *widget = new QWidget;
  widget->setGeometry(QRect(10, 10, 300, 30));

  const QPixmap *p;
  p = IconDictionary::instance()->getIconInstance("buildIcon");

  QLabel *buildicon = new QLabel(widget);
  buildicon->setPixmap(*p);

  buildicon->setGeometry(QRect(10, 0, 50, 30));
  QLabel *buildtext = new QLabel(widget);
  buildtext->setText("module en construction");
  buildtext->setGeometry(QRect(60, 0, 200, 30));

  mainlay->addWidget(widget);
  setLayout(mainlay);
}

SurfpaintToolsWindow::SurfpaintToolsWindow(AWindow3D *win, string textype) :
  QWidget(0, "SurfpaintControl", Qt::WDestructiveClose), Observer(), _window(
      win), _textype(textype), destroying(false)
{
  //win->addObserver(this);
  setCaption(tr("SurfpaintControl") + _window->Title().c_str());

  const QPixmap *p;
  p = IconDictionary::instance()->getIconInstance("SurfpaintControl");
  this->setWindowIcon(*p);

  tabI3D = new Infos3DTab(_textype,this);
  tabCE = new ConstraintsEditorTab();

  tabWidget = new QTabWidget;
  tabWidget->addTab(tabI3D, tr("Infos3D"));
  tabWidget->addTab(tabCE, tr("ConstraintsEditor"));

  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addWidget(tabWidget);
  setLayout(mainLayout);

//mainlay->addWidget(buildicon);
//mainlay->addWidget(tbar);
//
}

SurfpaintToolsWindow::~SurfpaintToolsWindow()
{
  destroying = true;
//  _window->deleteObserver(this);
//  _window->painttoolsWinDestroyed();
//  cleanupObserver();
  //_window->deleteObserver( this );
  //delete tabI3d;
}

void SurfpaintToolsWindow::update(const Observable*, void* arg)
{
  cout << "SurfpaintToolsWindow::update\n";

//  if (arg == 0)
//  {
//    cout << "called obsolete SurfpaintToolsWindow::update( obs, NULL )\n";
//    delete this;
//    return;
//  }
//
//  blockSignals(true);
//
//  blockSignals(false);
}

void SurfpaintToolsWindow::unregisterObservable(Observable* o)
{
//  Observer::unregisterObservable(o);
//  if (!destroying) delete this;
}

void SurfpaintToolsWindow::setPolygon(int p)
{
  tabI3D->IDPolygonSpinBox->setValue(p);

  _window->Refresh();
}

void SurfpaintToolsWindow::setVertex(int v)
{
  tabI3D->IDVertexSpinBox->setValue(v);

  _window->Refresh();
}
float SurfpaintToolsWindow::getTextureValue(void)
{
  if (_textype == "S16" || (_textype == "U32") || (_textype == "S32")) return (float) (dynamic_cast<QSpinBox*> (tabI3D->textureSpinBox)->value());

  if (_textype == "FLOAT") return (float) (dynamic_cast<QDoubleSpinBox*> (tabI3D->textureSpinBox)->value());
}

void SurfpaintToolsWindow::setTextureValue(float v)
{
  if (_textype == "S16" || (_textype == "U32") || (_textype == "S32"))
    setTextureValueInt((int) v);

  if (_textype == "FLOAT") setTextureValueFloat((double) v);

  _window->Refresh();
}

void SurfpaintToolsWindow::setTextureValueInt(int v)
{
  dynamic_cast<QSpinBox*> (tabI3D->textureSpinBox)->setValue(v);

  _window->Refresh();
}

void SurfpaintToolsWindow::setTextureValueFloat(double v)
{
  dynamic_cast<QDoubleSpinBox*> (tabI3D->textureSpinBox)->setValue(v);

  _window->Refresh();
}

void SurfpaintToolsWindow::setMaxPoly(int max)
{
  tabI3D->IDPolygonSpinBox->setRange(0, max);
  _window->Refresh();
}

void SurfpaintToolsWindow::setMaxVertex(int max)
{
  tabI3D->IDVertexSpinBox->setRange(0, max);
  _window->Refresh();
}

void SurfpaintToolsWindow::setMinMaxTexture(float min, float max)
{
  if (_textype == "S16" || (_textype == "U32") || (_textype == "S32")) dynamic_cast<QSpinBox*> (tabI3D->textureSpinBox)->setRange(
      (int) min, (int) max);

  if (_textype == "FLOAT") dynamic_cast<QDoubleSpinBox*> (tabI3D->textureSpinBox)->setRange( min, max);

  _window->Refresh();
}
