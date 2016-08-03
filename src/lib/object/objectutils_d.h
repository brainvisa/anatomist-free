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


#ifndef ANA_OBJECT_OBJECTUTILS_D_H
#define ANA_OBJECT_OBJECTUTILS_D_H


#include <anatomist/object/objectutils.h>
#include <anatomist/mobject/MObject.h>


namespace anatomist
{

  template <typename iterator>
  std::string ObjectUtils::catObjectNames(
    const iterator & begin, const iterator & end, size_t limit,
    const std::set<AObject *> & setobj )
  {
    iterator obj;
    std::string  cat_name;
    AObject::ParentList::const_iterator   ip, fp;
    bool first = true;

    std::set<AObject *> setobj2;
    const std::set<AObject *> *psetobj = &setobj;
    if( setobj.empty() )
    {
      for( obj=begin; obj!=end; ++obj )
        setobj2.insert( *obj );
      psetobj = &setobj2;
    }
    std::set<AObject *>::const_iterator soe = psetobj->end();

    for( obj=begin; obj!=end; ++obj )
    {
      for( ip=(*obj)->Parents().begin(), fp=(*obj)->Parents().end();
          ip!=fp && psetobj->find( *ip ) == soe; ++ip ) {}
      if( ip == fp )    // no parent in list
      {
        if( first )
          first = false;
        else
          cat_name += ", ";
        switch( (*obj)->type() )
        {
        case AObject::FUSION2D:
        case AObject::FUSION3D:
          {
            MObject::iterator       io, fo = ((MObject *) *obj)->end();
            cat_name += "(";
            io=((MObject *) *obj)->begin();
            if( !(*io)->name().empty() )
              cat_name += (*io)->name();
            else
              cat_name += "<unnamed>";
            for( ++io; io!=fo; ++io )
              {
                cat_name += ",";
                if( !(*io)->name().empty() )
                  cat_name += (*io)->name();
                else
                  cat_name += "<unnamed>";
              }
            cat_name += ")";
          }
          break;
        default:
          if( !(*obj)->name().empty() )
            cat_name += (*obj)->name();
          else
            cat_name += "<unnamed>";
          break;
        }
      }
      if( limit != 0 && cat_name.length() >= limit )
      {
        cat_name.replace( limit, cat_name.length(), " ..." );
        break;
      }
    }

    return cat_name;
  }

}

#endif

