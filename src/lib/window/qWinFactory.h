/* Copyright (c) 1995-2005 CEA
 *
 *  This software and supporting documentation were developed by
 *      CEA/DSV/SHFJ
 *      4 place du General Leclerc
 *      91401 Orsay cedex
 *      France
 *
 * This software is governed by the CeCILL license version 2 under 
 * French law and abiding by the rules of distribution of free software.
 * You can  use, modify and/or redistribute the software under the 
 * terms of the CeCILL license version 2 as circulated by CEA, CNRS
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
 * knowledge of the CeCILL license version 2 and that you accept its terms.
 */


#ifndef ANATOMIST_WINDOW_QWINFACTORY_H
#define ANATOMIST_WINDOW_QWINFACTORY_H

#include <anatomist/window/winFactory.h>
#include <qpixmap.h>
#include <qstring.h>
#include <qobject.h>


/**	Specialized AWindowFactory storing Qt specific information 
	about windows (QPixmaps, QStrings, ...)
 */
class QAWindowFactory : public QObject, public anatomist::AWindowFactory
{
  Q_OBJECT

public:
  struct PixList
  {
    PixList() {}
    PixList( const QPixmap & s, const QPixmap & l, const QPixmap & a ) 
      : psmall( s ), plarge( l ), pactive( a ) {}
    ///	Small pixmap (windows list in control window)
    QPixmap	psmall;
    ///	Large pixmap (control window toolbar)
    QPixmap	plarge;
    ///	Active large pixmap (control window toolbar)
    QPixmap	pactive;
  };

  struct Descrip
  {
    Descrip() {}
    Descrip( const QString & b, const QString & l ) 
      : brief( b ), longer( l ) {}
    ///	Brief description - short, in a few words
    QString	brief;
    ///	Longer description - a few lines max
    QString	longer;
  };

  static const std::map<int, PixList> & pixmaps() { return( _pixmaps ); }
  static const std::map<int, Descrip> & description() 
  { return( _description ); }
  static const PixList & pixmaps( int type ) { return( _pixmaps[ type ] ); }
  static const PixList & pixmaps( const std::string & type ) 
  { return( _pixmaps[ typeID( type ) ] ); }
  static const Descrip & description( int type ) 
  { return( _description[ type ] ); }
  static const Descrip & description( const std::string & type )
  { return( _description[ typeID( type ) ] ); }
  static void setPixmaps( int type, const PixList & pix ) 
  { _pixmaps[ type ] = pix; }
  static void setPixmaps( const std::string & type, const PixList & pix ) 
  { _pixmaps[ typeID( type ) ] = pix; }
  static void loadDefaultPixmaps( int type );
  static void loadDefaultPixmaps( const std::string & type )
  { loadDefaultPixmaps( typeID( type ) ); }
  static void setDescription( int type, const Descrip & des )
  { _description[ type ] = des; }
  static void setDescription( const std::string & type, const Descrip & des )
  { _description[ typeID( type ) ] = des; }

protected:
  static std::map<int, PixList>	_pixmaps;
  static std::map<int, Descrip>	_description;
};



#endif
