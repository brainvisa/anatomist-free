%Import aims.sip

// switch to Qt 3 or Qt 4
%#if QT_VERSION >= 0x040000%
%Import QtOpenGL/QtOpenGLmod.sip
%#else%
%Import qt/qtmod.sip
%Import qtgl/qtglmod.sip
%#endif%

%Module anatomist.cpp.anatomistsip

%Include anatomist.sip

