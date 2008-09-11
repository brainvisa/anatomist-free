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


#ifndef ANATOMIST_ERPIO_ERPWRAPER_H
#define ANATOMIST_ERPIO_ERPWRAPER_H


#include <anatomist/observer/Observer.h>
#include <qwidget.h>
#include <string>
#include <set>


namespace anatomist
{
  class ATexture;
  class AObject;
  /// private data structure
  struct ErpWraper_data;
}


/**	Graphic loader with appropriate sliders to load any .erp cell / 
	observation in an ERP directory into a ATexture object */
class ErpWraper : public QWidget, public anatomist::Observer
{
  Q_OBJECT

public:
  ErpWraper( anatomist::ATexture* obj, const std::string & dirname, 
	     QWidget* parent = 0 );
  virtual ~ErpWraper();

  static bool initTexOptions();
  static void openWraper( const std::set<anatomist::AObject *> & obj );

  virtual void update( const anatomist::Observable*, void* arg );

protected slots:
  virtual void cellSliderChanged( int );
  virtual void obsSliderChanged( int );

protected:
  virtual void scanDir();
  virtual void fillEdits();
  virtual void loadErp();
  virtual void unregisterObservable( anatomist::Observable* );

  std::string			_dirname;
  anatomist::ATexture		*_texture;
  anatomist::ErpWraper_data	*_data;

private:
};


#endif
