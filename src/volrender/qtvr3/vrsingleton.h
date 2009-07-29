#ifndef _vrsingleton_h_
#define _vrsingleton_h_


namespace Vr
{


template < class T > class VrSingleton
{

  public:

    static T& instance() 
    { 

      static T _instance;
      return _instance; 

    }

};


}


#endif
