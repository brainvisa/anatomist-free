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


#ifndef ANATOMIST_CONTROL_GRAPHPARAMS_H
#define ANATOMIST_CONTROL_GRAPHPARAMS_H


#include <anatomist/application/Anatomist.h>
#include <qwidget.h>
#include <string>
#include <vector>

class Tree;
class QGraphParam;

namespace anatomist
{
  class AGraph;
  class AGraphObject;
  class Material;
  class Hierarchy;

  class GraphParams
  {
  public:
    GraphParams();
    ~GraphParams();

    static GraphParams* graphParams();

    void updateGraphs() const;
    /**	Function used by the AGraph coloring function when in labels color 
       mode. It shouldn't be used directly
       \see AGraph::specialColorFunc */
    static bool recolorLabelledGraph( AGraph* ag, AGraphObject* go, 
                                      Material & mat );
    static bool nomenclatureColorForLabel( const std::string & label,
                                           const Hierarchy *hie,
                                           Material & mat );
    ///	This function should be part of the Tree class (in graph library).
    static Tree* findTreeWith( const Tree* tr, const std::string & attribute, 
                               const std::string & value, 
                               std::vector<Tree *> & parents );
    /// @returns -1 if the mode is unknown
    int selectRenderModeFromString( const std::string & );

    bool	colorsActive;
    std::string	attribute;
    bool	toolTips;
    int		saveMode;
    bool	saveOnlyModified;
    bool	autoSaveDir;
    bool        loadRelations;
    int         selectRenderMode;
    std::vector<std::string> selectRenderModes;

  private:
    friend class AGraph;
    friend class ::QGraphParam;
    static GraphParams*& _graphParams();
    void allowRescanHierarchies( bool x );
    bool rescanhierarchies;
  };

}


///	Graph static parameters control window
class QGraphParam : public QWidget
{
  Q_OBJECT

public:
  QGraphParam( QWidget* parent = theAnatomist->getQWidgetAncestor(), const char* name = 0 );
  virtual ~QGraphParam();

  ///	Access the singleton
  static QGraphParam* theGP();

  void refreshGraphs() const;
  void update();
  ///	Attribute for nomenclature selection / colors
  const std::string & nomenclatureAttrib() const;

  ///	Silent change - doesn't update the window interface
  static void setLabelColorsActivated( bool state );
  static bool labelColorsActivated();
  ///	Silent change - doesn't update the window interface
  static void setColorsAttribute( const std::string & attr );
  static std::string colorsAttribute();
  static bool toolTipsInstalled();
  static int savingMode();
  static bool saveOnlyModified();
  static bool autoSaveDirectory();

protected slots:
  void btnClicked( int btn );
  void colorClicked( bool onoff );
  void attribActivated( const QString & str );
  void installToolTips( bool onoff );
  void invSelColorClicked( bool onoff );
  void selColorClicked();
  void loadRelationsChanged( bool );
  void saveModifChanged( bool );
  void autoDirChanged( bool );
  void saveModeSelected( int );
  void setSelectionRenderingMode( int );

private:
  static QGraphParam* & _qGraphParam();

  struct Private;

  Private	*d;
};



#endif
