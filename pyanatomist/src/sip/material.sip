
namespace anatomist
{

class Material
{
%TypeHeaderCode
#include <anatomist/color/Material.h>
%End

public:
  Material();
  virtual ~Material();

  void set( const carto::GenericObject & );
  carto::Object genericDescription() const;
};

};

