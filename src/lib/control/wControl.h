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


#ifndef ANA_CONTROL_WCONTROL_H
#define ANA_CONTROL_WCONTROL_H


#include <anatomist/object/optionMatcher.h>
#include <qmainwindow.h>
#include <string>
#include <map>
#include <set>

namespace anatomist
{
  class AWindow;
  class AObject;
  class MObject;
  class AControlMenuHandler;
  class Referential;
}


/**	The ControlWindow class is the Anatomist control window. It reflects 
	the contents of the application, allowing to visualize and handle 
	various data and windows. \\
	The ControlWindow class is based on the MPP MenuWindow class \\ \\
 */
class ControlWindow : public QMainWindow
{
  Q_OBJECT

public:
  ControlWindow();
  virtual ~ControlWindow();

  void registerWindow( anatomist::AWindow *win );
  void unregisterWindow( anatomist::AWindow *win );
  void NotifyWindowChange( anatomist::AWindow* win );
  void registerObject( anatomist::AObject *obj );
  void unregisterObject( anatomist::AObject *obj );
  void registerSubObject( anatomist::MObject* parent, 
			  anatomist::AObject* obj );
  void NotifyObjectChange( anatomist::AObject* obj );

  std::set<anatomist::AObject *> selectedObjects();
  std::set<anatomist::AWindow*> selectedWindows();
  std::set<int> SelectedWinGroups() const;
  void SelectObject( anatomist::AObject *obj );
  void UnselectAllObjects();
  void UnselectAllWindows();
  void SelectWindow( anatomist::AWindow *win );
  void ResizeWindow( anatomist::AWindow *win );
  void BuildObjectMenu();
  void UpdateToggleMenus();
  void UpdateObjectMenu();
  void UpdateWindowMenu();
  anatomist::AControlMenuHandler *menuHandler() { return( _menu ); }
  ///	Are reference colors markers visible ?
  bool ViewingRefColors() const;
  void ToggleRefColorsView();
  void enableRefWinMenu( bool );
  void enableLoadRefMenu( bool );
  void enablePreferencesMenu( bool );
  void enableFusionMenu( bool );
  void enableGroupMenu( bool );
  bool logoEnabled() const;
  void loadObject( const std::string& filter, const std::string& caption );
  anatomist::Referential* defaultObjectsReferential() const;
  anatomist::Referential* defaultWindowsReferential() const;
  void setDefaultObjectsReferential( anatomist::Referential* );
  void setDefaultWindowsReferential( anatomist::Referential* );
  /// allows or forbids closing the control window
  void enableClose( bool );
  bool closeEnabled() const;

  static ControlWindow* theControlWindow() { return( _theControlWindow ); }

public slots:
  void UpdateMenus() { UpdateToggleMenus(); BuildObjectMenu(); }
  void loadObject();
  void saveSettings();
  void saveWindowsConfig();
  void replayScenario();
  /// opens a window of type ID type
  void openWindow( int type );
  /// drop event on windows buttons
  void dropOnWindowIcon( int type, QDropEvent* ev );
  void dragEnterOnWindowIcon( int type, QDragEnterEvent* ev );
  void dragMoveOnWindowIcon( int type, QDragMoveEvent* ev );
  void openAxial();
  void openSagittal();
  void openCoronal();
  void openOblique();
  void open3D();
  void openBrowser();
  void openRefWin();
  void iconifyWindows();
  void restoreWindows();
  void closeWindows();
  void linkWindows();
  void addObjectsInWindows();
  void removeObjects();
  void deleteObjects();
  void reload();
  void groupObjects();
  /// same as chooseReferential() but for windows only
  void chooseWinReferential();
  void chooseReferential();
  void openConstraintEditor();
  void fusionObjects();
  void openPreferencesWin();
  void viewRefColors();
  void about();
  void help();
  void graphParams();
  void quit();
  void languageEnglish();
  void languageFrench();
  void enableLogo( bool );
  void objectTreeClick();
  void objectTreeDblClick();
  void windowTreeClick();
  void windowTreeDblClick( anatomist::AWindow * );
  void reloadPalettes();
  void modulesList();
  void aimsInfo();
  void openThreeViews();
  void openBlockView();
  /// clears all objects / windows / refs / transfos to reset the application
  void clearAll();

protected:
  void closeEvent(QCloseEvent *event);
  virtual void createMenu();
  virtual void createIcons();
  virtual void drawContents();
  virtual void CreateTitle();
  void setLanguage( const std::string & poFilename );

  anatomist::AControlMenuHandler	*_menu;

  static std::string 			_baseTitle;
  static ControlWindow			*_theControlWindow;

protected slots:
  void objectTreeRightClick( anatomist::AObject*, const QPoint & );

private:
  struct Private;
  Private	*d;
};


#endif
