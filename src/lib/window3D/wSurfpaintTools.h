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

#ifndef ANATOMIST_WINDOW3D_WSURFPAINTTOOLS_H
#define ANATOMIST_WINDOW3D_WSURFPAINTTOOLS_H

#include <anatomist/observer/Observer.h>
#include <qwidget.h>
#include <qspinbox.h>

using namespace std;

class AWindow3D;
class QTabWidget;
class QSpinBox;

 class Infos3DTab : public QWidget
 {
     Q_OBJECT

 public:
     Infos3DTab(string textype, QWidget *parent);

 public:
     QWidget *textureSpinBox;
     QSpinBox *IDPolygonSpinBox;
     QSpinBox *IDVertexSpinBox;
     string _textype;
 };

 class ConstraintsEditorTab : public QWidget
 {
     Q_OBJECT

 public:
     ConstraintsEditorTab(QWidget *parent = 0);
 };

///	Settings for 3D windows
class SurfpaintToolsWindow: public QWidget, public anatomist::Observer
{
  Q_OBJECT

  public:
    SurfpaintToolsWindow(AWindow3D *win, string t);
    virtual ~SurfpaintToolsWindow();

    virtual void update(const anatomist::Observable* observable, void* arg);

  public slots:
    float getTextureValue(void);
    void setTextureValue(float v);
    void setTextureValueInt(int v);
    void setTextureValueFloat(double v);

    void setMinMaxTexture(float min, float max);

    void setPolygon(int p);
    void setMaxPoly(int max);
    void setVertex(int v);
    void setMaxVertex(int max);

  protected:
    AWindow3D *_window;

    virtual void unregisterObservable(anatomist::Observable*);

  private:
    QTabWidget *tabWidget;
    Infos3DTab *tabI3D;
    ConstraintsEditorTab *tabCE;

    string _textype;
    bool destroying;
};

#endif
