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


#ifndef ANAQT_SELECTION_QSELMENU_H
#define ANAQT_SELECTION_QSELMENU_H

#include <anatomist/object/objectmenu.h>
#include <QMenu>

class Tree;
class QPixmap;

namespace anatomist
{
  class AWindow;
  class AObject;
}


///	Private class
class MenuCallback : public QObject
{
private:
  Q_OBJECT

public:
  MenuCallback( void (*func)( void * ), void *clientdata )
    : _func( func ), _clientdata( clientdata )
  {}
  virtual ~MenuCallback();

public slots:
  virtual void activated();

protected:
  void                        (*_func)( void * );
  void                        *_clientdata;
};


//	private class

class QAOptionMenuCallback : public MenuCallback
{
  Q_OBJECT

public:
  QAOptionMenuCallback( void (*func)( const std::set<anatomist::AObject *> & ),
                        const std::set<anatomist::AObject *> *obj );
  QAOptionMenuCallback( carto::rc_ptr<anatomist::ObjectMenuCallback> cbk,
                        const std::set<anatomist::AObject *> *obj )
    : MenuCallback( 0, (void *) obj ), _callback( cbk ) {}
  virtual ~QAOptionMenuCallback();

public slots:
  virtual void activated();

private:
  carto::rc_ptr<anatomist::ObjectMenuCallback>  _callback;
//   void	(*_optfunc)( const std::set<anatomist::AObject *> & );
};


///	Selection menu widget
class QSelectMenu : public QMenu
{
  Q_OBJECT

public:
  QSelectMenu( QWidget* parent = 0 );
  QSelectMenu( const QString & title, QWidget* parent = 0 );
  virtual ~QSelectMenu();

  virtual void update( anatomist::AWindow* win, const Tree* specific = 0 );
  void addOptionMenus( QMenu* menu, const Tree* tr );
  void addMenus( QMenu* menu, const Tree* tr );
  void setObjects( const std::set<anatomist::AObject *> & obj )
  { _objects = obj; }

signals:
  void viewSignal( anatomist::AWindow* win );
  void unselectSignal( anatomist::AWindow* win );
  void selectAllSignal( anatomist::AWindow* win );
  void removeSignal( anatomist::AWindow* win );
  void removeThisWinSignal( anatomist::AWindow* win );
  void neighboursSignal( anatomist::AWindow* win );
  void selAttribSignal( anatomist::AWindow* win );

public slots:
  void viewSlot();
  void unselectSlot();
  void selectAllSlot();
  void removeSlot();
  void removeThisWinSlot();
  void neighboursSlot();
  void selAttribSlot();

protected:
  void eraseCallbacks();

  anatomist::AWindow			*_win;
  std::set<MenuCallback *>		_callbacks;
  static QPixmap			*_defPixmap;
  std::set<anatomist::AObject *>	_objects;

private:
};


#endif
