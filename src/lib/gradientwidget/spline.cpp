/**************************************************************************
 * This file is part of the Fraqtive program.
 * Copyright (C) 2005 Michal Mecinski.
 * This program is licensed under the GNU General Public License.
 **************************************************************************/

#include "spline.h"

#include <qstringlist.h>
#include <iostream>
#include <cmath>
#include <stdlib.h>

Spline::Spline()
{
}

Spline::~Spline()
{
}

int Spline::findNode(QSize gradSize, QPoint ptPos, QSize ptSize) const
{
  for (int i = 0; i < (int)_array.size(); i++) {
    int x = (int)(_array[i]._x * gradSize.width());
    int y = (int)((1.0 - _array[i]._y) * gradSize.height());
    if (::abs(ptPos.x() - x) <= ptSize.width() && ::abs(ptPos.y() - y) <= ptSize.height())
      return i;
  }
  return -1;
}

void Spline::findSegment(QSize gradSize, QPoint ptPos, int& p1, int& p2) const
{
  p1 = -1;
  p2 = -1;
  std::cout << "Finding segment closest to (" << ptPos.x() << "," << ptPos.y() <<") ..." << std::endl;
  double dMin = 10000;
  if ((int)_array.size() == 1)
    p1 = 0;
  
  int xm = ptPos.x();
  int ym = ptPos.y();

  for (int i = 0; i < (int)_array.size(); i++) {
    if (i+1 >= (int)_array.size())
      break;
    std::cout << "Testing segment " << i << "-" << i+1 << std::endl;
    int xa = (int)(_array[i]._x * gradSize.width());
    int ya = (int)((1.0 - _array[i]._y) * gradSize.height());
    int xb = (int)(_array[i+1]._x * gradSize.width());
    int yb = (int)((1.0 - _array[i+1]._y) * gradSize.height());
    
    int babm = (xa-xb)*(xm-xb) + (ya-yb)*(ym-yb);
    int abam = (xb-xa)*(xm-xa) + (yb-ya)*(ym-ya);
    
    double d = 1.0;
    if (babm >= 0 && abam >= 0)
      d = std::sqrt( std::pow((double)((yb-ya)*(xm-xa)+(xb-xa)*(ym-ya)),2)
                     / (std::pow((double)(xb-xa),2) 
                        +std::pow((double)(yb-ya),2)) );
    else if (babm < 0)
      d = std::sqrt( std::pow((double)(xm-xa),2) + std::pow((double)(ym-ya),2) );
    else // abam < 0
      d = std::sqrt( std::pow((double)(xm-xb),2) + std::pow((double)(ym-yb),2) );
    
    //if (ptPos.x() - x < 0.0)
    //  break;
    //double d = std::sqrt(std::pow((double)(ptPos.x() - x), 2)
    //                     +std::pow((double)(ptPos.y() - y), 2));
    std::cout << "d=" << d << std::endl;
    if ( d < dMin)
      {
        std::cout << "Keeping it!" << std::endl;
        p1 = i;
        p2 = i+1<(int)_array.size() ? i+1 : -1;
        dMin = d; 
      }
  }
  std::cout << "Found nodes :" << p1 << "," << p2 << std::endl;
}

int Spline::insertNode(double x, double y)
{
  for (int i = 0; i < (int)_array.size(); i++) {
    if (_array[i]._x > x) {
      _array.insert(_array.begin() + i, Coord(x, y));
      return i;
    }
  }
  _array.push_back(Coord(x, y));
  return (int)_array.size() - 1;
}

void Spline::invert()
{
  int nodes = _array.size();
  for (int i = 0; i < nodes / 2; i++) {
    double tx = _array[i]._x;
    double ty = _array[i]._y;
    _array[i]._x = 1.0 - _array[nodes - i - 1]._x;
    _array[i]._y = _array[nodes - i - 1]._y;
    _array[nodes - i - 1]._x = 1.0 - tx;		
    _array[nodes - i - 1]._y = ty;		
  }
  if (nodes % 2)
    _array[nodes / 2]._x = 1.0 - _array[nodes / 2]._x;
}

void Spline::generateSpline(double* buffer, int length) const
{
  int nodes = _array.size();

  if (nodes == 0) {
    for (int i = 0; i < length; i++)
      buffer[i] = 0.0;
    return;
  }
  if (nodes == 1) {
    for (int i = 0; i < length; i++)
      buffer[i] = _array[0]._y;
    return;
  }

  const int SUBSEGMENTS = 8;
  double h[SUBSEGMENTS+1][4];

  for (int i = 0; i <= SUBSEGMENTS; i++) {
    double s = (double)i / (double)SUBSEGMENTS;
    double s2 = s * s;
    double s3 = s2 * s;
    h[i][0] = 2*s3 - 3*s2 + 1;
    h[i][1] = -2*s3 + 3*s2;
    h[i][2] = s3 - 2*s2 + s;
    h[i][3] = s3 - s2;
  }

  for (int seg = 0; seg < nodes; seg++) {
    double p1x = _array[seg]._x;
    double p1y = _array[seg]._y;

    double p2x, p2y;
    if (seg < nodes - 1) {
      p2x = _array[seg + 1]._x;
      p2y = _array[seg + 1]._y;
    } else {
      p2x = _array[0]._x + 1.0;
      p2y = _array[0]._y;
    }

    double t1x, t1y;
    if (seg > 0) {
      t1x = p1x - _array[seg - 1]._x;
      t1y = p1y - _array[seg - 1]._y;
    } else {
      t1x = p1x - _array[nodes - 1]._x + 1.0;
      t1y = p1x - _array[nodes - 1]._y;
    }

    double t2x, t2y;
    if (seg < nodes - 2) {
      t2x = _array[seg + 2]._x - p2x;
      t2y = _array[seg + 2]._y - p2y;
    } else {
      t2x = _array[seg + 2 - nodes]._x - p2x + 1.0;
      t2y = _array[seg + 2 - nodes]._y - p2y;
    }

    double dx = std::fabs(p2x - p1x);
    double alfa = 0.45 * (dx < 0.15 ? dx / 0.15 : 1.0);

    int lx = -1;
    double ly = 0.0;

    for (int i=0; i <= SUBSEGMENTS; i++) {
      double py = h[i][0]*p1y + h[i][1]*p2y + (h[i][2]*t1y + h[i][3]*t2y) * alfa;
      double px = h[i][0]*p1x + h[i][1]*p2x + (h[i][2]*t1x + h[i][3]*t2x) * alfa;

      if (py < 0.0)
        py = 0.0;
      else if (py > 1.0)
        py = 1.0;

      int x = (int)(px * length + 0.5);
      int cx = x - lx;

      if (i > 0 && cx > 0) {
        double scale = (py - ly) / (double)cx;
        for (int ix = 0; ix < cx; ix++)
          buffer[(ix + lx) % length] = ly + ix * scale;
      }

      lx = x;
      ly = py;
    }
  }
}

QString Spline::toString() const
{
  QStringList list;

  for (int i = 0; i < (int)_array.size(); i++) {
    QString str;
    list.append(str.setNum(_array[i]._x));
    list.append(str.setNum(_array[i]._y));
  }

  return list.join(";");
}

void Spline::fromString(const QString& string)
{
#if QT_VERSION >= 0x040000
  QStringList list = string.split(';');
#else
  QStringList list = QStringList::split(';', string);
#endif

  _array.clear();
  double lx = 0.0;
  for (QStringList::iterator it = list.begin(); it != list.end(); ++it) {
    double x = (*it).toDouble();
    if (++it != list.end()) {
      double y = (*it).toDouble();
      if (x >= lx && x <= 1.0 && y >= 0.0 && y <= 1.0)
        _array.push_back(Coord(x, y));
    }
    else
      break;
    lx = x;
  }
}
