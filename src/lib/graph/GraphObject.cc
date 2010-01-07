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


#include <anatomist/control/wControl.h>

#include <anatomist/graph/GraphObject.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/object/actions.h>
#include <anatomist/graph/Graph.h>
#include <anatomist/control/graphParams.h>
#include <anatomist/window/viewstate.h>
#include <aims/graph/graphmanip.h>
#include <graph/tree/tree.h>
#include <cartobase/stream/sstream.h>


using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;


AGraphObject::ShowType AGraphObject::_showType = AGraphObject::TRIANG;


AGraphObject::AGraphObject( GenericObject* go )
  : GLObjectList(), AttributedAObject(), _gobject( go )
{
  _type = GRAPHOBJECT;
  _material.SetDiffuse( 0., 0.5, 1., 1. );
}


AGraphObject::~AGraphObject()
{
  // cout << "~AGraphObject\n";
  cleanup();
  datatype::iterator	it;
  set<AObject *>	toDelete;

  for( it=_data.begin(); it!=_data.end(); ++it )
  {
    (*it)->UnregisterParent( this );
    (*it)->deleteObserver((Observer*)this);
  }

  _data.erase( _data.begin(), _data.end() );
}


GLPrimitives AGraphObject::glMainGLL( const ViewState & vs )
{
  GLPrimitives  glp;
  const_iterator i, j = end();
  bool          done = false, show, shown = false;
  GLComponent   *glc, *first = 0;
  bool          twod = ( vs.sliceVS() != 0 );

  for( i=begin(); !done && i!=j; ++i )
    if( ( glc = (*i)->glAPI() ) )
  {
    if( !first )
      first = glc;
    show = false;
    switch( _showType )
    {
      case FIRST:
        if( !twod || (*i)->Is2DObject() )
        {
          done = true;
          show = true;
        }
        break;
      case ALL:
        if( !twod || (*i)->Is2DObject() )
          show = true;
        break;
      case TRIANG:
        if( twod )
        {
          if( (*i)->Is2DObject() )
            show = true;
        }
        else if( (*i)->Is3DObject() && ! (*i)->Is2DObject() )
          show = true;
        break;
      case BUCKET:
        if( (*i)->Is2DObject() )
          show = true;
        break;
    }

    if( show )
    {
      GLPrimitives  p = glc->glMainGLL( vs );
      glp.insert( glp.end(), p.begin(), p.end() );
      shown = true;
    }
  }
  // if no corresponding object, show first of all
  if( !shown )
  {
    if( _showType == FIRST && first )
    {
      glp = first->glMainGLL( vs );
    }
    else // show all
    {
      for( i=begin(); i!=j; ++i )
        if( ( glc = (*i)->glAPI() ) )
        {
          GLPrimitives  p = glc->glMainGLL( vs );
          glp.insert( glp.end(), p.begin(), p.end() );
        }
    }
  }

  return glp;
}


const Material & AGraphObject::material() const
{
  return AObject::material();
}


Material & AGraphObject::GetMaterial()
{
  return AObject::GetMaterial();
}


void AGraphObject::SetMaterial( const Material & mat )
{
  /* cout << "AGraphObject::SetMaterial " << this << ", " << name() << ": "
       << mat.Diffuse(0) << ", "
       << mat.Diffuse(1) << ", " << mat.Diffuse(2) << ", "
       << mat.Diffuse(3) << endl; */

  glSetChanged( glMATERIAL );
  ParentList::iterator	ip, fp=_parents.end();
  AGraph		*ag;

  for( ip=_parents.begin(); ip!=fp; ++ip )
    {
      ag = dynamic_cast<AGraph *>( *ip );
      if( ag )
        {
	  const Material & pmat = ag->GetMaterial();
	  if( mat == pmat )	// if graph "default" material
	    {			// enable own default materials
	      SetMaterialOrDefault( ag, mat );
	      return;
	    }
	}
    }
  // cout << "hard-coded color\n";
  GLObjectList::SetMaterial( mat );
}


void AGraphObject::SetMaterialOrDefault( const AGraph* agr, 
					 const Material & mat )
{
  /* cout << "setMatOrDefault " << mat.Diffuse(0) << ", "
      << mat.Diffuse(1) << ", " << mat.Diffuse(2) << ", "
      << mat.Diffuse(3) << endl; */

  // see if there are some default colors
  const rc_ptr<map<string, vector<int> > > cols = agr->objAttColors();

  if( cols->size() == 0 || !GraphParams::graphParams()->colorsActive )
    {
      GLObjectList::SetMaterial( mat );
      return;
    }
  AObject::SetMaterial( mat );

  map<AObject *, string>				objmap;
  map<AObject *, string>::const_iterator		iom, eom;
  iterator						io, fo=end();
  set<AObject *>					nocol;
  set<AObject *>::iterator				is, fs=nocol.end();
  map<string, vector<int> >::const_iterator		ic, ec = cols->end();
  bool							hascol;
  AObject						*o;
  Material						mat2 = mat;
  rc_ptr<map<string, map<string,GraphElementCode> > >	codes;
  map<string,GraphElementCode>				*code = 0;
  map<string,GraphElementCode>::iterator		icode, ecode;

  _gobject->getProperty( "objects_map", objmap );
  eom = objmap.end();
  if( agr->graph()->getProperty( "aims_objects_table", codes ) )
    {
      SyntaxedInterface	*si = attributed()->getInterface<SyntaxedInterface>();
      string		snt;
      if( si && si->hasSyntax() )
        snt = si->getSyntax();
      else
        snt = attributed()->type();
      map<string, map<string,GraphElementCode> >::iterator 
	ic2 = codes->find( snt );
      if( ic2 != codes->end() )
	code = &ic2->second;
    }
  if( !code )
    {
      GLObjectList::SetMaterial( mat );
      return;
    }

  ecode = code->end();

  for( io=begin(); io!=fo; ++io )
    {
      o = *io;
      iom = objmap.find( o );
      hascol = false;
      if( iom != eom )
	{
	  icode = code->find( iom->second );
	  if( icode != ecode )
	    {
	      GraphElementCode	& gec = icode->second;
	      ic = cols->find( gec.global_index_attribute );
	      if( ic == ec )
		ic = cols->find( gec.local_file_attribute );
/*              if( ic == ec
                  && gec.global_index_attribute.substr( 0, 5 ) == "aims_" )
                ic = cols->find( gec.global_index_attribute.substr( 5,
                                 gec.global_index_attribute.length() - 5 ) );
              if( ic == ec
                  && gec.local_file_attribute.substr( 0, 5 ) == "aims_" )
                ic = cols->find( gec.local_file_attribute.substr( 5,
                                 gec.local_file_attribute.length() - 5 ) );
              */
              if( ic != ec )
		{
		  hascol = true;
		  const vector<int>	& col = (*ic).second;
                  float	da = mat.Diffuse( 3 );
                  if( col.size() >= 4 )
                    da = ((float) col[3]) / 255;
		  mat2.SetDiffuse( ((float) col[0]) / 255,
				   ((float) col[1]) / 255, 
				   ((float) col[2]) / 255, da );
		  o->SetMaterial( mat2 );
		}
	    }
	}
      if( !hascol )
	nocol.insert( o );
    }

  // set remaining sub-objects materials using material mat
  for( is=nocol.begin(); is!=fs; ++is )
    (*is)->SetMaterial( mat );
}


void AGraphObject::internalUpdate()
{
  string	newname;

  if( attributed()->getProperty( "name", newname ) )
    {
      string		oldname = name();

      setName( "tmp" );
      string		name = theAnatomist->makeObjectName( newname );
      setName( name.c_str() );
      if( oldname != name )
	{
	  if( theAnatomist->getControlWindow() )
	    theAnatomist->getControlWindow()->NotifyObjectChange(this);
	}
    }
  

  if( AGraph::specialColorFunc )
    {
      ParentList::const_iterator	ip, fp=Parents().end();
      AGraph			*ag;

      for( ip=Parents().begin(); ip!=fp; ++ip )
	{
	  ag = dynamic_cast<AGraph *>( *ip );
	  if( ag )
	    {
	      Material	mat2 = _material;
	      bool	changed = AGraph::specialColorFunc( ag, this, mat2 );
	      if( changed )
		SetMaterial( mat2 );
	      else
		SetMaterialOrDefault( ag, ag->GetMaterial() );
	      MObject::internalUpdate();
	      return;
	    }
	}
    }
  MObject::internalUpdate();
}

#include <cartobase/object/object_d.h>
#define _TMP_ std::map<anatomist::AObject *, std::string>
INSTANTIATE_GENERIC_OBJECT_TYPE( _TMP_ )
#undef _TMP_

