
namespace anatomist
{

class SelectFactory
{
%TypeHeaderCode
#include <anatomist/selection/selectFactory.h>
#if SIP_VERSION < 0x040700
#include "sipanatomistsipset_AObjectPtr.h"
#endif
#include <pyanatomist/setaobject.h>
%End

public:
  enum SelectMode { Normal = 0, Add = 1, Toggle = 2 };

  struct HColor
  {
    HColor();
    //HColor( float, float, float, float=1., bool=false );
    bool operator== ( const anatomist::SelectFactory::HColor & ) const;
    float r;
    float g;
    float b;
    float a;
    bool na;
  };

  virtual ~SelectFactory();

//const std::map<unsigned, set_AObjectPtr > & selected() const;
  SIP_PYOBJECT selected() const;
%MethodCode
  // just for sip3
  std::map<unsigned, std::set<anatomist::AObject *> > & s 
    = const_cast<std::map<unsigned, std::set<anatomist::AObject *> > & >
      ( sipCpp->selected() );
  PyObject	*l = PyDict_New();
  std::map<unsigned, std::set<anatomist::AObject *> >::iterator i, e = s.end();
  for( i=s.begin(); i!=e; ++i )
    if( PyDict_SetItem(l,PyInt_FromLong( i->first ), 
                         pyanatomistConvertFrom_set_anatomist_AObjectP
                         (&(i->second) ) ) 
        < 0 )
      {
        Py_DECREF(l);

        sipRes = 0;
        sipIsErr = 1;
        std::ostringstream msg;
        msg << "item " << i->first 
            << " could not be converted to a set of AObject";
        PyErr_SetString( PyExc_TypeError, msg.str().c_str() );
      }
  if( !sipIsErr )
    sipRes = l;
%End

  virtual void select( unsigned, const set_AObjectPtr &,
                       const anatomist::SelectFactory::HColor *=0 ) const;
  virtual void unselect( unsigned, 
                         const set_AObjectPtr & ) const;
  virtual void unselectAll( unsigned ) const;
  virtual void selectAll( anatomist::AWindow *, 
                          const anatomist::SelectFactory::HColor *=0 ) const;
  virtual void flip( unsigned, const set_AObjectPtr &,
                     const anatomist::SelectFactory::HColor *=0 ) const;
  virtual void select( anatomist::SelectFactory::SelectMode, unsigned, 
                       const set_AObjectPtr &,
                       const anatomist::SelectFactory::HColor *=0 ) const;
  virtual bool isSelected( unsigned, anatomist::AObject * ) const;
  virtual HColor highlightColor( anatomist::AObject * ) const;
  virtual void 
    setHighlightColor( anatomist::AObject *, 
                       const anatomist::SelectFactory::HColor *=0) const;
  virtual void refresh() const;
  virtual void handleSelectionMenu( anatomist::AWindow *, int, int, 
                                    const Tree *=0 );
  virtual void propagateSelection( unsigned ) const;
  virtual void remove( anatomist::AWindow * );
  virtual void removeFromThisWindow( anatomist::AWindow * );

  static anatomist::SelectFactory * factory();
};

};

