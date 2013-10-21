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


#ifndef ANATOMIST_WINDOW3D_LABELEDITACTION_H
#define ANATOMIST_WINDOW3D_LABELEDITACTION_H

#include <anatomist/controler/control.h>
#include <anatomist/controler/action.h>
#include <aims/rgb/rgb.h>


namespace anatomist
{
  class AObject;

  class LabelEditAction : public Action
  {
  public:
    static Action * creator() ;

    LabelEditAction();
    LabelEditAction( const LabelEditAction & a );
    virtual ~LabelEditAction();

    virtual std::string name() const;

    // pick seleciton label
    void pick();
    // set current (picked) label to seleted objects
    void edit();
    // current label
    std::string label() const;
    // set (pick) current label. obj is used to find a nomenclature and color
    void setLabel( const std::string & l, const AObject* obj = 0 );
    void setLabel( const std::string & l, const AimsRGB & color );

    QWidget* actionView( QWidget* );
    bool viewableAction() const;

  private:
    std::string _label;
    AimsRGB     _color;
    struct Private;
    Private *d;
  };

}

#endif

