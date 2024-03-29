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


#ifndef ANA_REFERENCE_WREFERENTIAL_H
#define ANA_REFERENCE_WREFERENTIAL_H


#include <qlabel.h>
#include <string>
#include <vector>


namespace anatomist
{

  class Transformation;
  class Referential;
  struct ReferentialWindow_PrivateData;
}

/**	Window displaying referentials and transformations between them.
	This interface allows to load new transformations
*/
class ReferentialWindow : public QWidget
{
  Q_OBJECT

public:
  ReferentialWindow( QWidget* parent = 0, const char* name = 0, 
		     Qt::WindowFlags f = Qt::WindowFlags() );
  virtual ~ReferentialWindow();

  /// updates contents (new referentials or transformations...)
  void refresh( bool partial = false );

  anatomist::Referential* refAt( const QPoint & pos, QPoint & newpos );
  anatomist::Transformation* transfAt( const QPoint & pos );
  std::vector<anatomist::Transformation*> transformsAt( const QPoint & pos );

  void popupRefMenu( const QPoint & pos, anatomist::Referential* ref );
  void popupTransfMenu(
    const QPoint & pos,
    const std::vector<anatomist::Transformation *> & trans );
  void addTransformationGui( anatomist::Referential* source,
                             anatomist::Referential* dest,
                             bool identity = false, bool merge = false );

  static QString referentialToolTipText(
    anatomist::Referential *ref, std::list<std::string> & temp_filenames );
  static QString transformationToolTipText(
    anatomist::Transformation *tr, std::list<std::string> & temp_filenames );
  static void unlinkFiles( const std::list<std::string> & temp_filenames );

protected:
  ///	opens the file selection dialog to choose a transformation
  void openSelectBox();
  ///	loads a new transformation, should be in a separate IO class...
  void loadTransformation( const std::string & filename );
  void saveTransformation( const std::string & filename );
  void loadTransformationGraph( const std::string & filename );
  void saveTransformationGraph( const std::string & filename );
  virtual void closeEvent ( QCloseEvent * );
  virtual void resizeEvent( QResizeEvent * );
  virtual void mousePressEvent( QMouseEvent* ev );
  virtual void mouseReleaseEvent( QMouseEvent* ev );
  virtual void mouseMoveEvent( QMouseEvent* ev );
  void popupRefMenu( const QPoint & pos );
  void popupTransfMenu( const QPoint & pos );
  void popupBackgroundMenu( const QPoint & pos );

  void deleteTransformation( anatomist::Transformation* );
  void invertTransformation( anatomist::Transformation* );
  void reloadTransformation( anatomist::Transformation* );
  void saveTransformation( anatomist::Transformation* );
  void mergeReferentials( anatomist::Transformation* );
  virtual bool event( QEvent* );

protected slots:
  void deleteReferential();
  void newReferential();
  void loadReferential();
  void loadNewTransformation();
  void loadTransformationGraph();
  void saveTransformationGraph();
  void clearUnusedReferentials();
  void splitReferential();
  void seeObjectsInReferential();
  void set3DView();
  void view3dDeleted();
  void refreshNow();
  void mergeIdenticalReferentials();

private:
  anatomist::ReferentialWindow_PrivateData	*pdat;
  friend class ReferentialWindow_TransCallback;
};


class ReferentialWindow_TransCallback : public QObject
{
Q_OBJECT

public:
  ReferentialWindow_TransCallback( ReferentialWindow* rw,
                                   anatomist::Transformation* t );
  virtual ~ReferentialWindow_TransCallback();

public slots:
  void deleteTransformation();
  void invertTransformation();
  void reloadTransformation();
  void saveTransformation();
  void mergeReferentials();

private:
  ReferentialWindow *refwin;
  anatomist::Transformation  *trans;
};


#endif
