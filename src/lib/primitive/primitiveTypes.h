#ifndef PRIMITIVE_TYPES_H
#define PRIMITIVE_TYPES_H


#include <list>
#include <cartobase/smart/rcobject.h>

namespace anatomist
{
  class GLItem;
  class Primitive;
  class GLList;
  class GLTexture;
  class GLItemList;

  typedef carto::rc_ptr<GLItem>		RefGLItem;
  typedef carto::rc_ptr<Primitive>	RefPrimitive;
  typedef carto::rc_ptr<GLList>		RefGLList;
  typedef carto::rc_ptr<GLTexture>	RefGLTexture;
  typedef carto::rc_ptr<GLItemList>	RefGLItemList;
  typedef std::list<RefGLItem>		GLPrimitives;
}

#endif // PRIMITIVE_TYPES_H