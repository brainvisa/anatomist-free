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
#include <anatomist/window3D/orientationAnnotation.h>
#include <anatomist/application/globalConfig.h>
#include <anatomist/window/glwidgetmanager.h>
#include <QGraphicsView>
#include <QGraphicsItem>

// Macro to deal with boost convert_to_container issue 
// using some specific GCC versions
#if __cplusplus >= 201100
#define BOOST_VECTOR_OF(TYPE, ...) \
boost::assign::list_of<TYPE> __VA_ARGS__ .convert_to_container<vector<TYPE> >()
#else
#define BOOST_VECTOR_OF(TYPE, ...) \
boost::assign::list_of<TYPE> __VA_ARGS__
#endif

#define BOOST_VECTOR_OF_INT(...) \
BOOST_VECTOR_OF( int, __VA_ARGS__ )


using namespace anatomist;
using namespace std;

OrientationAnnotation::OrientationAnnotation(AWindow3D* win)
    : QObject(),
      win_(win)
{
    InitParameters();
    connect(win_, SIGNAL(refreshed()), this, SLOT(update()), Qt::UniqueConnection);
}

OrientationAnnotation::OrientationAnnotation(const OrientationAnnotation& a)
    : QObject(),
      win_(a.win_)
{
}

OrientationAnnotation::~OrientationAnnotation()
{
}

void OrientationAnnotation::update()
{
    InitParameters();
    UpdateText();
}

QGraphicsView* OrientationAnnotation::GraphicsViewOnWindow()
{
    if (!win_)
    {
        return 0;
    }

    GLWidgetManager* gl_manager = dynamic_cast<GLWidgetManager*>(win_->view());
    QWidget* parent = gl_manager->qglWidget()->parentWidget();
    QGraphicsView* g_view = dynamic_cast<QGraphicsView*>(parent);
    if (g_view)
    {
        return g_view;
    }
    return 0;
}


void OrientationAnnotation::UpdateText()
{
    ClearTemporaryItems();
    if (!win_->leftRightDisplay() ||
        win_->isViewOblique())
    {
        return;
    }

    QGraphicsView* g_view = GraphicsViewOnWindow();
    if( !g_view )
      return;
    QGraphicsScene* scene = g_view->scene();
    if (!scene)
    {
        scene = new QGraphicsScene(g_view);
        g_view->setScene(scene);
    }

    for (map<Position, std::vector<int> >::iterator it=annotation_coord_params_.begin(); it!=annotation_coord_params_.end(); ++it)
    {
        DrawText(it->first);
    }
}

void OrientationAnnotation::DrawText(OrientationAnnotation::Position position)
{
    const vector<string> displayed_annotations = win_->displayedAnnotations();
    if (std::find(displayed_annotations.begin(), displayed_annotations.end(),
                  PositionFullLabel(position).toStdString()) == displayed_annotations.end())
    {
        return;
    }

    QGraphicsView* g_view = GraphicsViewOnWindow();
    if( !g_view )
      return;
    QGraphicsSimpleTextItem* g_text = new QGraphicsSimpleTextItem(PositionLabel(position));
    QGraphicsScene* scene = g_view->scene();
    QFont font = g_text->font();
    font.setPointSize(win_->leftRightDisplaySize());
    g_text->setFont(font);
    g_text->setScale(1.);
    QPen pen = QPen(QColor(255, 0, 0));
    pen.setWidth(1);
    g_text->setPen(pen);
    g_text->setBrush(QBrush(QColor(255, 0, 0)));
    QTransform tr = g_text->transform();
    const QPointF center = scene->sceneRect().center();
    float posx, posy;
    posx = center.x()*annotation_coord_params_[position][0];
    posy = center.y()*annotation_coord_params_[position][1];
    tr.translate(posx, posy);
    g_text->setTransform(tr);
    ConstrainCoordinates(g_text, scene);
    g_view->scene()->addItem(g_text);
    temporary_items_.push_back(g_text);
}

void OrientationAnnotation::ClearTemporaryItems()
{
    QGraphicsView* g_view = GraphicsViewOnWindow();
    if (!g_view)
    {
        return;
    }
    QGraphicsScene* scene = g_view->scene();
    list<QGraphicsItem *>::iterator it, et = temporary_items_.end();
    for (it=temporary_items_.begin(); it!=et; ++it)
    {
        scene->removeItem(*it);
        delete *it;
    }
    temporary_items_.clear();
}

QString OrientationAnnotation::PositionLabel(OrientationAnnotation::Position position)
{
    switch (position)
    {
    case OrientationAnnotation::RIGHT:
        return "R";
    case OrientationAnnotation::LEFT:
        return "L";
    case OrientationAnnotation::ANT:
        return "A";
    case OrientationAnnotation::POST:
        return "P";
    case OrientationAnnotation::SUP:
        return "S";
    case OrientationAnnotation::INF:
        return "I";
    default:
        return "";
    }

    return "";
}

QString OrientationAnnotation::PositionFullLabel(OrientationAnnotation::Position position)
{
    switch (position)
    {
    case OrientationAnnotation::RIGHT:
        return "Right";
    case OrientationAnnotation::LEFT:
        return "Left";
    case OrientationAnnotation::ANT:
        return "Anterior";
    case OrientationAnnotation::POST:
        return "Posterior";
    case OrientationAnnotation::SUP:
        return "Superior";
    case OrientationAnnotation::INF:
        return "Inferior";
    default:
        return "";
    }

    return "";
}

void OrientationAnnotation::ConstrainCoordinates(QGraphicsSimpleTextItem* g_text, const QGraphicsScene* scene)
{
    float adjust_posx = 0.;
    float adjust_posy = 0.;
    bool v_align = false;
    bool h_align = false;
    const QPointF pos_to_scene = g_text->mapToScene(g_text->pos());
    const QRectF g_text_rect = g_text->boundingRect();
    if (pos_to_scene.x() <= 0.)
    {
        adjust_posx = 0.;
        adjust_posy = -g_text_rect.height()*0.5;
    }
    else if (pos_to_scene.x() + g_text_rect.right() >= scene->width())
    {
        adjust_posx = -g_text_rect.width();
        adjust_posy = -g_text_rect.height()*0.5;
	}
    if (pos_to_scene.y() <= 0.)
    {
        adjust_posy = 0.;
        adjust_posx = -g_text_rect.width()*0.5;
    }
    else if (pos_to_scene.y() + g_text_rect.bottom() >= scene->height())
    {
        adjust_posy = -g_text_rect.height();
        adjust_posx = -g_text_rect.width()*0.5;
    }
    g_text->setPos(adjust_posx, adjust_posy);
}

void OrientationAnnotation::InitParameters()
{
    if (!win_)
    {
        return;
    }

    AWindow3D::ViewType view_type = win_->viewType();
    string ax_conv;
    theAnatomist->config()->getProperty("axialConvention", ax_conv);
    const bool is_ax_conv_radio = (ax_conv != "neuro");
    annotation_coord_params_.clear();

    if (view_type == AWindow3D::Axial)
    {
        if (is_ax_conv_radio)
        {
            annotation_coord_params_[RIGHT] = BOOST_VECTOR_OF_INT((0)(1));
            annotation_coord_params_[LEFT] = BOOST_VECTOR_OF_INT((2)(1));
        }
        else
        {
            annotation_coord_params_[RIGHT] = BOOST_VECTOR_OF_INT((2)(1));
            annotation_coord_params_[LEFT] = BOOST_VECTOR_OF_INT((0)(1));
        }
        annotation_coord_params_[ANT] = BOOST_VECTOR_OF_INT((1)(0));
        annotation_coord_params_[POST] = BOOST_VECTOR_OF_INT((1)(2));
    }
    else if (view_type == AWindow3D::Coronal)
    {
        if (is_ax_conv_radio)
        {
            annotation_coord_params_[RIGHT] = BOOST_VECTOR_OF_INT((0)(1));
            annotation_coord_params_[LEFT] = BOOST_VECTOR_OF_INT((2)(1));
        }
        else
        {
            annotation_coord_params_[RIGHT] = BOOST_VECTOR_OF_INT((2)(1));
            annotation_coord_params_[LEFT] = BOOST_VECTOR_OF_INT((0)(1));
        }
        annotation_coord_params_[SUP] = BOOST_VECTOR_OF_INT((1)(0));
        annotation_coord_params_[INF] = BOOST_VECTOR_OF_INT((1)(2));
    }
    else if (view_type == AWindow3D::Sagittal)
    {
        annotation_coord_params_[ANT] = BOOST_VECTOR_OF_INT((0)(1));
        annotation_coord_params_[POST] = BOOST_VECTOR_OF_INT((2)(1));
        annotation_coord_params_[SUP] = BOOST_VECTOR_OF_INT((1)(0));
        annotation_coord_params_[INF] = BOOST_VECTOR_OF_INT((1)(2));
    }
}
