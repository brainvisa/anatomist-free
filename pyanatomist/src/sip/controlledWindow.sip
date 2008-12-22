
class ControlledWindow : QAWindow /Abstract/
{
%TypeHeaderCode
#include <anatomist/window/Window.h>
#include <anatomist/window/qwindow.h>
#include <anatomist/window/controlledWindow.h>
#include <stdio.h>
%End

public:
  ControlledWindow( QWidget* = 0, const char* = 0, 
		    carto::Object = carto::none(), 
                    Qt::WFlags = Qt::WType_TopLevel );
  virtual ~ControlledWindow();

  virtual void registerObject( anatomist::AObject* , bool = false, int=-1 );
  virtual void unregisterObject( anatomist::AObject* );

  virtual void updateAvailableControls();
  virtual void updateActivableControls();
  virtual void updateActions();
  virtual void updateActiveControl();

  virtual const anatomist::View* view() const = 0;
  
  static ControlledWindow* fromObject( carto::GenericObject );
%MethodCode
  try
  {
    anatomist::AWindow *aw = a0->value<anatomist::AWindow *>();
    sipRes = dynamic_cast<ControlledWindow *>(aw);
  }
  catch( std::exception & e )
  {
    sipIsErr = 1;
    PyErr_SetString( PyExc_RuntimeError, e.what() );
    sipRes = 0;
  }
%End

};