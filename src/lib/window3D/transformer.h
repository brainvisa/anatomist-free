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


#ifndef ANATOMIST_WINDOW3D_TRANSFORMER_H
#define ANATOMIST_WINDOW3D_TRANSFORMER_H

#include <anatomist/window3D/trackball.h>
#include <qobject.h>
#include <map>

class QGraphicsItem;
class QTableWidget;
class QLineEdit;
class QComboBox;

namespace anatomist
{
  class Transformation;
  class AWindow;
  class Referential;

  namespace internal
  {

    class TransformerActionData : public QObject
    {
    Q_OBJECT

    public:
      TransformerActionData();
      TransformerActionData( const TransformerActionData & );
      virtual ~TransformerActionData();

      Transformation* mainTransformation() const;
      void selectTransformations( AWindow * );
      void setMainTransformation( Transformation* t );
      bool isMainTransDirect() const;
      Referential* mainSourceRef() const;
      Referential* mainDestRef() const;
      void setTransformData( const Transformation & t, bool absolute=false );
      virtual aims::Quaternion initialQuaternion() = 0;
      virtual View* tadView() = 0;

    public slots:
      virtual void resetTransform();
      virtual void resetRotation();

    protected:
      Transformation *_maintrans;
      std::map<Transformation*, Transformation>   _trans;
      std::map<Transformation*, Transformation>   _itrans;
      Point3df rotationAxis;

      virtual void updateTemporaryObjects(
        const aims::Quaternion & rotation ) = 0;
      virtual void updateGVInfo( const aims::Quaternion & q ) = 0;
      virtual void matrixCellChanged( int row, int col, QTableWidget* twid );
      virtual void axisCellChanged( int row, int col, QTableWidget* twid );
      virtual void centerCellChanged( int row, int col, QTableWidget* twid );
      virtual void rotationAngleChanged( QLineEdit* ledit, QComboBox* unit );
      virtual void rotationScaleChanged( QLineEdit* ledit );
    };

  }


  class Transformer
    : public internal::TransformerActionData, public Trackball
  {
    Q_OBJECT

  public:
    static Action * creator() ;

    Transformer();
    Transformer( const Transformer & a );
    virtual ~Transformer();

    virtual std::string name() const;

    virtual void beginTrackball( int x, int y, int globalX, int globalY );
    virtual void moveTrackball( int x, int y, int globalX, int globalY );
    virtual void endTrackball( int x, int y, int globalX, int globalY );
    virtual aims::Quaternion rotation( int x, int y );
    virtual aims::Quaternion initialQuaternion();
    virtual void showGraphicsView();
    virtual void clearGraphicsView();
    void toggleDisplayInfo();
    virtual View* tadView() { return view(); }

    struct Private; // not private since it is used by TranslaterAction
    Private* data();

  public slots:
    virtual void matrixCellChanged( int row, int col );
    virtual void axisCellChanged( int row, int col );
    virtual void centerCellChanged( int row, int col );
    virtual void rotationAngleChanged();
    virtual void rotationScaleChanged();

  protected:
    Private *d;

    virtual void updateTemporaryObjects( const aims::Quaternion & rotation );
    virtual void updateGVInfo( const aims::Quaternion & q );
  };


  class TranslaterAction
    : public internal::TransformerActionData, public Action
  {
  public:
    static Action * creator() ;

    TranslaterAction();
    TranslaterAction( const TranslaterAction & a );
    virtual ~TranslaterAction();

    virtual std::string name() const;

    virtual void begin( int x, int y, int globalX, int globalY );
    virtual void move( int x, int y, int globalX, int globalY );
    virtual void end( int x, int y, int globalX, int globalY );
    virtual View* tadView() { return view(); }
    virtual aims::Quaternion initialQuaternion();

  protected:
    struct Private;
    Private *d;
    bool	_started;
    int		_beginx;
    int		_beginy;

    virtual void updateTemporaryObjects( const aims::Quaternion & rotation );
    virtual void updateGVInfo( const aims::Quaternion & q );
  };


  class PlanarTransformer : public Transformer
  {
  public:
    static Action * creator() ;

    PlanarTransformer();
    PlanarTransformer( const PlanarTransformer & a );
    virtual ~PlanarTransformer();

    virtual std::string name() const;

    virtual aims::Quaternion rotation( int x, int y );
    virtual aims::Quaternion initialQuaternion();
  };


  class ResizerAction : public TranslaterAction
  {
  public:
    static Action * creator() ;

    ResizerAction();
    ResizerAction( const ResizerAction & a );
    virtual ~ResizerAction();

    virtual std::string name() const;

    virtual void begin( int x, int y, int globalX, int globalY );
    virtual void move( int x, int y, int globalX, int globalY );

  protected:
    virtual void updateTemporaryObjects( float zoom );
  };

}

#endif


