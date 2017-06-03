View / Controler model
======================

Keyboard and mouse events are managed by a Control / View model. Windows can use several control modes. A control manages all keyboad / mouse events and calls actions. Allowed controls fora  window are managed by a ControlManager.

In other words, Control objects define keyboard / mouse mapping to Actions, and are visible as icons on the left part of windows.

Actions perform "atomic" operations.

To define new interactions in windows, a program has to:

* define actions, classes inheriting the Action class. Their methods will be triggered when the corresponding key/mouse events occur.

* define one or more controls, classes inheriting the Control class. It will map Action methods to input events.

* register new actions and controls to the ActionDictionay and ControlDictonary global objects.

* register new controls to the global ControlManager with appropriate conditions on window type and objects types.


Actions
-------

In C++:
+++++++

Header code:

.. code-block:: c++

    #include <anatomist/controler/action.h>

    class CustomAction : public anatomist::Action
    {
    public:
      CustomAction();
      virtual ~CustomAction();
      static Action* creator();
      virtual std::string name() const { return "CustomAction"; }

      // callbacks which actually do things
      void print3DPosition( int x, int y, int globalX, int globalY );
    };

Definition:

.. code-block:: c++

    #include "customaction.h"
    #include <anatomist/window3D/window3D.h>

    using namespace anatomist;
    using namespace aims;
    using namespace std;

    CustomAction::CustomAction()
      : Action()
    {
    }

    CustomAction::~CustomAction()
    {
    }

    Action* CustomAction::creator()
    {
      return new CustomAction;
    }

    void CustomAction::print3DPosition( int x, int y, int, in )
    {
      AWindow *win = view()->aWindow();

      Point3df pos;
      if( win->positionFromCursor( x, y, pos ) )
      {
        cout << "Position : " << pos << endl;
      }
    }

In Python:
++++++++++

See also: :pyanatomist:`Custom controls example <pyanatomist_examples.html#custom-controls-example>`

.. code-block:: python

    from __future__ import print_function
    import anatomist.direct.api as ana

    class CustomAction(ana.cpp.Action):
        def name(self):
            return 'CustomAction'

        def print3DPosition(x, y, globalX, globalY):
            win = self.view().aWindow()
            pos = win.positionFromCursor(x, y)
            if pos is not None:
                print('Position:', pos)

Controls
--------

In C++:
+++++++

Header code:

.. code-block:: c++

    #include <anatomist/conroler/control.h>

    class CustomControl : public anatomist::Control
    {
    public:
      CustomControl( int priority );
      virtual ~CustomControl();
      static Control *creator();
      virtual void eventAutoSubscription( anatomist::ActionPool* pool );
    };

Definition:

.. code-block:: c++

    #include "customcontrol.h"
    #include "customaction.h"

    using namespace anatomist;

    CustomControl::CustomControl( int priority )
      : Control( priority, "CustomControl" )
    {
    }

    CustomControl::~CustomControl()
    {
    }

    Control* CustomControl::creator()
    {
      return CustomControl( 25 );
    }

    void CustomControl::eventAutoSubscription( ActionPool* pool )
    {
      mousePressButtonEventSubscribe(
        Qt::RightButton, Qt::NoModifier,
        MouseActionLinkOf<CustomAction>(pool->action( "CustomAction" ),
                                        &CustomAction::print3DPosition ),
        "print_3d_position" );
    }


In Python:
++++++++++

.. code-block:: python

    from __future__ import print_function
    import anatomist.direct.api as ana
    from soma.qt_gui.qt_backend imprort QtCore

    class CustomControl(ana.cpp.Control):
      def __init__(self, prio=25):
          ana.cpp.Control.__init__(self, prio, 'CustomControl')

      def eventAutoSubscription(self, pool):
          self.mousePressButtonEventSubscribe(
              QtCore.Qt.LeftButton, QtCore.Qt.NoModifier,
              pool.action('CustomAction').print3DPosition)


Registration in the system
--------------------------

Registration has to be done just once. Calling it several times will have no other effect than printing error messages.

In C++:
+++++++

.. code-block:: c++

    #include <anatomist/controler/controldictionary.h>
    #include <anatomist/controler/controlmanager.h>
    #include <anatomist/controler/actiondictionary.h>
    #include <anatomist/controler/icondictionary.h>
    #include "customcontrol.h"
    #include "customaction.h"
    #include <QPixmap>

    using namespace anatomist;

    // needs to be called from somewhere, once,
    // after Anatomist is instantiated.

    void init_plugin()
    {
      QPixmap pix;
      if( pix.load( Settings::findResourceFile(
                    "icons/customcontrol.jpg" ).c_str() ) )
        IconDictionary::instance()->addIcon( "CustomControl", pix );

      ActionDictionary::instance()->addAction(
        "CustomAction", &CustomAction::creator );

      ControlDictionary::instance()->addControl(
        "CustomControl", &CustomControl::creator, 25 );
      ControlManager::instance()->addControl(
        "QAGLWidget3D", "", "CustomControl" );
    }

In Python:
++++++++++

.. code-block:: python

    from __future__ import print_function
    import anatomist.direct.api as ana
    from soma.qt_gui.qt_backend imprort QtGui
    from custom_action import CustomAction
    from custom_control import CustomControl

    a = ana.Anatomist()
    pix = QtGui.QPixmap('customcontrol.jpg')
    ana.cpp.IconDictionary.instance().addIcon('CustomControl', pix)
    ad = ana.cpp.ActionDictionary.instance()
    ad.addAction('CustomAction', CustomAction)
    cd = ana.cpp.ControlDictionary.instance()
    cd.addControl('CustomControl', CustomControl )
    cm = ana.cpp.ControlManager.instance()
    cm.addControl('QAGLWidget3D', '', 'CustomControl')


Example
-------

:pyanatomist:`Custom controls example <pyanatomist_examples.html#custom-controls-example>`
