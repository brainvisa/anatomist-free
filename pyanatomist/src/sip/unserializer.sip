

namespace anatomist
{

  class Unserializer
  {
%TypeHeaderCode
#include <anatomist/processor/unserializer.h>
%End

  public:
    Unserializer();
    virtual ~Unserializer ();

    int makeID(void *, const std::string &="");

    int id(void *, const std::string & = "") const;
%MethodCode
  try
  {
    sipRes = sipCpp->id(a0, *a1);
  }
  catch( std::exception & e )
  {
    sipIsErr = 1;
    PyErr_SetString( PyExc_RuntimeError, e.what() );
    sipRes = 0;
  }
%End


  };
};

