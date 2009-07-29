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


#ifndef ANATOMIST_WINDOW_CONTROLLEDWINDOW_H
#define ANATOMIST_WINDOW_CONTROLLEDWINDOW_H


#include <anatomist/window/qwindow.h>


namespace anatomist
{
  struct ControlledWindow_PrivateData;
}


/**	Anatomist Window using Qt and controler system, providing a toolbar 
	for controls */
class ControlledWindow : public QAWindow
{
  Q_OBJECT

public:
  ControlledWindow( QWidget* parent = 0, const char* name = 0, 
		    carto::Object options = carto::none(), 
                    Qt::WFlags f = Qt::WType_TopLevel );
  virtual ~ControlledWindow();

  virtual void registerObject( anatomist::AObject* object,
                               bool temporaryObject = false,
                               int position = -1 );
  virtual void unregisterObject( anatomist::AObject* object );

  virtual void updateAvailableControls();
  virtual void updateActivableControls();
  virtual void updateActions();
  virtual void updateActiveControl();

  virtual anatomist::View* view() = 0;
  virtual const anatomist::View* view() const = 0;

public slots:
  /** just calls \updateActiveControl(). We cannot use directly 
      \updateActiveControl() as a virtual slot, due to a \moc limitation 
      (documented in Qt HTML documentation) */
  virtual void activeControlChanged();
  virtual void updateControls();

protected:

private:
  anatomist::ControlledWindow_PrivateData	*d;
};


#endif
