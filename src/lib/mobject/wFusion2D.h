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


#ifndef ANA_MOBJECT_WFUSION2D_H
#define ANA_MOBJECT_WFUSION2D_H

#include <anatomist/observer/Observer.h>
#include <anatomist/mobject/Fusion2D.h>
#include <qwidget.h>
#include <set>


/**	Fusion2D control
 */
class Fusion2DWindow : public QWidget, public anatomist::Observer
{
  Q_OBJECT

public:
  Fusion2DWindow( const std::set<anatomist::AObject *> &, QWidget* parent = 0, 
		  const char *name = 0, Qt::WFlags f = Qt::WDestructiveClose );
  virtual ~Fusion2DWindow();

  const std::set<anatomist::AObject *> & Objects() const { return( _obj ); }
  virtual void update( const anatomist::Observable* observable, void* arg );
  virtual void unregisterObservable( anatomist::Observable* );

protected:
  std::set<anatomist::AObject *>	_obj;

  ///	Updates fusions and redraw them
  void updateObjects();
  void drawContents();
  void updateInterface();
  anatomist::Fusion2D *currentObject() const;
  anatomist::AObject *currentSubOIbject() const;
  void selectSubObject( int x );

protected slots:
  void moveUpSubObject();
  void moveDownSubObject();
  void subObjectSelected();
  void chooseObject();
  void objectsChosen( const std::set<anatomist::AObject *> & );

private:
  struct Private;
  Private	*pdat;
};


#endif
