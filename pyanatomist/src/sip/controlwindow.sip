
%ModuleCode
#if SIP_VERSION >= 0x040700
#include <anatomist/control/wControl.h>
#include <anatomist/window3D/window3D.h>
#endif
%End


class ControlWindow : QMainWindow
{

%TypeHeaderCode
#include <anatomist/control/wControl.h>
#include <anatomist/window/Window.h>
%End

%ConvertToSubClassCode
  if( dynamic_cast<ControlWindow *>( sipCpp ) )
    sipClass = sipClass_ControlWindow;
  else
    if( dynamic_cast<QAWindow *>( sipCpp ) )
      if( dynamic_cast<ControlledWindow *>( sipCpp ) )
        if( dynamic_cast<AWindow3D *>( sipCpp ) )
          sipClass = sipClass_AWindow3D;
        else
          sipClass = sipClass_ControlledWindow;
      else
        sipClass = sipClass_QAWindow;
    else
      sipClass = 0;
%End

public:
  ControlWindow();
  virtual ~ControlWindow();

  set_AWindowPtr selectedWindows();
  void SelectWindow(anatomist::AWindow *win);
  void UnselectAllWindows();
  void enableClose( bool );
  bool closeEnabled() const;
};


