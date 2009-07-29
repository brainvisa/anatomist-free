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


#ifndef ANA_SELECTION_QSELECTFACTORY_H
#define ANA_SELECTION_QSELECTFACTORY_H


#include <anatomist/config/anatomist_config.h>
#include <anatomist/selection/selectFactory.h>
#include <qobject.h>
#include <string>

class QSelectMenu;
class Graph;
class Tree;


namespace anatomist
{

  class AGraph;
  class AGraphObject;
}


/**	Selection windows factory, providing Qt windows / widgets
 */
class ANATOMIST_API QSelectFactory 
  : public QObject, public anatomist::SelectFactory
{
  Q_OBJECT
public:
  QSelectFactory();
  virtual ~QSelectFactory();

  virtual anatomist::WSelectChooser* 
  createSelectChooser( unsigned group, 
		       const std::set<anatomist::AObject *> & objects ) const;
  virtual void handleSelectionMenu( anatomist::AWindow* win, int x, int y, 
				    const Tree* specific = 0 );
  virtual void selectNeighbours( anatomist::AGraph* graph, 
				 anatomist::AGraphObject* go, 
				 std::set<anatomist::AObject *> 
				 & toselect ) const;
  virtual void selectNodesWith( std::set<anatomist::AObject *> & tosel, 
				const Graph* gr, const std::string & attr, 
				const std::string & val );

public slots:
  virtual void view( anatomist::AWindow* win );
  virtual void unselect( anatomist::AWindow* win );
  virtual void selectAll( anatomist::AWindow* win );
  virtual void remove( anatomist::AWindow* win );
  virtual void removeFromThisWindow( anatomist::AWindow* win );
  virtual void neighbours( anatomist::AWindow* win );
  virtual void selAttrib( anatomist::AWindow* win );

protected:
  QSelectMenu	*_selMenu;

private:
};


#endif
