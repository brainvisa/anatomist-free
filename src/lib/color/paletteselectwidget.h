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


#ifndef ANATOMIST_COLOR_PALETTESELECTWIDGET_H
#define ANATOMIST_COLOR_PALETTESELECTWIDGET_H

#include <anatomist/color/palette.h>
#include <QTableWidget>
#include <string>

class QPixmap;


namespace anatomist
{

  class PaletteSelectWidget : public QTableWidget
  {
    Q_OBJECT

  public:
    PaletteSelectWidget( QWidget *parent = 0,
                         const std::string & selected = "",
                         bool allow_none = false );
    virtual ~PaletteSelectWidget();
    std::string selectedPalette() const;
    void selectPalette( const std::string & name );

    static void fillPalette( const carto::rc_ptr<APalette> pal,
                             QPixmap & pix );

  signals:
    void paletteSelected( const std::string & palette );

  protected:
    void fillPalettes();

  protected slots:
    void paletteChanged();

  private:
    std::string _init_selected;
    bool _allow_none;

  };

}

#endif


