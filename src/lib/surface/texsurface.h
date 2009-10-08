/* This software and supporting documentation are distributed by
 *     Institut Federatif de Recherche 49
 *     CEA/NeuroSpin, Batiment 145,
 *     91191 Gif-sur-Yvette cedex
 *     France
 *
 * This software is governed by the CeCILL-B license under
 * French law and abiding by the rules of distribution of free software.
 * You can  use, modify and/or redistribute the software under the
 * terms of the CeCILL-B license as circulated by CEA, CNRS
 * and INRIA at the following URL "http://www.cecill.info".
 *
 * As a counterpart to the access to the source code and  rights to copy,
 * modify and redistribute granted by the license, users are provided only
 * with a limited warranty  and the software's author,  the holder of the
 * economic rights,  and the successive licensors  have only  limited
 * liability.
 *
 * In this respect, the user's attention is drawn to the risks associated
 * with loading,  using,  modifying and/or developing or reproducing the
 * software by the user in light of its specific status of free software,
 * that may mean  that it is complicated to manipulate,  and  that  also
 * therefore means  that it is reserved for developers  and  experienced
 * professionals having in-depth computer knowledge. Users are therefore
 * encouraged to load and test the software's suitability as regards their
 * requirements in conditions enabling the security of their systems and/or
 * data to be ensured and,  more generally, to use and operate it in the
 * same conditions as regards security.
 *
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL-B license and that you accept its terms.
 */


#ifndef ANA_SURFACE_TEXSURFACE_H
#define ANA_SURFACE_TEXSURFACE_H


#include <anatomist/mobject/glmobject.h>


namespace anatomist
{

  class ATexSurface;
  class const_ATexSurfaceIterator;


  ///	Textured surface iterator
  class ATexSurfaceIterator : public BaseIterator
  {
  public:
    friend class ATexSurface;
    ATexSurfaceIterator( AObject *, AObject *, int );
    ATexSurfaceIterator( carto::rc_ptr<AObject>, carto::rc_ptr<AObject>, int );
    ATexSurfaceIterator( const ATexSurfaceIterator & );
    virtual ~ATexSurfaceIterator();
    virtual BaseIterator *clone() const;
    virtual bool operator != ( const BaseIterator & ) const;
    virtual bool operator != ( const ATexSurfaceIterator & ) const;
    virtual bool operator != ( const const_ATexSurfaceIterator & ) const;
    virtual AObject *operator * () const;
    virtual ATexSurfaceIterator & operator ++ ();
    virtual ATexSurfaceIterator & operator -- ();

  protected:
    carto::rc_ptr<AObject>	_surf;
    carto::rc_ptr<AObject>	_tex;
    int 	_no;
  };


  ///	Textured surface const_iterator
  class const_ATexSurfaceIterator : public BaseIterator
  {
  public:
    friend class ATexSurface;
    const_ATexSurfaceIterator( AObject *, AObject *, int );
    const_ATexSurfaceIterator( carto::rc_ptr<AObject>, carto::rc_ptr<AObject>,
                               int );
    const_ATexSurfaceIterator( const const_ATexSurfaceIterator & );
    virtual ~const_ATexSurfaceIterator();
    virtual BaseIterator *clone() const;
    virtual bool operator != ( const BaseIterator & ) const;
    virtual bool operator != ( const const_ATexSurfaceIterator & ) const;
    virtual bool operator != ( const ATexSurfaceIterator & ) const;
    virtual AObject *operator * () const;
    virtual const_ATexSurfaceIterator & operator ++ ();
    virtual const_ATexSurfaceIterator & operator -- ();

  protected:
    carto::rc_ptr<AObject> 	_surf;
    carto::rc_ptr<AObject> 	_tex;
    int 	_no;
  };


/**	Fusion object merging a triangulation and a texture object */
class ATexSurface : public GLMObject
{
public:
  ATexSurface( AObject* surface, AObject* texture );
  virtual ~ATexSurface();

  virtual int MType() const { return( AObject::TEXSURFACE ); }

  virtual size_t size() const { return( 2 ); }
  virtual iterator begin();
  virtual const_iterator begin() const;
  virtual iterator end();
  virtual const_iterator end() const;
  virtual const_iterator find( const AObject * ) const;

  virtual bool CanRemove( AObject* ) { return( false ); }

  virtual void update(const Observable* observable, void* arg);

  virtual bool Is2DObject() { return _surf->Is2DObject(); }
  virtual bool Is3DObject() { return true; }

  virtual float MinT() const;
  virtual float MaxT() const;
  virtual bool boundingBox( Point3df & bmin, Point3df & bmax ) const;

  AObject* surface() { return( _surf.get() ); }
  const AObject* surface() const { return( _surf.get() ); }
  AObject* texture() { return( _tex.get() ); }
  const AObject* texture() const { return( _tex.get() ); }

  virtual Tree* optionTree() const;
  static Tree*	_optionTree;

protected:
  carto::rc_ptr<AObject>	_surf;
  carto::rc_ptr<AObject>	_tex;

private:
  ///	Disable edition
  virtual void insert( AObject * ) {}
  ///	Disable edition
  virtual void erase( iterator & ) {}
};


//	Code


inline bool ATexSurfaceIterator::operator != ( const BaseIterator & x ) const
{
  return( x.operator != ( *this ) );
}


inline bool 
const_ATexSurfaceIterator::operator != ( const BaseIterator & x ) const
{
  return( x.operator != ( *this ) );
}


inline bool
ATexSurfaceIterator::operator != ( const ATexSurfaceIterator & x ) const
{
  return( _surf != x._surf || _tex != x._tex || _no != x._no );
}


inline bool 
const_ATexSurfaceIterator::operator != 
( const const_ATexSurfaceIterator & x ) const
{
  return( _surf != x._surf || _tex != x._tex || _no != x._no );
}


inline bool 
const_ATexSurfaceIterator::operator != ( const ATexSurfaceIterator & ) const
{
  return(true);
}


inline bool
ATexSurfaceIterator::operator != ( const const_ATexSurfaceIterator & ) const
{
  return(true);
}


inline AObject * ATexSurfaceIterator::operator * () const
{
  AObject *res=NULL;
  if (_no == 0) res = _surf.get();
  if (_no == 1) res = _tex.get();
 
  return( res );
}

inline AObject * const_ATexSurfaceIterator::operator * () const
{
  AObject *res=NULL;
  if (_no == 0) res = _surf.get();
  if (_no == 1) res = _tex.get();
 
  return( res );
}

inline ATexSurfaceIterator & ATexSurfaceIterator::operator ++ ()
{
  ++_no;
  return( *this );
}


inline const_ATexSurfaceIterator & const_ATexSurfaceIterator::operator ++ ()
{
  ++_no;
  return( *this );
}


inline ATexSurfaceIterator & ATexSurfaceIterator::operator -- ()
{
  --_no;
  return( *this );
}


inline const_ATexSurfaceIterator & const_ATexSurfaceIterator::operator -- ()
{
  --_no;
  return( *this );
}



inline BaseIterator *ATexSurfaceIterator::clone() const
{
  return( new ATexSurfaceIterator( *this ) );
}


inline BaseIterator *const_ATexSurfaceIterator::clone() const
{
  return( new const_ATexSurfaceIterator( *this ) );
}

}


#endif
