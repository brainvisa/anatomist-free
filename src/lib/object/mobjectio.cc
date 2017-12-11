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

#include <anatomist/object/mobjectio.h>
#include <anatomist/fusion/fusionFactory.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/graph/attribAObject.h>
#include <anatomist/processor/Processor.h>
#include <anatomist/processor/context.h>
#include <anatomist/commands/cFusionObjects.h>
#include <anatomist/mobject/MObject.h>
#include <anatomist/surface/textobject.h>
#include <anatomist/surface/transformedobject.h>
#include <aims/io/writer.h>
#include <cartobase/stream/fileutil.h>

using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;


namespace
{

  string getFusionMethod( const string & otype,
                          const list<AObject *> & aobjects )
  {
    // TODO: move this map somewhere else
    static map<string, string> type_to_method;
//     if( type_to_method.empty() )
//     {
//       type_to_method[] = ;
//     }

    map<string, string>::const_iterator it = type_to_method.find( otype );
    if( it != type_to_method.end() )
    {
      // hard coded in table
      return it->second;
    }

    // look for a compatible method

    FusionFactory *ffac = FusionFactory::factory();
    multimap<int, string> methods = ffac->allowedMethods( set<AObject *>(
      aobjects.begin(), aobjects.end() ) );
    if( methods.empty() )
        return "";
    if( methods.size() == 1 )
        return methods.begin()->second; // OK, not ambiguous
    // check fusion type
    list<string> types;
    multimap<int, string>::const_iterator im, em = methods.end();
    bool found = false;
    string meth_name;
    for( im=methods.begin(); im!=em; ++im )
    {
      string gtype = ffac->method( im->second )->generatedObjectType();
      if( gtype == otype )
      {
        if( found )
          return ""; // 2 or more matching fusion types
        found = true;
        meth_name = im->second;
      }
    }
    if( found )
      return meth_name;
    // not found.
    return "";
  }


  Object make_obj( Object obj, const string & id, bool return_id )
  {
    if( return_id )
    {
      Object robj = Object::value( carto::ObjectVector() );
      carto::ObjectVector & vo = robj->value<carto::ObjectVector>();
      vo.push_back( obj );
      vo.push_back( Object::value( id ) );
      return robj;
    }
    else
      return obj;
  }

  map<string, Object> getCommands()
  {
    static map<string, Object> commands;
    if( commands.empty() )
    {
      Object c = Object::value( Dictionary() );
      c->setProperty( "name", "TexturingParams" );
      c->setProperty( "objects", "objects" );
      c->setProperty( "objects_list", true );
      commands[ "texturing_params" ] = c;
      c = Object::value( Dictionary() );
      c->setProperty( "name", "Fusion3DParams" );
      c->setProperty( "objects", "object" );
      c->setProperty( "objects_list", false );
      commands[ "fusion3dparams" ] = c;
    }
    return commands;
  }


  string objectId( AObject* aobj, map<AObject *, string> & obj_map )
  {
    string obj_id = aobj->name();
    set<string> values;
    // build reverse map (could be optimized externally)
    map<AObject *, string>::const_iterator iom, eom = obj_map.end();
    for( iom=obj_map.begin(); iom!=eom; ++iom )
      values.insert( iom->second );

    while( values.find( obj_id ) != values.end() )
        obj_id = obj_id + " (2)"; // FIXME NOT GOOD
    return obj_id;
  }


  bool objectHasNativeSave( AObject* aobj )
  {
    // WARNING: non-portable
    // http://stackoverflow.com/questions/4741271/ways-to-detect-whether-a-c-virtual-function-has-been-redefined-in-a-derived-cl
//     AObject &obj = *aobj;
//     return reinterpret_cast<void*>(obj.*(&AObject::save))
//       != reinterpret_cast<void*>(&AObject::save);
    return aobj->savable();
  }


  Object objectFromFactory( const string & otype,
                            const list<AObject *> children, Object properties )
  {
//     cout << "objectFromFactory " << otype << endl;
    Object obj;
    if( otype == "TextObject" )
    {
      string text = "<no text>";
      Point3df pos( 0.f, 0.f, 0.f );
      float fsize = 30., scale = 1.;
      Object material;
      if( properties.get() )
      {
        try
        {
          text = properties->getProperty( "text" )->getString();
        }
        catch( ... )
        {
        }
        try
        {
          Object p = properties->getProperty( "position" );
          if( p && p->size() >= 3 )
          {
            Object pit = p->objectIterator();
            unsigned i;
            for( i=0; i < 3 && pit->isValid(); ++i, pit->next() )
              pos[i] = float( pit->currentValue()->getScalar() );
          }
        }
        catch( ... )
        {
        }
        try
        {
          fsize = float( properties->getProperty( "font_size" )->getScalar() );
        }
        catch( ... )
        {
        }
        try
        {
          scale = float( properties->getProperty( "scale" )->getScalar() );
        }
        catch( ... )
        {
        }
        try
        {
          material = properties->getProperty( "material" );
        }
        catch( ... )
        {
        }
      }
      TextObject *to = new TextObject( text, pos, fsize );
      to->setScale( scale );
      if( !material.isNull() )
        to->attributed()->setProperty( "material", material );
      to->setHeaderOptions();
      obj = Object::value( static_cast<AObject *>( to ) );
    }
    else if( otype == "TransformedObject" )
    {
      vector<carto::shared_ptr<AObject> > children_vec;
      children_vec.reserve( children.size() );
      bool follow_orientation = false, follow_position = true;
      bool use_dyn_offset = false;
      Point3df pos( 0.f, 0.f, 0.f ), dynoffset( 0.f, 0.f, 0.f );
      Point3df offset( 0.f, 0.f, 0.f );
      float scale = 1.f;
      list<AObject *>::const_iterator ic, ec = children.end();
      for( ic=children.begin(); ic!=ec; ++ic )
        children_vec.push_back( rc_ptr<AObject>( *ic ) );

      if( properties.get() )
      {
        try
        {
          follow_orientation = bool(
            properties->getProperty( "follow_orientation" )->getScalar() );
        }
        catch( ... )
        {
        }
        try
        {
          follow_position = bool(
            properties->getProperty( "follow_position" )->getScalar() );
        }
        catch( ... )
        {
        }
        try
        {
          Object p = properties->getProperty( "position" );
          if( p && p->size() >= 3 )
          {
            Object pit = p->objectIterator();
            unsigned i;
            for( i=0; i < 3 && pit->isValid(); ++i, pit->next() )
              pos[i] = float( pit->currentValue()->getScalar() );
          }
        }
        catch( ... )
        {
        }
        try
        {
          Object p = properties->getProperty( "dynamic_offset_from_point" );
          if( p && p->size() >= 3 )
          {
            Object pit = p->objectIterator();
            unsigned i;
            for( i=0; i < 3 && pit->isValid(); ++i, pit->next() )
              dynoffset[i] = float( pit->currentValue()->getScalar() );
            use_dyn_offset = true;
          }
        }
        catch( ... )
        {
        }
        try
        {
          Object p = properties->getProperty( "offset" );
          if( p && p->size() >= 3 )
          {
            Object pit = p->objectIterator();
            unsigned i;
            for( i=0; i < 3 && pit->isValid(); ++i, pit->next() )
              offset[i] = float( pit->currentValue()->getScalar() );
          }
        }
        catch( ... )
        {
        }
        try
        {
          scale = float( properties->getProperty( "scale" )->getScalar() );
        }
        catch( ... )
        {
        }
      }

      TransformedObject *tro
        = new TransformedObject( children_vec, follow_orientation,
                                 follow_position, pos );
      tro->setScale( scale );
      tro->setOffset( offset );
      if( use_dyn_offset )
        tro->setDynamicOffsetFromPoint( dynoffset );
      obj = Object::value( static_cast<AObject *>( tro ) );
    }
    return obj;
  }

}


Object MObjectIO::readMObject( const std::string & filename )
{
  Reader<GenericObject> ro( filename );
  Object aobjects( ro.read() );
  return readMObject( aobjects, FileUtil::dirname( filename ) );
}


Object MObjectIO::readMObject( Object object_descr, const string & path,
                               map<string, Object> *pobj_map,
                               bool return_id )
{
//   cout << "readMObject\n";
  rc_ptr<map<string, Object> > robj_map;
  if( !pobj_map )
    robj_map.reset( new map<string, Object> );
  else
    robj_map.reset( pobj_map );
  map<string, Object> & obj_map = *robj_map;
  if( pobj_map )
    robj_map.release();

  string obj_id;
  Object obj_desc;
  if( object_descr->isString() )
  {
    obj_id = object_descr->getString();
    obj_desc = object_descr;
  }
//   else if( object_descr->isArray() && object_descr->size() == 2 )
//   {
//     // 2-tuple case. Ambiguous with list of 2 objects
//   }
  else
    obj_desc = object_descr;

  if( !obj_id.empty() )
  {
    map<string, Object>::const_iterator iom = obj_map.find( obj_id );
    if( iom != obj_map.end() )
      // reuse existing object
      return make_obj( iom->second, obj_id, return_id );
  }

  if( obj_desc->isString() )
  {
    // read sub_object
    string fpath = obj_desc->getString();
    if( !path.empty() && !FileUtil::isAbsPath( fpath ) )
      fpath = path + FileUtil::separator() + fpath;
    list<AObject *> aobj = theAnatomist->loadObject( fpath );
    if( aobj.empty() )
      throw io_error( "Could not read object in file " + fpath );
    Object robj;
    if( aobj.size() == 1 )
      robj = Object::value( *aobj.begin() );
    else
    {
      robj = Object::value( carto::ObjectVector() );
      vector<Object> & vo = robj->value<carto::ObjectVector>();
      vo.reserve( aobj.size() );
      list<AObject *>::iterator io, eo=aobj.end();
      for( io=aobj.begin(); io!=eo; ++io )
        vo.push_back( Object::value( *io ) );
    }
    return make_obj( robj, obj_id, return_id );
  }

  Object properties = carto::none();
  string obj_name;
  Object objects;
  string otype, fmethod;

  if( !obj_desc->isString() && obj_desc->isArray() )
  {
    // list
    objects = obj_desc;
    // otype = ""; fmethod = "";
  }
  else
  {
    // dict
    try
    {
      otype = obj_desc->getProperty( "object_type" )->getString();
    }
    catch( ... )
    {
    }
    try
    {
      fmethod = obj_desc->getProperty( "fusion" )->getString();
    }
    catch( ... )
    {
    }
    try
    {
      objects = obj_desc->getProperty( "objects" );
    }
    catch( ... )
    {
    }
    if( obj_id.empty() )
      try
      {
        obj_id = obj_desc->getProperty( "identifier" )->getString();
      }
      catch( ... )
      {
      }
    try
    {
      properties = obj_desc->getProperty( "properties" );
    }
    catch( ... )
    {
    }
    try
    {
      obj_name = obj_desc->getProperty( "name" )->getString();
    }
    catch( ... )
    {
    }
  }

//   cout << "id: " << obj_id << ", otype: " << otype << ", fmethod: " << fmethod << ", name: " << obj_name << endl;

  list<AObject *> aobjects;

  if( !objects.isNull() && !objects->isString() && objects->isArray() )
  {
    Object oit = objects->objectIterator();
    for( ; oit->isValid(); oit->next() )
    {
      Object robj = readMObject( oit->currentValue(), path, &obj_map, true );
      Object aobj = robj->getArrayItem( 0 );
      string sub_id = robj->getArrayItem( 1 )->getString();
      if( !sub_id.empty() )
        obj_map[ sub_id ] = aobj;
      if( aobj->isArray() )
      {
        Object lit = aobj->objectIterator();
        for( ; lit->isValid(); lit->next() )
          aobjects.push_back( lit->value<AObject *>() );
      }
      else
      {
        aobjects.push_back( aobj->value<AObject *>() );
      }
    }
  }
  else if( !objects.isNull() )
  {
    Object robj = readMObject( objects, path, &obj_map, true );
    Object aobj = robj->getArrayItem( 0 );
    string sub_id = robj->getArrayItem( 1 )->getString();
    if( !sub_id.empty() )
      obj_map[ sub_id ] = aobj;
    if( !obj_id.empty() )
      obj_map[ obj_id ] = aobj;
    if( aobj->isArray() )
    {
      Object lit = aobj->objectIterator();
      for( ; lit->isValid(); lit->next() )
        aobjects.push_back( lit->value<AObject *>() );
    }
    else
    {
      AObject *aaobj = aobj->value<AObject *>();
      aobjects.push_back( aaobj );
      if( !properties.isNull() )
      {
        AttributedAObject *aaaobj = dynamic_cast<AttributedAObject *>( aaobj );
        if( aaaobj )
        {
          aaaobj->attributed()->copyProperties( properties );
          aaobj->setHeaderOptions();
        }
        else
          aaobj->setProperties( properties );
      }
    }
  }

  Object mobj;

  if( objects.isNull() )
  {
    mobj = objectFromFactory( otype, aobjects, properties );
  }
  else if( !objects.isNull() && !objects->isString() && objects->isArray() )
  {
    if( ( otype.empty() && fmethod.empty() ) || otype == "list" )
    {
      // raw list
      mobj = Object::value( vector<Object>() );
      vector<Object> & lobj = mobj->value<vector<Object> >();
      list<AObject *>::const_iterator iao, eao = aobjects.end();
      for( iao=aobjects.begin(); iao!=eao; ++iao )
        lobj.push_back( Object::value( *iao ) );
    }
    else
    {
      // fusion them
      if( fmethod.empty() )
        fmethod = getFusionMethod( otype, aobjects );
      if( fmethod.empty() )
      {
        mobj = objectFromFactory( otype, aobjects, properties );
      }
      else
      {
        list<AObject *>::const_iterator iao, eao = aobjects.end();
        for( iao=aobjects.begin(); iao!=eao; ++iao )
          theAnatomist->unmapObject( *iao ); // remove from ctrlwin
        FusionObjectsCommand *c
          = new FusionObjectsCommand( vector<AObject *>(aobjects.begin(),
                                                        aobjects.end() ),
                                      fmethod, -1, false );
        bool allowidle = theProcessor->execWhileIdle();
        theProcessor->allowExecWhileIdle( true );
        theProcessor->execute( c );
        AObject *fobj = c->createdObject();
  //       for( iao=aobjects.begin(); iao!=eao; ++iao )
  //         theAnatomist->releaseObject( *iao ); // release app ref
        mobj = Object::value( fobj );
        if( !properties.isNull() )
        {
          AttributedAObject *aaaobj
            = dynamic_cast<AttributedAObject *>( fobj );
          if( aaaobj )
          {
            aaaobj->attributed()->copyProperties( properties );
            fobj->setHeaderOptions();
          }
          else
            fobj->setProperties( properties );
          Object pit = properties->objectIterator();
          const map<string, Object> & commands = getCommands();
          map<string, Object>::const_iterator cit, ecit = commands.end();
          for( ; pit->isValid(); pit->next() )
          {
            cit = commands.find( pit->key() );
            if( cit != ecit )
            {
              Object params = Object::value( Dictionary() );
              params->copyProperties( pit->currentValue() );
              string cname, cobjects;
              bool c_islist = false;
              cit->second->getProperty( "name", cname );
              cit->second->getProperty( "objects", cobjects );
              cit->second->getProperty( "objects_list", c_islist );
              rc_ptr<Unserializer> ser
                = CommandContext::defaultContext().unserial;
              int objnum = ser->makeID( fobj, "AObject" );
              if( c_islist )
                params->setProperty( cobjects, vector<int>( 1, objnum ) );
              else
                params->setProperty( cobjects, objnum );
              theProcessor->execute( cname, params );
            }
          }
        }
        theProcessor->allowExecWhileIdle( allowidle );
      }
    }
  }
  else
  {
    mobj = Object::value( *aobjects.begin() );
  }

  if( !obj_name.empty() && !mobj.isNull() && !mobj->isArray() )
  {
    AObject *obj = mobj->value<AObject *>();
    obj->setName( theAnatomist->makeObjectName( obj_name ) );
    theAnatomist->NotifyObjectChange( obj );
  }

  return make_obj( mobj, obj_id, return_id );
}


Object MObjectIO::createMObjectDescr( Object aobject,
                                      const std::string & path,
                                      bool writeLeafs )
{
  if( aobject->isArray() )
  {
    Object objects = Object::value( carto::ObjectVector() );
    carto::ObjectVector & ov = objects->value<carto::ObjectVector>();

    map<AObject*, string> obj_map;
    Object it = aobject->objectIterator();
    for( ; it->isValid(); it->next() )
    {
      Object item = createMObjectDescr( it->currentValue()->value<AObject *>(),
                                        path, writeLeafs, &obj_map );
      ov.push_back( item );
    }
    return objects;
  }
  else
  {
    AObject *aobj = aobject->value<AObject *>();
    return createMObjectDescr( aobj, path, writeLeafs );
  }
}


Object MObjectIO::createMObjectDescr( AObject* aobject, const string & path,
                                      bool writeLeafs,
                                      map<AObject*, string> *pobj_map )
{
  rc_ptr<map<AObject*, string> > robj_map;
  if( !pobj_map )
    robj_map.reset( new map<AObject*, string> );
  else
    robj_map.reset( pobj_map );
  map<AObject*, string> & obj_map = *robj_map;
  if( pobj_map )
    robj_map.release();

  map<AObject*, string>::iterator iom = obj_map.find( aobject );
  if( iom != obj_map.end() )
    return Object::value( iom->second );

  Object objects;

  string otype = aobject->objectFullTypeName();
  string ftype;
  list<AObject *> children;
  if( dynamic_cast<MObject *>( aobject ) )
  {
    MObject *mobj = static_cast<MObject *>( aobject );
    children = mobj->generativeChildren();
    ftype = getFusionMethod( otype, children );
  }
  if( !ftype.empty() )
  {
    objects = Object::value( Dictionary() );
    objects->setProperty( "fusion", ftype );
    objects->setProperty( "name", aobject->name() );
    objects->setProperty( "identifier", objectId( aobject, obj_map ) );
    Object props = aobject->makeHeaderOptions();
    if( props )
      objects->setProperty( "properties", props );
    carto::ObjectVector sub_obj;
    sub_obj.reserve( children.size() );
    list<AObject *>::iterator ic, ec = children.end();
    for( ic=children.begin(); ic!=ec; ++ic )
    {
      Object sobj = createMObjectDescr( *ic, path, writeLeafs, &obj_map );
      sub_obj.push_back( sobj );
      if( obj_map.find( *ic ) == obj_map.end() )
      {
        obj_map[ *ic ] = objectId( *ic, obj_map );
      }
    }
    objects->setProperty( "objects", sub_obj );
  }
  else if( objectHasNativeSave( aobject ) )
  {
    string filename = aobject->fileName();
    string bname = FileUtil::basename( filename );
    string idname = filename;
    if( bname.empty() )
    {
      bname = aobject->name();
      filename = FileUtil::dirname( filename ) + FileUtil::separator() + bname;
    }
    if( !path.empty() )
    {
      if( filename.substr( 0, path.length() + 1 )
          == path + FileUtil::separator() )
        idname = filename.substr( path.length() + 1, filename.length() );
      else
      {
        filename = path + FileUtil::separator() + bname;
        idname = bname;
      }
    }
    cout << "save filename: " << filename << ", id: " << idname << endl;
    if( writeLeafs )
    {
      // FIXME: should maybe not be done here
      // FIXME: sometimes needs file extension / format
      aobject->save( filename );
    }
    string obj_id = idname;
    obj_map[ aobject ] = obj_id;
    objects = Object::value( Dictionary() );
    objects->setProperty( "objects", idname );
    objects->setProperty( "identifier", obj_id );
    objects->setProperty( "name", aobject->name() );
    Object props = aobject->makeHeaderOptions();
    if( props )
      objects->setProperty( "properties", props );
  }
  else if( dynamic_cast<TransformedObject *>( aobject ) )
  {
    TransformedObject *tro = static_cast<TransformedObject *>( aobject );
    objects = Object::value( Dictionary() );
    objects->setProperty( "object_type", "TransformedObject" );
    objects->setProperty( "name", aobject->name() );
    objects->setProperty( "identifier", objectId( aobject, obj_map ) );
    Object props = aobject->makeHeaderOptions();
    if( !props )
      props = Object::value( Dictionary() );
    props->setProperty( "position", tro->position().toStdVector() );
    props->setProperty( "scale", tro->scale() );
    props->setProperty( "follow_orientation", tro->followsOrientation() );
    props->setProperty( "follow_position", tro->followsPosition() );
    if( tro->usesDynamicOffset() )
      props->setProperty( "dynamic_offset_from_point",
                          tro->dynamicOffsetFromPoint().toStdVector() );
    props->setProperty( "offset", tro->offset().toStdVector() );
    objects->setProperty( "properties", props );

    carto::ObjectVector sub_obj;
    sub_obj.reserve( tro->size() );

    MObject::iterator it, et = tro->end();
    for( it=tro->begin(); it!=et; ++it )
    {
      Object item = createMObjectDescr( *it, path, writeLeafs, &obj_map );
      sub_obj.push_back( item );
    }
    objects->setProperty( "objects", sub_obj );
  }
  else if( dynamic_cast<TextObject *>( aobject ) )
  {
    TextObject *to = static_cast<TextObject *>( aobject );
    objects = Object::value( Dictionary() );
    objects->setProperty( "object_type", "TextObject" );
    objects->setProperty( "name", aobject->name() );
    objects->setProperty( "identifier", objectId( aobject, obj_map ) );
    Object props = aobject->makeHeaderOptions();
    if( !props )
      props = Object::value( Dictionary() );
    props->setProperty( "text", to->text() );
    props->setProperty( "position", to->position().toStdVector() );
    props->setProperty( "scale", to->scale() );
    props->setProperty( "font_size", to->fontSize() );
    objects->setProperty( "properties", props );
  }
  else
  {
    throw runtime_error( string( "Cannot serialize object type " ) + otype );
  }

  return objects;
}



bool MObjectIO::writeMObject( AObject* aobject, const string & filename,
                              bool writeLeafs )
{
  Object objects = createMObjectDescr( aobject, FileUtil::dirname( filename ),
                                       writeLeafs );
  return saveDescription( objects, filename );
}


bool MObjectIO::writeMObject( Object aobject, const string & filename,
                              bool writeLeafs )
{
  Object objects = createMObjectDescr( aobject, FileUtil::dirname( filename ),
                                       writeLeafs );
  return saveDescription( objects, filename );
}


bool MObjectIO::saveDescription( Object objects, const string & filename,
                                 bool writeLeafs )
{
  if( objects )
  {
    Writer<Object> wo( filename );
    string format = "JSON";
    wo.write( objects, true, &format );
    return true;
  }
  return false;
}

