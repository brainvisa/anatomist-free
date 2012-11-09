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

#ifndef ANATOMIST_WINDOW3D_ORIENTATIONANNOTATION_H
#define ANATOMIST_WINDOW3D_ORIENTATIONANNOTATION_H

#include <anatomist/object/Object.h>
#include <anatomist/window3D/window3D.h>
#include <aims/resampling/quaternion.h>
#include <string>
#include <map>

namespace anatomist
{
    class TransformedObject;
    class TextObject;

    class OrientationAnnotation : public AObject
    {
    public:
    	// Position enum
        enum Position
        {
            RIGHT = 0,
            LEFT,
            ANT,
            POST,
            UP,
            DOWN
        };

        OrientationAnnotation( AWindow3D * win );
        OrientationAnnotation( const OrientationAnnotation & a );
        virtual ~OrientationAnnotation();
        // Gets the object name
        virtual std::string name() const;
        // Removes annotations
        void remove();
        // Updates annotations
        void update();
        virtual bool Is2DObject()
        {
            return false;
        }
        virtual bool Is3DObject()
        {
            return true;
        }
        // Gets a annotation position label
        static std::string getPositionLabel( OrientationAnnotation::Position pos );

    protected:
        // Builds annotations
        void build();
        // Gets a annotation position
        Point3df getPosition( OrientationAnnotation::Position pos );
        // Gets the view depth direction
        int getZFactor() const;
        // Gets the annotation shift parameters
        void getShiftParams( OrientationAnnotation::Position pos,
                             int & shiftIndex, int & shiftDirection, float & shiftValue );
        // Get the image and window dimensions
        void getImageAndWinDimensions( OrientationAnnotation::Position pos,
                                       float & sizeImgAlongAxis, float & sizeImgAlongNormAxis,
                                       float & sizeWinAlongAxis, float & sizeWinAlongNormAxis ) const;
        // Sets the available annotations according to the 3D window view type
        void initParams();
        // Updates the window coordinate parameters
        void updateWindowCoordParams();

        AWindow3D * _win; // The view window
        std::vector< Point3df > _winCoordParams; // The coordinate parameters: < bbox min, bbox max, bbox center, current position >
        std::map< Position, std::vector< int > > _winCoordParamIndexes; // The indexes to know what coordinate parameter to use for a given axis
        std::map< Position, TransformedObject * > _annotMap; // The annotation map
        std::map< Position, TextObject * > _textMap; // The text map
        std::vector< Position > _activedAnnot; // The current active annotations
        AWindow3D::ViewType _viewType; // The window view type
        aims::Quaternion _nonObliqueSliceQuaternion; // The initial slice quaternion
        float _fontSize; // The font size
    };
}

#endif
