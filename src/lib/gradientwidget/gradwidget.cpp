/**************************************************************************
 * This file is part of the Fraqtive program.
 * Copyright (C) 2005 Michal Mecinski.
 * This program is licensed under the GNU General Public License.
 **************************************************************************/

#include "gradwidget.h"
#include <qpainter.h>
#include <qdrawutil.h>
#include <qpixmap.h>
#include <qimage.h>
#include <iostream>
#include <cmath>
#if QT_VERSION >= 0x040000
#include <QMouseEvent>
#endif

GradientWidget::GradientWidget(QWidget* parent, const char* name, 
                               const QString& gradString, double vMin,
                               double vMax) :
#if QT_VERSION >= 0x040000
  QWidget( parent ),
#else
  QWidget(parent, name, Qt::WNoAutoErase),
#endif
  _hasAlpha( false )
{
  _pointerPos = QPoint();
  _posText = QString("");
  _minVal = vMin;
  if( vMin == vMax )
    vMax = vMin + 1;
  _maxVal = vMax;
  _dragMode = noDragMode;
  _dragSegLeft = -1;
  _dragSegRight = -1;
  _prevMousePos = QPoint();
  newRgb();
  calcRects();
  update();
#if QT_VERSION >= 0x040000
  QStringList l = gradString.split('#');
#else
  QStringList l = QStringList::split('#', gradString);
#endif
  int i = 0;
  for (QStringList::iterator it = l.begin(); it != l.end() && i < 4; ++it,i++)
  {
    _gradient.getSpline(i).fromString(*it);
  }
}

GradientWidget::~GradientWidget()
{
}

QSize GradientWidget::minimumSizeHint() const
{
  return QSize(200, 200);
}

QSize GradientWidget::sizeHint() const
{
  return QSize(400, 300);
}

void GradientWidget::setGradient(const Gradient& gradient)
{
  _gradient = gradient;
  calcRects();
  update();
}

void GradientWidget::newRgb()
{
  _gradient = Gradient(false);

  for (int i=0; i<3; i++) {
    _gradient.getSpline(i).addNode(0.0, 0.0);
    _gradient.getSpline(i).addNode(1.0, 1.0);
  }
  _gradient.getSpline(3).addNode(0.5, 1.0);

  calcRects();
  update();
}

void GradientWidget::newHsv()
{
  _gradient = Gradient(true);

  _gradient.getSpline(0).addNode(0.0, 0.0);
  _gradient.getSpline(0).addNode(1.0, 1.0);
  _gradient.getSpline(1).addNode(0.5, 1.0);
  _gradient.getSpline(2).addNode(0.5, 0.5);
  _gradient.getSpline(3).addNode(0.5, 1.0);

  calcRects();
  update();
}

void GradientWidget::invert()
{
  _gradient.invert();
  update();
}

int GradientWidget::realValToWidgetX(double v)
{
  if( _maxVal == _minVal )
    return _gradRect.right();
  double x = (v-_maxVal) * 
    (_gradRect.right()-_gradRect.left())/(_maxVal-_minVal)
    + _gradRect.right() ;
  return int(floor(x+0.5)) ;
}

QString GradientWidget::getRGBnodes()
{
  int tmpLength = 100;
  double* temp = new double[tmpLength];
  double* data;
  QStringList sl;
  int ncomps = 3;
  if( _hasAlpha )
    ncomps = 4;
  for (int s = 0; s < ncomps; s++){
    QStringList slTmp;
    const Spline& spline = _gradient.getSpline(s);
    int nbs = spline.getNodesCnt();
    if (nbs == 1){
      sl.append(QString("%1;%2;%3;%4")
                .arg(0.)
                .arg(spline.getNodeY(0))
                .arg(1.)
                .arg(spline.getNodeY(0)));
      continue;
    }
    int nbNodes = nbs;
    double startNode = -1.0;
    double endNode = -1.0;
    bool splineGenerated = false;
    if (spline.getNodeX(0) != 0.0)
      {
        nbNodes++;
        spline.generateSpline(temp, tmpLength);
        splineGenerated = true;
        startNode = temp[0];
      }
    if (spline.getNodeX(nbs-1) != 1.0)
      {
        if (!splineGenerated)
          spline.generateSpline(temp, tmpLength);
        endNode = temp[tmpLength-1];
        nbNodes++;
      }
    data = new double[nbNodes*2];
    int n = 0;
    if (startNode >= 0.0)
      {
        data[0] = 0.0;
        data[1] = startNode;
        n = 2;
      }
    for (int i=0; i<nbs ; i++)
      {
        data[n++] = spline.getNodeX(i);
        data[n++] = spline.getNodeY(i);
      }
    if (endNode >= 0.0)
      {
        data[n++] = 1.0;
        data[n] = endNode;
      }

    // Display all nodes :
    for (int i=0 ; i < (nbNodes*2)-1 ; i++)
      {
        slTmp.append( QString("%1").arg(data[i]) );
      }
    slTmp.append( QString("%1").arg(data[(nbNodes*2)-1]) );

    sl.append(slTmp.join(";"));
  } 
  
  delete[] temp;
  delete[] data;
  return sl.join("#");
}

void GradientWidget::paintEvent(QPaintEvent*)
{
  QPixmap pm(size());
  QPainter p;
#if QT_VERSION >= 0x040000
  p.begin(&pm);
#else
  p.begin(&pm, this);
#endif

  p.eraseRect(rect());
  
  /*
  for (int i = 1; i <= 3; i++) {
    int lx = _gradRect.left() + _gradRect.width() * i / 4;
    qDrawShadeLine(&p, lx, 0, lx, height(), colorGroup(), true, 1, 1);
  }
  */
#if QT_VERSION >= 0x040000
  qDrawShadePanel(&p, _gradRect.left() - 1, _gradRect.top() - 1,
                  _gradRect.width() + 2, _gradRect.height() + 2,
                  palette(), true, 1, NULL);
#else
  qDrawShadePanel(&p, _gradRect.left() - 1, _gradRect.top() - 1,
                  _gradRect.width() + 2, _gradRect.height() + 2,
                  colorGroup(), true, 1, NULL);
#endif

  /*
  for (int i = 0; i < 3; i++) {
    qDrawShadePanel(&p, _splineRect[i].left() - 1, _splineRect[i].top() - 1,
                    _splineRect[i].width() + 2, _splineRect[i].height() + 2,
                    colorGroup(), true, 1, NULL);
  }
  */
  int ncomps = hasAlpha() ? 4 : 3;
  int length = _gradRect.width();
  QRgb* buffer = new QRgb[length];
  double* data = new double[length*ncomps];

  _gradient.fillGradient(buffer, length, data, hasAlpha());

  int x1 = _gradRect.left();
  int y1 = _gradRect.top();
  int y2 = _gradRect.bottom();

  // Display actual gradient:
  for (int x = 0; x < length; x++) {
    //std::cout << "buffer["<< x<< "] = " << buffer[x] << std::endl;
    p.setPen(buffer[x]);
    p.drawLine(x1 + x, y1, x1 + x, y2);
  }

  QColor txtBgColor = Qt::white;
  //txtBgColor.setAlpha(10); supported only in Qt 4.0

  // Display tick marks:
  double b = floor(log10(std::max(std::abs(_maxVal), std::abs(_minVal))));
  double aMin = _minVal/(double) pow(10,b);

  // m = a*10**b
  int minX = x1;
  int maxX = _gradRect.right();
  int yTop = y1;
  int yBottom = y2;

  int intTickHeight = 7;
  int intTickWidth = 2;
  QPen intTickPen(Qt::black, intTickWidth);
  int floatTickHeight = 3;
  int floatTickWidth = 1;
  QPen floatTickPen(Qt::black, floatTickWidth);

  double dn = 0.5;
  double n = floor(aMin);
  
  QRect br;
  if( !std::isinf( b ) && !std::isnan( b ) )
    while(realValToWidgetX(n*pow(10,b)) < maxX)
    {
      if( n > 11 )
        break;
      int x = realValToWidgetX(n*pow(10,b));
      if (x < 0.0) 
        {
          n += dn;
          continue;
        }
      //std::cout << n*pow(10,b) << " mapped to " << x << std::endl;
      // Draw the tick mark:
      int th = 0;
      if (floor(n) == n)
        {
          p.setPen(intTickPen);
          th = intTickHeight;
        }
      else
        {
          p.setPen(floatTickPen);
          th = floatTickHeight;
        }
      p.drawLine(x, yTop, x, yTop-th);
      p.setBackgroundMode(Qt::OpaqueMode);
#if QT_VERSION >= 0x040000
      p.setBackground(txtBgColor);
#else
      p.setBackgroundColor(txtBgColor);
#endif

      br = p.boundingRect(QRect(x-20, yTop, 40, 40),
                         Qt::AlignHCenter | Qt::AlignTop,
                         QString("%1").arg(n, 0, 'f', 1));
      //std::cout << "br : " << br.left() << " - " << br.right() << std::endl;
      if (br.left() >= 0 and br.right() <= maxX)
        p.drawText(QRect(x-20, yTop, 40, 40),
                   Qt::AlignHCenter | Qt::AlignTop,
                   QString("%1").arg(n, 0, 'f', 1));
      else if (br.left() < 0)
        p.drawText(QRect(0, yTop, 40, 40),
                   Qt::AlignLeft | Qt::AlignTop,
                   QString("%1").arg(n, 0, 'f', 1));
      else
        p.drawText(QRect(maxX-40, yTop, 40, 40),
                   Qt::AlignRight | Qt::AlignTop,
                   QString("%1").arg(n, 0, 'f', 1));
      
      n += dn;
    }
  if (b != 0.0)
    {
      //std::cout << "yBottom" << yBottom << std::endl; 
      p.drawText(minX, yBottom, QString("x1e%1").arg(b));
    }

  if (!_gradient.isHsv()) {
    y1 = _splineRect[0].top();
    y2 = _splineRect[0].bottom();
    for (int x = 0; x < length; x++) {
      p.setPen(qRgb(qRed(buffer[x]), 0, 0));
      p.drawLine(x1 + x, y1, x1 + x, y2);
    }
    y1 = _splineRect[1].top();
    y2 = _splineRect[1].bottom();
    for (int x = 0; x < length; x++) {
      p.setPen(qRgb(0, qGreen(buffer[x]), 0));
      p.drawLine(x1 + x, y1, x1 + x, y2);
    }
    y1 = _splineRect[2].top();
    y2 = _splineRect[2].bottom();
    for (int x = 0; x < length; x++) {
      p.setPen(qRgb(0, 0, qBlue(buffer[x])));
      p.drawLine(x1 + x, y1, x1 + x, y2);
    }
  } else {
#if QT_VERSION >= 0x040000
    QImage image(_splineRect[0].size(), QImage::Format_RGB32 );
#else
    QImage image(_splineRect[0].size(), 32);
#endif
    y1 = _splineRect[0].top();
    y2 = _splineRect[0].bottom();
    int h = _splineRect[0].height();
    for (int x = 0; x < length; x++) {
      QRgb color = Gradient::hsvToRgb(data[x], 1.0, 0.5);
      for (int y = 0; y < h; y++)
        ((QRgb*)image.scanLine(y))[x] = color;
    }
    p.drawImage(x1, y1, image);

#if QT_VERSION >= 0x040000
    image = QImage( _splineRect[1].size(), QImage::Format_RGB32 );
#else
    image.create(_splineRect[1].size(), 32);
#endif
    y1 = _splineRect[1].top();
    h = _splineRect[1].height() - 1;
    for (int x = 0; x < length; x++) {
      int y;
      for (y = 0; y < h; y += 2) {
        QRgb color = Gradient::hsvToRgb(data[x], 1.0 - (double)y / (double)h, 0.5);
        ((QRgb*)image.scanLine(y))[x] = color;
        ((QRgb*)image.scanLine(y + 1))[x] = color;
      }
      if (y == h) {
        QRgb color = Gradient::hsvToRgb(data[x], 1.0 - (double)y / (double)h, 0.5);
        ((QRgb*)image.scanLine(y))[x] = color;
      }
    }
    p.drawImage(x1, y1, image);

#if QT_VERSION >= 0x040000
    image = QImage( _splineRect[2].size(), QImage::Format_RGB32 );
#else
    image.create(_splineRect[2].size(), 32);
#endif
    y1 = _splineRect[2].top();
    h = _splineRect[2].height();
    for (int x = 0; x < length; x++) {
      for (int y = 0; y < h; y++) {
        QRgb color = Gradient::hsvToRgb(data[x], data[x + length], 1.0 - (double)y / (double)h);
        ((QRgb*)image.scanLine(y))[x] = color;
      }
    }
    p.drawImage(x1, y1, image);
  }
  if( hasAlpha() )
  {
    y1 = _splineRect[3].top();
    y2 = _splineRect[3].bottom();
    for (int x = 0; x < length; x++) {
      p.setPen(qRgb(qAlpha(buffer[x]), qAlpha(buffer[x]), qAlpha(buffer[x])));
      p.drawLine(x1 + x, y1, x1 + x, y2);
    }
  }

  QPen dottedPen(Qt::white, 0, Qt::DotLine);
  p.setPen(dottedPen);

  //p.setBrush(QColor(255,255,255,0));
  if ( (_posText != "") && (not _pointerPos.isNull()))
    {
      //std::cout << "painting pos..." << std::endl;
      y1 = _gradRect.top();
      y2 = _gradRect.bottom();

      // Display xPosition vertical indicator on top gradient:
#if QT_VERSION >= 0x040000
      p.setBackground(Qt::black);
#else
      p.setBackgroundColor(Qt::black);
#endif
      p.drawLine(_pointerPos.x(), y1, _pointerPos.x(), y2);

      p.setPen(Qt::SolidLine);
#if QT_VERSION >= 0x040000
      p.setBackground(Qt::white);
#else
      p.setBackgroundColor(Qt::white);
#endif
      p.setBackgroundMode(Qt::OpaqueMode);

      br = p.boundingRect(_pointerPos.x(),(int) y1+38, 40, 40,
                          Qt::AlignLeft, _posText);
      if (br.right() < maxX)
        p.drawText(_pointerPos.x(),(int) y1+38, _posText);
      else
        p.drawText(_pointerPos.x()-br.width(),(int) y1+38, _posText);
    }

  p.setPen(dottedPen);
  p.setBackgroundMode(Qt::OpaqueMode);
#if QT_VERSION >= 0x040000
      p.setBackground(Qt::black);
#else
  p.setBackgroundColor(Qt::black);
#endif

#if QT_VERSION >= 0x040000
    QPolygon arr( length );
#else
    QPointArray arr(length);
#endif
for (int i = 0; i < ncomps; i++) {
    y1 = _splineRect[i].top();
    y2 = _splineRect[i].bottom();
    int h = _splineRect[i].height() - 1;
    for (int x = 0; x < length; x++) {
      arr.setPoint(x, x1+x, y1 + (int)((1.0 - data[i*length + x]) * h));
    }
    p.drawPolyline(arr);
    //p.drawLine(_xPos, y1, _xPos, y2);
  }

  p.setPen(Qt::black);
  p.setBrush(Qt::white);

  for (int i = 0; i < ncomps; i++) {
    y1 = _splineRect[i].top();
    int h = _splineRect[i].height() - 1;
    const Spline& spline = _gradient.getSpline(i);
    for (int node = 0; node < spline.getNodesCnt(); node++) {
      int x = (int)(spline.getNodeX(node) * (length-1));
      int y = (int)((1.0 - spline.getNodeY(node)) * h);
      p.drawRect(x1 + x - 4, y1 + y - 4, 9, 9);
    }
  }

  delete[] buffer;
  delete[] data;

  // display position mapping at pointer position:

  p.end();
#if QT_VERSION >= 0x040000
  QPainter pq( this );
  pq.drawPixmap( 0, 0, pm );
#else
  bitBlt(this, 0, 0, &pm);
#endif
}

void GradientWidget::resizeEvent(QResizeEvent* e)
{
  QWidget::resizeEvent(e);
  calcRects();
}

void GradientWidget::mousePressEvent(QMouseEvent* e)
{
  _prevMousePos = e->pos();
  for (int i = 0; i < 4; i++) {
    QRect rectCheck(_splineRect[i].left() - 5, _splineRect[i].top() - 5,
                    _splineRect[i].width() + 10, _splineRect[i].height() + 10);
    if (rectCheck.contains(e->pos())) {
      _dragSpline = i;
      break;
    }
  }

  if (_dragSpline < 0)
    return;

  Spline& spline = _gradient.getSpline(_dragSpline);

  QPoint pt(e->x() - _splineRect[_dragSpline].left(), 
            e->y() - _splineRect[_dragSpline].top());
  QSize size(_splineRect[_dragSpline].size());

  if (e->button() == Qt::LeftButton)
    {
      _dragMode = dragNodeMode;
      _dragNode = spline.findNode(size, pt, QSize(5, 5));

      if (_dragNode < 0 && _splineRect[_dragSpline].contains(e->pos())) {
        _dragNode = spline.insertNode((double)pt.x() / (double)size.width(),
                                      1.0 - (double)pt.y() / (double)size.height());
      }

      if (_dragNode < 0)
        _dragSpline = -1;
      else {
        _dragRemove = false;
        mouseMoveEvent(e);
      }
    }
  else if (e->button() == Qt::RightButton)
    {
      _dragMode = dragSegmentMode;
      
      spline.findSegment(size, pt, _dragSegLeft, _dragSegRight);
    }
}

void GradientWidget::mouseReleaseEvent(QMouseEvent*)
{
  _dragSpline = -1;
  _posText = "";
  _dragMode = noDragMode;
  _dragSegLeft = -1;
  _dragSegRight = -1;
  getRGBnodes();
  update();
  // Will send gradientChanged signal ...
  emit gradientChanged(getRGBnodes());

}

QString GradientWidget::getGradientString() const
{
  QStringList sl;
  int ncomps = hasAlpha() ? 4 : 3;
  for (int i=0; i<ncomps; i++)
    sl.append( _gradient.getSpline(i).toString());

  return sl.join("#");

}

void GradientWidget::setGradient(const QString& s)
{
#if QT_VERSION >= 0x040000
  QStringList list = s.split('#');
#else
  QStringList list = QStringList::split('#', s);
#endif
  int i=0;
  for (QStringList::iterator it = list.begin(); it != list.end() && i < 4;
       ++it,i++) {
    _gradient.getSpline(i).fromString((*it));
  }

}

void GradientWidget::mouseMoveEvent(QMouseEvent* e)
{
  /*
    if ( (e->x() >= _gradRect.left()) && e->x() <= _gradRect.right() )
    {
    _xPos = e->x();
    update();
    }
  */
  if (_dragSpline < 0)
    return;

  
  QRect rectCheck(_splineRect[_dragSpline].left() - 40, 
                  _splineRect[_dragSpline].top() - 40,
                  _splineRect[_dragSpline].width() + 80,
                  _splineRect[_dragSpline].height() + 80);

  if (_dragMode == dragNodeMode)
    {
      if (!_dragRemove) {
        if (_gradient.getSpline(_dragSpline).getNodesCnt() > 1 
            && !rectCheck.contains(e->pos())) {
          _dragRemove = true;
          _gradient.getSpline(_dragSpline).removeNode(_dragNode);
        }
      } else {
        if (rectCheck.contains(e->pos())) {
          _dragRemove = false;
          _gradient.getSpline(_dragSpline).insertAt(_dragNode);
        } else
          return;
      }


      if (!_dragRemove) {
        //std::cout << "!_dragRemove" << std::endl;
        double x = (double)(e->x() - _splineRect[_dragSpline].left()) / (double)_splineRect[_dragSpline].width();
        double y = 1.0 - (double)(e->y() - _splineRect[_dragSpline].top()) / (double)_splineRect[_dragSpline].height();

        double minX = _gradient.getSpline(_dragSpline).getLimitMin(_dragNode);
        double maxX = _gradient.getSpline(_dragSpline).getLimitMax(_dragNode);
        
        if (x < minX)
          x = minX;
        else if (x > maxX)
          x = maxX;
        
        if (y < 0.0)
          y = 0.0;
        else if (y > 1.0)
          y = 1.0;

        _gradient.getSpline(_dragSpline).setNode(_dragNode, x, y);
        _pointerPos.setX((int) (x*_splineRect[_dragSpline].width() + 
                                _splineRect[_dragSpline].left()));
        _pointerPos.setY((int) ( (1.0-y)*_splineRect[_dragSpline].height()+
                                 _splineRect[_dragSpline].top() ) );
        double a = (x*(_maxVal-_minVal)+_minVal) 
          / pow(10, floor(log10(std::max(std::abs(_maxVal),
                                          std::abs(_minVal)))));
        _posText.sprintf("%1.3f", a);
      }

  }
  else if (_dragMode == dragSegmentMode)
    {
      double dx = e->x() - _prevMousePos.x();
      double dy = e->y() - _prevMousePos.y();
      
      dx = dx / (double)_splineRect[_dragSpline].width();
      dy = -dy / (double)_splineRect[_dragSpline].height();

      _prevMousePos = e->pos();

      double minX1 = _gradient.getSpline(_dragSpline).getLimitMin(_dragSegLeft);
      double maxX1 = _gradient.getSpline(_dragSpline).getLimitMax(_dragSegLeft);
      double xl = _gradient.getSpline(_dragSpline).getNodeX(_dragSegLeft);

      double minX2 = _gradient.getSpline(_dragSpline).getLimitMin(_dragSegRight);
      double maxX2 = _gradient.getSpline(_dragSpline).getLimitMax(_dragSegRight);
      double xr = _gradient.getSpline(_dragSpline).getNodeX(_dragSegRight);
      
      double dl=0, dr=0;
      
      if (maxX1-xl > 1.0)
        dl = xl+dx<minX1 ?  minX1-xl : xl+dx>maxX1 ? maxX1-xl : dx;
      else
        dl = xl+dx<minX1 ?  minX1-xl : dx;
      
      if (minX2-xr > 1.0)
        dr = xr+dx<minX2 ?  minX2-xr : xr+dx>maxX2? maxX2-xr : dx;
      else
        dr = xr+dx>maxX2? maxX2-xr : dx;

      double fdx = std::abs(dl)<std::abs(dr) ? dl : dr;

      double minY = 0.0;
      double maxY = 1.0;

      double yl = _gradient.getSpline(_dragSpline).getNodeY(_dragSegLeft);
      double yr = _gradient.getSpline(_dragSpline).getNodeY(_dragSegRight);
      
      dl = yl+dy<minY ?  minY-yl : yl+dy>maxY ? maxY-yl : dy;
      dr = yr+dy<minY ?  minY-yr : yr+dy>maxY ? maxY-yr : dy;
      double fdy = std::abs(dr)<std::abs(dl) ? dr : dl;

      _gradient.getSpline(_dragSpline).setNode(_dragSegLeft, xl+fdx, yl+fdy);
      _gradient.getSpline(_dragSpline).setNode(_dragSegRight, xr+fdx, yr+fdy);

      if (xr == xl)
        {
          _pointerPos.setX((int) (xr*_splineRect[_dragSpline].width() + 
                                  _splineRect[_dragSpline].left()));
          double a = (xr*(_maxVal-_minVal)+_minVal) 
            / pow(10, floor(log10(std::max(std::abs(_maxVal),
                                            std::abs(_minVal)))));
          _posText.sprintf("%1.3f", a);
        }
    }

  update();
}

void GradientWidget::calcRects()
{
  QRect baseRect(5, 5, width() - 10, height() - 10);
  
  _gradRect = baseRect;
  _gradRect.setHeight(70);

  for (int i=0; i<4; i++)
    _splineRect[i] = baseRect;

  int ncomps = hasAlpha() ? 4 : 3;
  if (!_gradient.isHsv()) { // RGB
    int h = (baseRect.height() - 70) / ncomps - 10;
    for (int i=0; i<ncomps; i++) {
      _splineRect[i].setTop(_gradRect.bottom() + i * (h + 10) + 10);
      _splineRect[i].setHeight(h);
    }
  } else { 
    int h = (baseRect.height() - 70 - 10) / (ncomps+1) - 10;
    _splineRect[0].setTop(_gradRect.bottom() + 10);
    _splineRect[0].setHeight(2*h);
    _splineRect[1].setTop(_splineRect[0].bottom() + 10);
    _splineRect[1].setHeight(h);
    _splineRect[2].setTop(_splineRect[1].bottom() + 10);
    _splineRect[2].setHeight(h);
    _splineRect[3].setTop(_splineRect[2].bottom() + 10);
    _splineRect[3].setHeight(h);
  }
}

void GradientWidget::setHasAlpha( bool x )
{
  _hasAlpha = x;
  calcRects();
  update();
}

void GradientWidget::setBounds( double vmin, double vmax )
{
  if( vmin != _minVal || vmax != _maxVal )
  {
    _minVal = vmin;
    if( vmin == vmax )
      _maxVal = vmin + 1;
    else
      _maxVal = vmax;
    update();
  }
}

