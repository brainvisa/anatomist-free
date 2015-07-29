/**************************************************************************
 * This file is part of the Fraqtive program.
 * Copyright (C) 2005 Michal Mecinski.
 * This program is licensed under the GNU General Public License.
 **************************************************************************/

#ifndef GRADCONTROL_H
#define GRADCONTROL_H

#include "gradient.h"

#include <sstream>
#include <qstring.h>
#include <qwidget.h>
#include <qrect.h>

/*!
 * \brief A widget for editing the gradient splines.
 */
#define noDragMode 0
#define dragNodeMode 1
#define dragSegmentMode 2

class GradientWidget : public QWidget
{
  Q_OBJECT
    public:
  /*!
   * \brief Constructor.
   */
  GradientWidget(QWidget* parent = 0, const char* name = 0,
                 const QString& gradString = "", double vMin = 0,
                 double vMax = 1.);
  //GradientWidget(QWidget* parent=0, const char* name=0);

  /*!
   * \brief Destructor.
   */
  ~GradientWidget();

 public:
  /*!
   * \brief Return the minimum size of the widget.
   */
  QSize minimumSizeHint() const;

  /*!
   * \brief Return the recommended size of the widget.
   */
  QSize sizeHint() const;

  /*!
   * \brief Get the current gradient.
   *
   * \return The gradient object.
   */
  const Gradient& getGradient() const
    {
      return _gradient;
    }

  QString getGradientString() const;
  void setGradient(const QString& s);

  bool hasAlpha() const
  { return _hasAlpha; }
  void setHasAlpha( bool x );
  void setBounds( double vMin = 0, double vMax = 1. );
  double minBound() const { return _minVal; }
  double maxBound() const { return _maxVal; }

 signals:
  void gradientChanged(QString s);

  public slots:
  /*!
   * \brief Set the gradient to edit.
   *
   * \param gradient The gradient object.
   */
  void setGradient(const Gradient& gradient);

  /*!
   * \brief Set a default RGB gradient.
   */
  void newRgb();

  /*!
   * \brief Set a default HSV gradient.
   */
  void newHsv();

  /*!
   * \brief Invert the current gradient.
   */
  void invert();

 private:
  // process events
  void paintEvent(QPaintEvent* e);
  void resizeEvent(QResizeEvent* e);
  void mousePressEvent(QMouseEvent* e);
  void mouseReleaseEvent(QMouseEvent* e);
  void mouseMoveEvent(QMouseEvent* e);

  // layout the area rectangles
  void calcRects();

  // Make mapping between real value and x position:
  int realValToWidgetX(double v);

  // Retrieve colormap data compatible with matplotlib
  QString getRGBnodes();

 private:
  Gradient _gradient;
    
  // area rectangles
  QRect _gradRect;
  QRect _splineRect[4];

  // node dragging
  int _dragMode; // either noDragMode, dragNodeMode or dragSegmentMode
  int _dragSpline;
  int _dragSegLeft;
  int _dragSegRight;
  QPoint _prevMousePos;
  int _dragNode;
  bool _dragRemove;

  // real value mapping
  double _minVal;
  double _maxVal;
  QPoint _pointerPos;
  QString _posText;
  bool _hasAlpha;
};

#endif
