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


#ifndef ANAQT_BROWSER_QWOBJECTBROWSER_H
#define ANAQT_BROWSER_QWOBJECTBROWSER_H

#include <anatomist/window/controlledWindow.h>
#include <anatomist/browser/qObjBrowserWid.h>
#include <cartobase/object/attributed.h>
#include <set>
#include <aims/qtcompat/qlistview.h>

class QStatusBar;
class QLabel;
class QLabelEdit;

namespace anatomist
{
  class Hierarchy;
  class StaticInitializers;
  class AttDescr;
}


/**	Objects browser window
 */
class QObjectBrowser : public ControlledWindow
{
  Q_OBJECT

public:
  ///Bit combinations for edit mode
  enum EditMode
  {
    NORMAL = 0,
    EDIT = 1,
    SENDEDIT = 2 //,
  };

  ///	Attribute editor function type
  typedef bool (*EditFunc)( const std::set<carto::GenericObject*> & ao, 
			    const std::string & att, 
			    QObjectBrowserWidget* br,
                            const std::set<Q3ListViewItem*> & item );

  QObjectBrowser( QWidget * parent=0, const char * name=0, 
                  carto::Object options = carto::none(), 
                  Qt::WFlags f = Qt::WType_TopLevel | Qt::WDestructiveClose );
  virtual ~QObjectBrowser();

  virtual Type type() const;
  void displayClickPoint();
  void Draw( bool flush = true );
  virtual void update( const anatomist::Observable* observable, void* arg );
  /// Add an object to the objects
  virtual void registerObject( anatomist::AObject* object,
                               bool temporaryObject = false,
                               int position = -1 );
  /// Remove an object from the objects
  virtual void unregisterObject( anatomist::AObject* object );
  /// Updates an object contents
  virtual void updateObject( anatomist::AObject* obj );

  /**@name	Edit mode related fonctions */
  //@{
  virtual void setMode( unsigned mode );
  unsigned mode() const;
  virtual std::string modeString() const;
  void editCancel();
  void editValidate();
  //@}

  void setShowDetailsUponRegister( bool );
  bool showDetailsUponRegister() const;

  static int classType();
  static anatomist::AttDescr & attDescr();
  static void registerTypeEditor( const std::string & type, EditFunc func );
  static void registerAttributeEditor( const std::string & syntax, 
				       const std::string & att, 
				       EditFunc func );
  static bool stringEditor( const std::set<carto::GenericObject*> & ao,
			    const std::string & att, 
			    QObjectBrowserWidget* br,
                            const std::set<Q3ListViewItem*> & item );
  static bool intEditor( const std::set<carto::GenericObject*> & ao,
                         const std::string & att,
			 QObjectBrowserWidget* br,
                         const std::set<Q3ListViewItem*> & item );
  static bool floatEditor( const std::set<carto::GenericObject*> & ao,
			   const std::string & att, 
			   QObjectBrowserWidget* br,
                           const std::set<Q3ListViewItem*> & item );
  static bool doubleEditor( const std::set<carto::GenericObject*> & ao,
			    const std::string & att, 
			    QObjectBrowserWidget* br,
                            const std::set<Q3ListViewItem*> & item );
  static bool labelEditor( const std::set<carto::GenericObject*> & ao,
			   const std::string & att, 
			   QObjectBrowserWidget* br,
                           const std::set<Q3ListViewItem*> & item );
  static bool colorEditor( const std::set<carto::GenericObject*> & ao,
			   const std::string & att, 
			   QObjectBrowserWidget* br,
                           const std::set<Q3ListViewItem*> & item );

  static anatomist::AWindow* createBrowser( void *, carto::Object );

  virtual const std::set<unsigned> & typeCount() const;
  virtual std::set<unsigned> & typeCount();
  virtual const std::string & baseTitle() const;

  virtual anatomist::View* view();
  virtual const anatomist::View* view() const;

protected:
  void updateRightPanel();
  ///	returns a pointer to the default action item (if any, 0 if none)
  Tree* buildSpecificMenuTree( QObjectBrowserWidget* br, 
			       Q3ListViewItem* item, 
			       Tree & tr );
  static void modifyAttributeStatic( void* parent );
  virtual void modifyAttribute();
  static void modifNameStatic( void* parent );
  static void modifLabelStatic( void* parent );
  virtual void modifAttrib( const std::string & attrib );
  static void addAttributeStatic( void* parent );
  virtual void addAttribute();
  static void removePropertyStatic( void* parent );
  virtual void removeProperty();
  /**	Finds the attribute pointed to by the list item, and returns 
	its parent GenericObject. Returns 0 if not found */
  virtual carto::GenericObject* 
  attributeCaract( const QObjectBrowserWidget* br, const Q3ListViewItem* item, 
		   std::string & attrib );
  virtual carto::GenericObject* gObject( const QObjectBrowserWidget* br, 
					    const Q3ListViewItem* item, 
					    int type );
  virtual anatomist::AObject* aObject( const QObjectBrowserWidget* br, 
				       const Q3ListViewItem* item );
  ///	dispatcher function: chooses the right editor function
  bool editAttribute( carto::GenericObject* ao, const std::string & att, 
		      QObjectBrowserWidget* br, Q3ListViewItem* item, 
		      bool & edited );
  bool editAttribute( const std::set<carto::GenericObject*> & objs,
                      const std::string & att,
                      QObjectBrowserWidget* br,
                      const std::set<Q3ListViewItem*> & items,
                      bool & edited );
  ///	called by leftSelectionChanged() in NORMAL mode
  void normalModeSelectionChanged();
  ///	called in SENDEDIT mode
  static void sendModeSelection( void *parent );
  static void setAttributeToAllSelected( void *parent );
  ///	tells if a nomenclature item can send its value (and if so, returns it)
  std::string canSend( QObjectBrowserWidget* br, Q3ListViewItem* item );
  /// same as canSend() but does not check if a browser is in edit mode
  std::string canSendToAny( QObjectBrowserWidget* br, Q3ListViewItem* item );
  ///	special selection mode of Nomenclature nodes
  void nomenclatureClick( anatomist::Hierarchy* h, 
                          QObjectBrowserWidget::ItemDescr & descr, 
                          std::set<anatomist::AObject *> & tosel );
  static unsigned countSelected( Q3ListViewItem* parent, 
				 Q3ListViewItem* & current );
  /// modifier can be 0 (normal) or 1 (select edge ends)
  void updateRightSelectionChange( int modifier );
  virtual bool event( QEvent* ev );
  virtual void keyPressEvent( QKeyEvent* ev );

protected slots:
  virtual void leftSelectionChangedSlot();
  virtual void rightButtonClickedSlot( Q3ListViewItem *, const QPoint &, int );
  virtual void rightButtonRightPanel( Q3ListViewItem *, const QPoint &, int );
  virtual void doubleClickedSlot( Q3ListViewItem* );
  void rightPanelDoubleClicked( Q3ListViewItem* );
  virtual void leftItemRenamed( Q3ListViewItem * item, int col,
                                const QString & text );
  virtual void rightSelectionChangedSlot();
  virtual void refreshNow();
  void updateRightPanelNow();
  void startDrag( Q3ListViewItem*, Qt::ButtonState );
  virtual void leftItemStartsRename( Q3ListViewItem * item, int col );
  virtual void leftItemCancelsRename( Q3ListViewItem * item, int col );

private:
  struct Private;
  struct Static;
  friend class anatomist::StaticInitializers;

  ///	ensures the window class is registered in Anatomist
  static int registerClass();
  static Static & staticState();

  Private  *d;
};


#endif

