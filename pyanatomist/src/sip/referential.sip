namespace anatomist
{

class Referential
{
%TypeHeaderCode
#include <anatomist/reference/Referential.h>
%End

public:
  Referential();
  virtual ~Referential();

  std::string uuid() const;
%MethodCode
  sipRes = new std::string( sipCpp->uuid().toString() );
%End

  carto::GenericObject & header();

  static anatomist::Referential *referentialOfUUID( const std::string &uuid );
  static anatomist::Referential *acPcReferential();
  static anatomist::Referential *mniTemplateReferential();
};


class Transformation
{
%TypeHeaderCode
#include <anatomist/reference/Transformation.h>
%End

public:
  Transformation( anatomist::Referential *, anatomist::Referential *,
                  bool regist=false, bool generated=false );
  Transformation( anatomist::Referential *, anatomist::Referential *,
                  const anatomist::Transformation &trans );
  virtual ~Transformation();

  Motion & motion();
  aims::Quaternion quaternion() const;
  void setQuaternion( const aims::Quaternion &q );
  Point3df translation() const;

  anatomist::Referential* source() const;
  anatomist::Referential* destination() const;
  void invert();
  void invertReferentials();
  bool isDirect() const;
  Point3df transform( const Point3df &pos ) const;
  void registerTrans();
  void unregisterTrans();
  bool isGenerated() const;
  // void setGenerated( bool x );
  SIP_PYTUPLE transformBoundingBox( const Point3df & pmin1,
                                    const Point3df & pmax1 );
%MethodCode
  Point3df bmin, bmax;
  sipCpp->transformBoundingBox( *a0, *a1, bmin, bmax );
  sipRes = PyTuple_New( 2 );
  PyTuple_SetItem( sipRes, 0,
    sipConvertFromInstance( new Point3df( bmin ),
      sipClass_AimsVector_FLOAT_3, Py_None ) );
  PyTuple_SetItem( sipRes, 1,
    sipConvertFromInstance( new Point3df( bmax ),
      sipClass_AimsVector_FLOAT_3, Py_None ) );
%End
};

};

