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

#include <boost/assign.hpp>
#include <cfloat>
#include <anatomist/window3D/orientationAnnotation.h>
#include <anatomist/surface/transformedobject.h>
#include <anatomist/surface/textobject.h>
#include <anatomist/application/globalConfig.h>
#include <anatomist/window/glwidgetmanager.h>

using namespace anatomist;
using namespace aims;
using namespace std;

//---------------------------------------------------------------
OrientationAnnotation::OrientationAnnotation( AWindow3D * win )
    : AObject(),
      _win( win ),
      _viewType( AWindow3D::Oblique ),
      _fontSize( 0.3 )
{
	initParams();
}

//---------------------------------------------------------------
OrientationAnnotation::OrientationAnnotation( const OrientationAnnotation & a )
    : _win( a._win ),
      _winCoordParams( a._winCoordParams ),
      _winCoordParamIndexes( a._winCoordParamIndexes ),
      _annotMap( a._annotMap ),
	  _textMap( a._textMap ),
	  _activedAnnot( a._activedAnnot ),
	  _viewType( a._viewType ),
	  _nonObliqueSliceQuaternion( a._nonObliqueSliceQuaternion ),
	  _fontSize( a._fontSize )
{
}

//---------------------------------------------------------------
OrientationAnnotation::~OrientationAnnotation()
{
    // Destroy all annotations
    remove();
}

//---------------------------------------------------------------
string OrientationAnnotation::name() const
{
    return "OrientationAnnotation";
}

//---------------------------------------------------------------
void OrientationAnnotation::remove()
{
    if ( !_win )
    {
        return;
    }

    // Remove and destroy all annotations
	std::map< Position, TransformedObject * >::iterator it;
	for ( it = _annotMap.begin() ; it != _annotMap.end() ; it++ )
	{
	    _win->unregisterObject( (*it).second );
		delete (*it).second;
	}
	_annotMap.clear();
	_textMap.clear();
}

//---------------------------------------------------------------
void OrientationAnnotation::update()
{
	if ( !_win )
	{
		return;
	}

	// View type changed: reset all annotations
    if ( _viewType != _win->viewType() )
    {
    	initParams();
    	remove();
    }

    // Remove the annotations if display option is off or if the view is oblique
    if ( !_win->leftRightDisplay() ||
         _nonObliqueSliceQuaternion.vector() != _win->sliceQuaternion().vector() )
    {
        remove();
        return;
    }

    // Update the window coordinate parameters
    updateWindowCoordParams();

    // Annotation map is empty: need to build it
    if ( _annotMap.empty() )
    {
        build();
    }

    // Update the annotation positions
    std::vector< Position >::iterator it;
    for ( it = _activedAnnot.begin() ; it != _activedAnnot.end() ; it++ )
    {
    	_annotMap[ *it ]->setPosition( getPosition( *it ) );
    	_textMap[ *it ]->setScale( _fontSize/_win->getZoom() );
    }
}

//---------------------------------------------------------------
string OrientationAnnotation::getPositionLabel( OrientationAnnotation::Position pos )
{
    // Get the label according to an annotation position
    switch ( pos )
    {
    case OrientationAnnotation::RIGHT:
        return "R";
    case OrientationAnnotation::LEFT:
        return "L";
    case OrientationAnnotation::ANT:
        return "A";
    case OrientationAnnotation::POST:
        return "P";
    case OrientationAnnotation::UP:
        return "U";
    case OrientationAnnotation::DOWN:
        return "D";
    default:
        return "";
    }

    return "";
}

//---------------------------------------------------------------
void OrientationAnnotation::build()
{
    if ( !_win )
    {
        return;
    }

    vector< Position >::iterator it;
    for ( it = _activedAnnot.begin() ; it != _activedAnnot.end() ; it++ )
    {
        // Create text object
        TextObject * to = new TextObject( "" );
        // Set the font size
        to->setScale( _fontSize );
        // Set the label
        std::ostringstream oss;
        oss << getPositionLabel( (*it) );
        std::string text = oss.str();
        to->setText( text );
        // Set the font color
        Material & mat = to->GetMaterial();
        mat.SetDiffuse( 1, 0, 0, 1 );
        to->SetMaterial( mat );
        _textMap[ *it ] = to ;
        vector<AObject *> vto;
        vto.push_back( to );
        // Create transformed object
        _annotMap[ *it ] = new TransformedObject( vto, false, true, getPosition( *it ) );
        _win->registerObject( _annotMap[ *it ], true );
    }
}

//---------------------------------------------------------------
Point3df OrientationAnnotation::getPosition( OrientationAnnotation::Position pos )
{
    if ( !_win )
    {
        return Point3df( FLT_MAX, FLT_MAX, FLT_MAX );
    }

    Point3df bmin, bmax;
    float tmin, tmax;
    // Get the view bounding box (since it is a temporary object, an annotation will be ignored)
    _win->boundingBox( bmin, bmax, tmin, tmax );
    Point3df center = ( bmin + bmax ) * 0.5;
    // Get the zoom factor of the view
    const float zoom = _win->getZoom();

    // Get the view quaternion and init coordinates
    GLWidgetManager * w = dynamic_cast<GLWidgetManager *>( _win->view() );
    const Quaternion & q = w->quaternion();
    Point3df coords = q.transform( Point3df( 0, 0, 0 ) );

    // Compute the coordinates (without shift adjustment)
    for ( int i = 0 ; i < _winCoordParamIndexes[ pos ].size() ; i++ )
    {
        coords[ i ] += _winCoordParams [ _winCoordParamIndexes[ pos ][ i ] ][ i ];
        if ( _winCoordParamIndexes[ pos ][ i ] == 3 )
        {
            coords[ i ] += 2*getZFactor();
        }
    }

    // Compute the shift adjustment according to the window dimensions
    float wf = 1.5;
    theAnatomist->config()->getProperty( "windowSizeFactor", wf );

    float sizeImgAlongAxis = 0.;
    float sizeImgAlongNormAxis = 0.;
    float sizeWinAlongAxis = 0.;
    float sizeWinAlongNormAxis = 0.;
    getImageAndWinDimensions( pos,
                              sizeImgAlongAxis, sizeImgAlongNormAxis,
                              sizeWinAlongAxis, sizeWinAlongNormAxis );

    coords -= center - w->rotationCenter();
    float ratioImg = sizeImgAlongAxis / sizeImgAlongNormAxis;
    float ratioWin = sizeWinAlongAxis / sizeWinAlongNormAxis;
    sizeImgAlongAxis *= wf;
    sizeImgAlongNormAxis *= wf;
    float currImgSizeAlongAxis = 0.;
    if ( ratioImg <= ratioWin )
    {
        currImgSizeAlongAxis = ratioImg * sizeWinAlongNormAxis;
    }
    else
    {
        currImgSizeAlongAxis = sizeWinAlongAxis;
    }
    currImgSizeAlongAxis *= zoom;
    float delta = ( sizeImgAlongAxis / currImgSizeAlongAxis ) * ( sizeWinAlongAxis - currImgSizeAlongAxis ) / ( 2*wf );
    int directionFac = 1;
    int shiftIndex = 0;
    int shiftDirection = 0;
    float shiftValue = 0.;
    getShiftParams( pos, shiftIndex, shiftDirection, shiftValue );
    coords[shiftIndex] += ( delta + shiftValue / zoom ) * shiftDirection;

    return coords;
}

//---------------------------------------------------------------
int OrientationAnnotation::getZFactor() const
{
    if ( !_win )
    {
        return 0;
    }

    // Set the factor to display the annotations in front of the image
    string axConv;
    theAnatomist->config()->getProperty( "axialConvention", axConv );
    const bool isAxConvRadio = ( axConv != "neuro" );

    if ( _viewType == AWindow3D::Axial )
    {
        return ( isAxConvRadio ? 1 : -1 );
    }
    else if ( _viewType == AWindow3D::Coronal )
    {
        return ( isAxConvRadio ? -1 : 1 );
    }
    else if ( _viewType == AWindow3D::Sagittal )
    {
        return 1;
    }

    return 1;
}

//---------------------------------------------------------------
void OrientationAnnotation::getImageAndWinDimensions( OrientationAnnotation::Position pos,
                                                      float & sizeImgAlongAxis, float & sizeImgAlongNormAxis,
                                                      float & sizeWinAlongAxis, float & sizeWinAlongNormAxis ) const
{
    // Get the dimensions in the direction of the annotation axis and in the direction of its perpendicular axis
    GLWidgetManager * w = dynamic_cast<GLWidgetManager *>( _win->view() );
    switch ( pos )
    {
        case OrientationAnnotation::RIGHT:
        case OrientationAnnotation::LEFT:
        {
            sizeImgAlongAxis = _winCoordParams[1][0] - _winCoordParams[0][0];
            if ( _viewType == AWindow3D::Axial )
            {
                sizeImgAlongNormAxis = _winCoordParams[1][1] - _winCoordParams[0][1];
            }
            else if ( _viewType == AWindow3D::Coronal )
            {
                sizeImgAlongNormAxis = _winCoordParams[1][2] - _winCoordParams[0][2];
            }
            sizeWinAlongAxis = w->width();
            sizeWinAlongNormAxis = w->height();

            break;
        }
        case OrientationAnnotation::ANT:
        case OrientationAnnotation::POST:
        {
            sizeImgAlongAxis = _winCoordParams[1][1] - _winCoordParams[0][1];
            if ( _viewType == AWindow3D::Axial )
            {
                sizeImgAlongNormAxis = _winCoordParams[1][0] - _winCoordParams[0][0];
                sizeWinAlongAxis = w->height();
                sizeWinAlongNormAxis = w->width();
            }
            else if ( _viewType == AWindow3D::Sagittal )
            {
                sizeImgAlongNormAxis = _winCoordParams[1][2] - _winCoordParams[0][2];
                sizeWinAlongAxis = w->width();
                sizeWinAlongNormAxis = w->height();
            }

            break;
        }
        case OrientationAnnotation::UP:
        case OrientationAnnotation::DOWN:
        {
            sizeImgAlongAxis = _winCoordParams[1][2] - _winCoordParams[0][2];
            if ( _viewType == AWindow3D::Sagittal )
            {
                sizeImgAlongNormAxis = _winCoordParams[1][1] - _winCoordParams[0][1];
            }
            else if ( _viewType == AWindow3D::Coronal )
            {
                sizeImgAlongNormAxis = _winCoordParams[1][2] - _winCoordParams[0][2];
            }
            sizeWinAlongAxis = w->height();
            sizeWinAlongNormAxis = w->width();

            break;
        }
    }
}

//---------------------------------------------------------------
void OrientationAnnotation::getShiftParams( OrientationAnnotation::Position pos,
                                            int & shiftIndex, int & shiftDirection, float & shiftValue )
{
    // Compute the shift to put the annotation at a window border
    string axConv;
    theAnatomist->config()->getProperty( "axialConvention", axConv );
    const bool isAxConvRadio = ( axConv != "neuro" );
    switch ( pos )
    {
        case OrientationAnnotation::RIGHT:
        {
            shiftDirection = -1;
            if ( _viewType == AWindow3D::Axial || _viewType == AWindow3D::Coronal )
            {
                shiftIndex = 0;
            }
            if ( ( _viewType == AWindow3D::Axial || _viewType == AWindow3D::Coronal ) &&
                 !isAxConvRadio )
            {
                shiftValue = -10;
            }

            break;
        }
        case OrientationAnnotation::LEFT:
        {
            shiftDirection = 1;
            if ( _viewType == AWindow3D::Axial || _viewType == AWindow3D::Coronal )
            {
                shiftIndex = 0;
            }
            if ( ( _viewType == AWindow3D::Axial || _viewType == AWindow3D::Coronal ) &&
                 isAxConvRadio )
            {
                shiftValue = -10;
            }

            break;
        }
        case OrientationAnnotation::ANT:
        {
            shiftDirection = -1;
            if ( _viewType == AWindow3D::Axial || _viewType == AWindow3D::Sagittal )
            {
                shiftIndex = 1;
            }
            if ( _viewType == AWindow3D::Axial )
            {
                shiftValue = -14;
            }

            break;
        }
        case OrientationAnnotation::POST:
        {
            shiftDirection = 1;
            if ( _viewType == AWindow3D::Axial || _viewType == AWindow3D::Sagittal )
            {
                shiftIndex = 1;
            }
            if ( _viewType == AWindow3D::Sagittal )
            {
                shiftValue = -10;
            }

            break;
        }
        case OrientationAnnotation::UP:
        {
            shiftDirection = -1;
            if ( _viewType == AWindow3D::Coronal || _viewType == AWindow3D::Sagittal )
            {
                shiftIndex = 2;
            }
            if ( _viewType == AWindow3D::Coronal || _viewType == AWindow3D::Sagittal )
            {
                shiftValue = -14;
            }

            break;
        }
        case OrientationAnnotation::DOWN:
        {
            shiftDirection = 1;
            if ( _viewType == AWindow3D::Coronal || _viewType == AWindow3D::Sagittal )
            {
                shiftIndex = 2;
            }

            break;
        }
    }
}

//---------------------------------------------------------------
void OrientationAnnotation::initParams()
{
	if ( !_win )
	{
		return;
	}

	// Get view type
	_viewType = _win->viewType();

	// Get the initial slice quaternion to handle with oblique views
	_nonObliqueSliceQuaternion = _win->sliceQuaternion();

	_winCoordParamIndexes.clear();
	// Define the displayed annotations according to the view type
	if ( _viewType == AWindow3D::Axial )
	{
		_activedAnnot = boost::assign::list_of( OrientationAnnotation::RIGHT )
											  ( OrientationAnnotation::LEFT )
											  ( OrientationAnnotation::ANT )
											  ( OrientationAnnotation::POST );

		_winCoordParamIndexes[ RIGHT ] = boost::assign::list_of( 0 )( 2 )( 3 );
		_winCoordParamIndexes[ LEFT ] = boost::assign::list_of( 1 )( 2 )( 3 );
		_winCoordParamIndexes[ ANT ] = boost::assign::list_of( 2 )( 0 )( 3 );
		_winCoordParamIndexes[ POST ] = boost::assign::list_of( 2 )( 1 )( 3 );
	}
	else if ( _viewType == AWindow3D::Coronal )
	{
		_activedAnnot = boost::assign::list_of( OrientationAnnotation::RIGHT )
											  ( OrientationAnnotation::LEFT )
											  ( OrientationAnnotation::UP )
											  ( OrientationAnnotation::DOWN );
		_winCoordParamIndexes[ RIGHT ] = boost::assign::list_of( 0 )( 3 )( 2 );
        _winCoordParamIndexes[ LEFT ] = boost::assign::list_of( 1 )( 3 )( 2 );
        _winCoordParamIndexes[ UP ] = boost::assign::list_of( 2 )( 3 )( 0 );
        _winCoordParamIndexes[ DOWN ] = boost::assign::list_of( 2 )( 3 )( 1 );
	}
	else if ( _viewType == AWindow3D::Sagittal )
	{
		_activedAnnot = boost::assign::list_of( OrientationAnnotation::ANT )
											  ( OrientationAnnotation::POST )
											  ( OrientationAnnotation::UP )
											  ( OrientationAnnotation::DOWN );
		_winCoordParamIndexes[ UP ] = boost::assign::list_of( 3 )( 2 )( 0 );
        _winCoordParamIndexes[ DOWN ] = boost::assign::list_of( 3 )( 2 )( 1 );
        _winCoordParamIndexes[ ANT ] = boost::assign::list_of( 3 )( 0 )( 2 );
        _winCoordParamIndexes[ POST ] = boost::assign::list_of( 3 )( 1 )( 2 );
	}
}

//---------------------------------------------------------------
void OrientationAnnotation::updateWindowCoordParams()
{
    if ( !_win )
    {
        return;
    }

    Point3df bmin, bmax;
    float tmin, tmax;
    // Get the view bounding box (since it is a temporary object, an annotation will be ignored)
    _win->boundingBox( bmin, bmax, tmin, tmax );
    Point3df center = ( bmin + bmax ) * 0.5;

    // Add bounding box min, max, center and current position
    _winCoordParams.clear();
    _winCoordParams = boost::assign::list_of( bmin )
                                            ( bmax )
                                            ( center )
                                            ( _win->GetPosition() );
}
