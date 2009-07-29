#ifndef _creator_h_
#define _creator_h_


namespace Vr
{


template < class T, class U >
struct Creator
{

  static U* create()
  { 

    return new T; 

  }

};


}


#endif
