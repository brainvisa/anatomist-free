
%Include controlwindow.sip
%Include glComponent.sip
%Include referential.sip
%Include palette.sip
%Include map_STRING_ObjectMenuPtr.sip
%Include object.sip
%Include slice.sip
%Include objectmenu.sip
%Include mobject.sip
%Include window.sip
%Include controlledWindow.sip
%Include asurface_3.sip
%Include asurface_2.sip
%Include objectlist.sip
%Include graph.sip
%Include map_S32_QPixmap.sip
%Include qobjecttree.sip
%Include unserializer.sip
%Include rcptr_Unserializer.sip
%Include command.sip
%Include event.sip
%Include rcptr_EventHandler.sip
%Include module.sip
%Include control_VOID.sip
%Include set_AObjectPtr.sip
%Include set_AWindowPtr.sip
%Include set_ReferentialPtr.sip
%Include set_TransformationPtr.sip
%Include vector_AObjectPtr.sip
//%Include map_U32_set_AObjectPtr.sip
//%Include map_U32_set_AWindowPtr.sip
%Include selector.sip
%Include observer.sip
%Include fusion.sip
%Include material.sip
%Include primitive.sip
%Include rcptr_GLItem.sip
%Include list_RefGLItem.sip
%Include geometry.sip
%Include viewstate.sip
%Include rcptr_AObject.sip
%Include rcptr_AWindow.sip
%Include sharedptr_AObject.sip
%Include sharedptr_AWindow.sip
%Include control3d.sip
%Include rcptr_APalette.sip
%Include list_rc_ptr_APalette.sip
%Include objectparamselect.sip


class AnatomistSip
{
%TypeHeaderCode
#include <pyanatomist/pyanatomist.h>
#include <pyanatomist/setaobject.h>
#include <pyanatomist/setawindow.h>
#include <cartobase/config/verbose.h>
%End

public:

  AnatomistSip( const vector_STRING & = vector_STRING() );
  QWidget *createWindow( const QString &, QWidget * = 0 ) /Factory/;
  void releaseWindow( anatomist::AWindow * );
  void releaseObject( anatomist::AObject * );
  QString anatomistSharedPath();
  QString anatomistHomePath();
  ControlWindow *getControlWindow();
  set_AWindowPtr getWindowsInGroup( int = 0 );
  void setObjectName( anatomist::AObject*, const std::string & );
  anatomist::Processor *theProcessor();
  set_AObjectPtr getObjects () const;
  set_AWindowPtr getWindows () const;
  set_ReferentialPtr getReferentials() const;
  set_TransformationPtr getTransformations() const;
  anatomist::PaletteList & palettes();
  Point3df lastPosition( const anatomist::Referential *toref=0 ) const;
  int userLevel() const;
  void setUserLevel( int );

  void registerObject( anatomist::AObject *, bool = true );
%MethodCode
  theAnatomist->registerObject( a0, a1 );
%End
  void unregisterObject( anatomist::AObject * );
%MethodCode
  theAnatomist->unregisterObject( a0 );
%End
  anatomist::Referential *centralReferential() const;
%MethodCode
  sipRes = theAnatomist->centralReferential();
%End
    int verbose();
%MethodCode
    sipRes = carto::verbose;
%End

  anatomist::Transformation*
    getTransformation( const anatomist::Referential*,
                       const anatomist::Referential* );
%MethodCode
  sipRes = theAnatomist->getTransformation( a0, a1 );
%End
};


class AObjectConverter
{
%TypeHeaderCode
#include <pyanatomist/pyanatomist.h>
%End

public:
  static AimsData_U8 *aimsData_U8( anatomist::AObject* );
  static AimsData_S16 *aimsData_S16( anatomist::AObject* );
  static AimsData_U16 *aimsData_U16( anatomist::AObject* );
  static AimsData_S32 *aimsData_S32( anatomist::AObject* );
  static AimsData_U32 *aimsData_U32( anatomist::AObject* );
  static AimsData_FLOAT *aimsData_FLOAT( anatomist::AObject* );
  static AimsData_DOUBLE *aimsData_DOUBLE( anatomist::AObject* );
  static AimsData_RGB *aimsData_RGB( anatomist::AObject* );
  static AimsData_RGBA *aimsData_RGBA( anatomist::AObject* );
  static AimsTimeSurface_2 *aimsSurface2( anatomist::AObject* );
  static AimsTimeSurface_3 *aimsSurface3( anatomist::AObject* );
  static AimsTimeSurface_4 *aimsSurface4( anatomist::AObject* );
  static BucketMap_VOID *aimsBucketMap_VOID( anatomist::AObject* );
  static TimeTexture_FLOAT *aimsTexture_FLOAT( anatomist::AObject* );
/*  static TimeTexture_S16 *aimsTexture_S16( anatomist::AObject* );
  static TimeTexture_S32 *aimsTexture_S32( anatomist::AObject* );
  static TimeTexture_U32 *aimsTexture_U32( anatomist::AObject* );
*/
  static TimeTexture_POINT2DF *aimsTexture_POINT2DF( anatomist::AObject* );
  static Graph *aimsGraph( anatomist::AObject* );

  static anatomist::AObject* anatomist( AimsData_U8* );
  static anatomist::AObject* anatomist( AimsData_S16* );
  static anatomist::AObject* anatomist( AimsData_U16* );
  static anatomist::AObject* anatomist( AimsData_S32* );
  static anatomist::AObject* anatomist( AimsData_U32* );
  static anatomist::AObject* anatomist( AimsData_FLOAT* );
  static anatomist::AObject* anatomist( AimsData_DOUBLE* );
  static anatomist::AObject* anatomist( AimsData_RGB* );
  static anatomist::AObject* anatomist( AimsData_RGBA* );
  static anatomist::AObject* anatomist( AimsTimeSurface_2* );
  static anatomist::AObject* anatomist( AimsTimeSurface_3* );
  static anatomist::AObject* anatomist( AimsTimeSurface_4* );
  static anatomist::AObject* anatomist( BucketMap_VOID * );
  static anatomist::AObject* anatomist( Graph * /Transfer/ );
  static anatomist::AObject* anatomist( TimeTexture_FLOAT * );
  static anatomist::AObject* anatomist( TimeTexture_S16 * );
  static anatomist::AObject* anatomist( TimeTexture_S32 * );
  static anatomist::AObject* anatomist( TimeTexture_U32 * );
  static anatomist::AObject* anatomist( TimeTexture_POINT2DF * );
};

