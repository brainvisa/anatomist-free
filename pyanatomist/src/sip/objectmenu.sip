
namespace anatomist
{

class ObjectMenuCallback
{
%TypeHeaderCode
#include <anatomist/object/objectmenu.h>
%End

public:
  ObjectMenuCallback();
  virtual ~ObjectMenuCallback();

  virtual void doit( const set_AObjectPtr & ) = 0;
  virtual bool operator == ( const anatomist::ObjectMenuCallback & ) const;
};


class ObjectMenuRegistrerClass : carto::RCObject
{
%TypeHeaderCode
#include <anatomist/object/objectmenu.h>
%End

public:
  ObjectMenuRegistrerClass();
  virtual ~ObjectMenuRegistrerClass();

  virtual anatomist::ObjectMenu* doit( const anatomist::AObject*,
                                       anatomist::ObjectMenu* ) = 0;
};


class ObjectMenu
{
%TypeHeaderCode
#include <anatomist/object/objectmenu.h>
%End

public:
  ObjectMenu();
  ObjectMenu( const anatomist::ObjectMenu & );
  ObjectMenu( Tree & /Transfer/ );
  ~ObjectMenu();

  void insertItem( const vector_STRING &, const std::string & );
  void insertItem( const vector_STRING &, const std::string &,
                   anatomist::ObjectMenuCallback* /Transfer/ );
  Tree* item( const vector_STRING &, bool = true );
  Tree* tree();
  Tree* releaseTree() /TransferBack/;
};


class ObjectActions
{
%TypeHeaderCode
#include <anatomist/object/actions.h>
%End

public:
  static anatomist::ObjectMenuCallback* fileReloadMenuCallback() /Factory/;
  static anatomist::ObjectMenuCallback* colorPaletteMenuCallback() /Factory/;
  static anatomist::ObjectMenuCallback* colorMaterialMenuCallback() /Factory/;
  static anatomist::ObjectMenuCallback* referentialLoadMenuCallback() /Factory/;
  static anatomist::ObjectMenuCallback* fusion2DControlMenuCallback() /Factory/;
  static anatomist::ObjectMenuCallback* fusion3DControlMenuCallback() /Factory/;
  static anatomist::ObjectMenuCallback* textureControlMenuCallback() /Factory/;
  static anatomist::ObjectMenuCallback* saveStaticMenuCallback() /Factory/;
  static anatomist::ObjectMenuCallback* saveTextureMenuCallback() /Factory/;
  static anatomist::ObjectMenuCallback* extractTextureMenuCallback() /Factory/;
  static anatomist::ObjectMenuCallback* renameObjectMenuCallback() /Factory/;
  static anatomist::ObjectMenuCallback*
    generateTexture1DMenuCallback() /Factory/;
  static anatomist::ObjectMenuCallback*
    generateTexture2DMenuCallback() /Factory/;
  static anatomist::ObjectMenuCallback*
    displayGraphChildrenMenuCallback() /Factory/;

  static void fileReload( const set_AObjectPtr & obj );
  static void colorPalette( const set_AObjectPtr & obj );
  static void colorMaterial( const set_AObjectPtr & obj );
  static void referentialLoad( const set_AObjectPtr & obj );
  static void fusion2DControl( const set_AObjectPtr & obj );
  static void fusion3DControl( const set_AObjectPtr & obj );
  static void textureControl( const set_AObjectPtr & obj );
  static void saveStatic( const set_AObjectPtr & obj ) ;
  static std::string specificSaveStatic( set_AObjectPtr & obj,
                                          const std::string& filter,
                                          const std::string & caption ) ;
  static void saveTexture( const set_AObjectPtr & obj ) ;
  static void extractTexture( const set_AObjectPtr & obj ) ;
  static std::string specificSaveTexture( const set_AObjectPtr & obj,
                                          const std::string& filter,
                                          const std::string & caption ) ;
  static void renameObject( const set_AObjectPtr & obj );
  static void generateTexture1D( const set_AObjectPtr & obj );
  static void generateTexture2D( const set_AObjectPtr & obj );
  static void displayGraphChildren( const set_AObjectPtr & obj );
  static void displayGraphRelations( const set_AObjectPtr & obj );
  static void loadGraphSubObjects( const set_AObjectPtr & obj );

  static bool askName( std::string & newname, const std::string & type = "",
                        const std::string & origname = "" );
  static void setAutomaticReferential( const set_AObjectPtr & obj );
  static void graphLabelToName( const set_AObjectPtr & obj );
  static void graphUseLabel( const set_AObjectPtr & obj );
  static void graphUseName( const set_AObjectPtr & obj );
  static void graphUseDefaultLabelProperty( const set_AObjectPtr & obj );
};

};

