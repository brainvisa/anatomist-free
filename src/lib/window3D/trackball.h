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


#ifndef ANATOMIST_WINDOW3D_TRACKBALL_H
#define ANATOMIST_WINDOW3D_TRACKBALL_H


#include <anatomist/controler/action.h>
#include <aims/resampling/quaternion.h>
#include <qobject.h>

namespace aims
{
  class Quaternion;
}


namespace anatomist
{

  class Trackball : public Action
  {
  public:
    static Action * creator() ;

    Trackball();
    Trackball( const Trackball & a );
    virtual ~Trackball();

    virtual std::string name() const;
    QWidget* actionView( QWidget* );
    bool viewableAction() const;

    virtual void beginTrackball( int x, int y, int globalX, int globalY );
    virtual void moveTrackball( int x, int y, int globalX, int globalY );
    /**	Performs trackball calculations, but doesn't update view
	\return true if something has changed (view must be updated) */
    virtual bool moveTrackballInternal( int x, int y );
    virtual void endTrackball( int x, int y, int globalX, int globalY );
    void setCenter();
    void showRotationCenter();

  protected:
    static aims::Quaternion initQuaternion( float x1, float y1, float x2,
                                            float y2 );
    static float tbProj2Sphere( float r, float x, float y );

    int		_beginx;
    int		_beginy;
    aims::Quaternion _beginquat;
  };


  class ContinuousTrackball : public QObject, public Trackball
  {
    Q_OBJECT

  public:
    static Action * creator() ;

    ContinuousTrackball();
    ContinuousTrackball( const ContinuousTrackball & a );
    virtual ~ContinuousTrackball();

    virtual std::string name() const;

    virtual void beginTrackball( int x, int y, int globalX, int globalY );
    virtual void moveTrackball( int x, int y, int globalX, int globalY );
    void endTrackball( int x, int y, int globalX, int globalY );
    void startOrStop();
    void stop();

  public slots:
    void goOn();

  private:
    struct Private;
    Private	*d;
  };


  class KeyFlightAction : public Action
  {
  public:
    static Action* creator() ;

    KeyFlightAction();
    KeyFlightAction( const KeyFlightAction & a );
    virtual ~KeyFlightAction();

    virtual std::string name() const;
    QWidget* actionView( QWidget* );
    bool viewableAction() const;
    
    void up();
    void down();
    void left();
    void right();
    void spinLeft();
    void spinRight();
    void boost();
    void brake();
    void release();
    void stop();
    void runStep();
    void reverse();

    void increaseAngleSpeed();
    void decreaseAngleSpeed();

  private:
    float		_angle;
    float		_speed;
    bool		_angleChanged;
    bool		_auto;
    static float	_maxAngle;
    static float	_maxSpeed;
  };

}


#endif
