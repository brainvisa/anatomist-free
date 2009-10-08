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


#ifndef ANATOMIST_INTERFACE_QWSURFMATCH_H
#define ANATOMIST_INTERFACE_QWSURFMATCH_H


#include <anatomist/observer/Observer.h>
#include <cartobase/object/attributed.h>
#include <cartobase/object/syntax.h>
#include <qlineedit.h>


namespace anatomist
{
  class ASurfMatcher;
}


///	Surface matcher control window
class QSurfMatchWin : public QWidget, public anatomist::Observer
{
  Q_OBJECT

public:
  QSurfMatchWin( QWidget* parent, anatomist::ASurfMatcher* obj );
  virtual ~QSurfMatchWin();

  virtual void update( const anatomist::Observable* obs, void* arg );

signals:
  void changeParam( const std::string & attrib, const QString & val );

protected slots:
  void invertDirection();
  void startProcess();
  void stopProcess();
  void resetProcess();
  void processStepFinished();
  void record( bool );
  void paramChanged( const std::string & attrib, const QString & val );
  void addPointByClickOrg();
  void addPointByClickDst();
  void addPointByNumOrg();
  void addPointByNumDst();
  void deletePointOrg();
  void deletePointDst();

protected:
  class ASThread;
  struct Widgets;
  friend class ASThread;

  virtual void unregisterObservable( anatomist::Observable* );
  void fillDirectionLabel();
  QWidget* paramWidget( QWidget* parent );
  QWidget* ctrlPointsWidget( QWidget* parent );
  void fillCtrlPoints();
  void processThread();
  void notifyUpdate();
  QString strAttribute( const std::string & attr, 
			const carto::AttributedObject & ao, 
			const carto::SyntaxSet & synt );

  anatomist::ASurfMatcher	*_obj;
  Widgets			*_widgets;

private:
};


class QSurfMatchWin_AttEdit : public QLineEdit
{
  Q_OBJECT

public:
  QSurfMatchWin_AttEdit( const std::string & attr, const QString & txt, 
			 QWidget* parent );
  virtual ~QSurfMatchWin_AttEdit();

signals:
  void textChanged( const std::string &, const QString & );

protected slots:
  void textChangedSlot();

protected:
  virtual void leaveEvent( QEvent* ev );
  virtual void focusOutEvent( QFocusEvent* ev );

  std::string	_attrib;
};


#endif
