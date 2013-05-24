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

#include <anatomist/browser/attDescr.h>
#include <anatomist/browser/qObjBrowserWid.h>
#include <anatomist/control/coloredpixmap.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/application/globalConfig.h>
#include <aims/listview/editablelistviewitem.h>
#include <cartobase/object/attributed.h>
#include <cartobase/object/pythonwriter.h>
#include <graph/tree/tree.h>
#include <QTreeWidgetItem>
#include <stdio.h>
#include <vector>
#include <iostream>

using namespace anatomist;
using namespace carto;
using namespace std;

// define this to enabl editable listview items.
// change it also in qwObjectBrowser.cc
// #define ANA_USE_EDITABLE_LISTVIEWITEMS

AttDescr* AttDescr::_theAttDescr = 0;


AttDescr::AttDescr()
{
  delete _theAttDescr;
  _theAttDescr = 0;
  initHelpers();
  _theAttDescr = this;

  // base syntax
  Syntax	& s = syntax[ "__default__" ];
  s[ "aims_objects_table"         ] = Semantic( "internal_type", false, true );
  s[ "aims_reader_loaded_objects" ] = Semantic( "internal_type", false, true );
  s[ "object_attributes_colors"   ] = Semantic( "internal_type", false, true );
  s[ "ana_object"                 ] = Semantic( "internal_type", false, true );
  s[ "objects_map"                ] = Semantic( "internal_type", false, true );
  s[ "aims_ss"                    ] = Semantic( "internal_type", false, true );
  s[ "aims_ss_ana"                ] = Semantic( "internal_type", false, true );
  s[ "aims_bottom"                ] = Semantic( "internal_type", false, true );
  s[ "aims_bottom_ana"            ] = Semantic( "internal_type", false, true );
  s[ "aims_other"                 ] = Semantic( "internal_type", false, true );
  s[ "aims_other_ana"             ] = Semantic( "internal_type", false, true );
  s[ "aims_Tmtktri"               ] = Semantic( "internal_type", false, true );
  s[ "aims_Tmtktri_ana"           ] = Semantic( "internal_type", false, true );
  s[ "aims_junction"              ] = Semantic( "internal_type", false, true );
  s[ "aims_junction_ana"          ] = Semantic( "internal_type", false, true );
  s[ "aims_cortical"              ] = Semantic( "internal_type", false, true );
  s[ "aims_cortical_ana"          ] = Semantic( "internal_type", false, true );
  s[ "aims_plidepassage"          ] = Semantic( "internal_type", false, true );
  s[ "aims_plidepassage_ana"      ] = Semantic( "internal_type", false, true );
  s[ "aims_roi"                   ] = Semantic( "internal_type", false, true );
  s[ "aims_roi_ana"               ] = Semantic( "internal_type", false, true );
  s[ "aims_cluster"               ] = Semantic( "internal_type", false, true );
  s[ "aims_cluster_ana"           ] = Semantic( "internal_type", false, true );
  s[ "cliques"                    ] = Semantic( "internal_type", false, true );
  s[ "possible_labels"            ] = Semantic( "internal_type", false, true );
}


AttDescr::~AttDescr()
{
  _theAttDescr = 0;
}


void AttDescr::setSyntax( const SyntaxSet & s )
{
  syntax = s;
}


void AttDescr::addSyntax( const SyntaxSet & s )
{
  //cout << "addSyntax : " << syntax.size() << endl;
  SyntaxSet::const_iterator	is, es = s.end();
  for( is=s.begin(); is!=es; ++is )
    syntax.insert( *is );
  //cout << "apr� ajout : " << syntax.size() << endl;
}


QPixmap AttDescr::rgbPixmap( const QColor & rgb )
{
  return ColoredPixmapCache::coloredPixmap( rgb, 16, "ball.png",
                                            "ball_refl.png" );
}


void AttDescr::setHelpers( const HelperSet & help )
{
  initHelpers();
  HelperSet::const_iterator	ih, eh = help.end();
  for( ih=help.begin(); ih!=eh; ++ih )
    helpers.insert( *ih );
}


void AttDescr::addHelpers( const HelperSet & help )
{
  HelperSet::const_iterator	ih, eh = help.end();
  for( ih=help.begin(); ih!=eh; ++ih )
    helpers.insert( *ih );
}


void AttDescr::setListHelpers( const ListHelperSet & help )
{
  listHelpers = help;

  if( listHelpers.find( "tree" ) == listHelpers.end() )
    listHelpers[ "tree" ] = &treeListHelper;
  if( listHelpers.find( "list" ) == listHelpers.end() )
    listHelpers[ "list" ] = &treeListHelper;
}


void AttDescr::addListHelpers( const ListHelperSet & help )
{
  ListHelperSet::const_iterator	ih, eh = help.end();
  for( ih=help.begin(); ih!=eh; ++ih )
    listHelpers.insert( *ih );
}


void AttDescr::describeAttributes( QObjectBrowserWidget* br, 
				   QTreeWidgetItem* parent, 
				   const GenericObject *ao, 
				   bool regist, bool checkexisting ) const
{
  Syntax			s;
  Syntax::const_iterator	is, eds;
  string			sem, type, value;
  Object			ia;
  SyntaxSet::const_iterator	iss;
  //QTreeWidgetItem			*item;

  const SyntaxedInterface	*si = ao->getInterface<SyntaxedInterface>();
  if( si && si->hasSyntax() )
    {
      iss = syntax.find( si->getSyntax() );
      if( iss != syntax.end() )
        s = (*iss).second;
    }
  Syntax	ds;
  bool		hidden;
  iss = syntax.find( "__default__" );
  if( iss != syntax.end() )
    ds = iss->second;
  eds = ds.end();

  if( parent->childCount() == 0 )
    checkexisting = false;

  int ul = theAnatomist->userLevel();

  for( ia=ao->objectIterator(); ia->isValid(); ia->next() )
    {
      sem = ia->key();
      is = s.find( sem );
      hidden = false;
      if( is != s.end() )
        //	{
        {
          type = is->second.type;
          hidden = is->second.internal;
        }
      else
        {
          is = ds.find( sem );
          if( is != eds )
            {
              type = is->second.type;
              hidden = is->second.internal;
            }
          else
            type = ia->currentValue()->type();
        }
      if( !hidden || ul >= 3 )
        printAttribute( br, ao, sem, type, parent, regist, checkexisting );
    }
}


void AttDescr::treeListHelper( QObjectBrowserWidget* br, 
                               const GenericObject & ao, 
                               const string & semantic, 
                               QTreeWidgetItem* parent, const AttDescr* gvw, 
                               bool regist )
{
  Tree	*val;

  if( !ao.getProperty( semantic, val ) )
    printError( parent, semantic.c_str() );
  else
  {
    QTreeWidgetItem	*item;

    item = br->itemFor( parent, &ao, regist );
    if( !item )
    {
      item = new QTreeWidgetItem( parent );
      item->setText( 0, semantic.c_str() );
      item->setText( 1, "tree" );
      if( regist )
        br->registerGObject( item, (GenericObject *) &ao );
      gvw->describeTree( br, val, item, regist );
    }
    else
    {
      item->setText( 0, semantic.c_str() );
      item->setText( 1, "tree" );
    }
  }
}


namespace
{

  void clipValueString( string & value, long cut = -1 )
  {
    if( cut < 0 )
      try
      {
        cut = 0;
        Object o = theAnatomist->config()->getProperty( "clipBrowserValues" );
        if( o )
          cut = (long) o->getScalar();
      }
      catch( exception & )
      {
      }
    if( cut <= 0 )
      return;
    if( value.length() > (string::size_type) cut )
      value.replace( cut, value.length() - cut, " [...]" );
  }

}


void AttDescr::printAttribute( QObjectBrowserWidget* br, 
			       const GenericObject* ao, 
			       const string & semantic, 
			       const string & type, 
			       QTreeWidgetItem* parent, bool regist, 
                               bool checkexisting ) const
{
  ListHelperSet::const_iterator	ilh;

  int clip = 0;
  try
  {
    Object o = theAnatomist->config()->getProperty( "clipBrowserValues" );
    if( o )
      clip = (long) o->getScalar();
  }
  catch( exception & )
  {
  }

  if( (ilh = listHelpers.find( type )) != listHelpers.end() )
    ((*ilh).second)( br, *ao, semantic, parent, this, regist );
  else
    {
      string value = printAttribute( br, ao, semantic, type );
      QTreeWidgetItem	*item;

      clipValueString( value, clip );

      if( checkexisting )
        if( regist )
          item = br->itemFor( parent, br->ATTRIBUTE, semantic );
        else
          item = br->itemFor( parent, semantic );
      else
        item = 0;
      if( !item )
      {
#ifdef ANA_USE_EDITABLE_LISTVIEWITEMS
        item = new aims::gui::QEditableListViewItem( parent,
            semantic.c_str(), type.c_str(), value.c_str() );
        item->setRenameEnabled( 2, true );
#else
        item = new QTreeWidgetItem( parent );
        item->setText( 0, semantic.c_str() );
        item->setText( 1, type.c_str() );
        item->setText( 2, value.c_str() );
#endif
        if( regist )
          br->registerAttribute( item );
      }
      else
      {
        item->setText( 1, type.c_str() );
        item->setText( 2, value.c_str() );
      }
    }
}


namespace
{

  void printGeneric( QObjectBrowserWidget*, 
                    const GenericObject & ao, 
                    const string & semantic, string & output )
  {
    PythonWriter	pw;
    ostringstream	oss;
    pw.attach( oss );
    pw.setSingleLineMode( true );
    try
      {
        pw.write( ao.getProperty( semantic ), false, false );
        output = oss.str();
        if( !output.empty() && output[ output.length() - 1 ] == '\n' )
          output.erase( output.length() - 1, 1 );
      }
    catch( ... )
      {
        output = "** unsupported / internal type **";
      }
  }

}


string AttDescr::printAttribute( QObjectBrowserWidget* br, 
				 const GenericObject* ao, 
				 const string & semantic, 
				 const string & type ) const
{
  HelperSet::const_iterator	ih;
  string	ostr;

  if( (ih = helpers.find( type )) != helpers.end() )
    {
      ((*ih).second)( br, *ao, semantic, ostr );

      return ostr;
    }
  else
    {
      printGeneric( br, *ao, semantic, ostr );
      return ostr;
    }
    // return( "** unsupported type **" );
}


void AttDescr::printError( QTreeWidgetItem* parent, 
			   const string & semantic )
{
  QTreeWidgetItem* item = new QTreeWidgetItem( parent );
  item->setText( 0, semantic.c_str() );
  item->setText( 1, "tree" );
  item->setText( 2, "** error **" );
}


void AttDescr::describeTree( QObjectBrowserWidget* br, const Tree* tr, 
			     QTreeWidgetItem* parent, bool regist ) const
{
  QTreeWidgetItem	*item;
  string	name;

  item = br->itemFor( parent, tr, regist );
  if( !item )
    {
      item = new QTreeWidgetItem( parent );
      item->setText( 0, tr->getSyntax().c_str() );
      item->setText( 1, "Tree" );
      if( regist )
	br->registerGObject( item, (Tree *) tr );
    }
  else
    {
      item->setText( 0, tr->getSyntax().c_str() );
      item->setText( 1, "Tree" );
    }
  if( tr->getProperty( "name", name ) )
    item->setText( 2, name.c_str() );
  if( tr->getProperty( "label", name ) )
    item->setText( 3, name.c_str() );
  vector<int> col;
  if( tr->getProperty( "color", col ) && col.size() >= 3 )
  {
    QColor rgb( col[0], col[1], col[2] );
    QPixmap cpix = rgbPixmap( rgb );
    item->setIcon( 0, cpix );
  }
  else if( parent->parent() )
  {
    // propagate parent pixmap (color) except for the
    // top-level (AObject level) TODO: use a better test than top-level
    QIcon cpix = parent->icon( 0 );
    if( !cpix.isNull() )
      item->setIcon( 0, cpix );
  }
  describeTreeInside( br, tr, item, regist );
}


void AttDescr::describeTreeInside( QObjectBrowserWidget* br, const Tree* tr, 
				   QTreeWidgetItem* item, bool regist ) const
{
  describeAttributes( br, item, tr, regist );

  Tree::const_iterator	it, ft=tr->end();
  for( it=tr->begin(); it!=ft; ++it )
    describeTree( br, (Tree*) (*it), item, regist );
}


string AttDescr::objectName( const GenericObject* ao ) const
{
  string	name, n2;

  //if( !ao->getProperty( "label", name ) && !ao->getProperty( "name", name ) )
  if( !ao->getProperty( "name", name ) )
    {
      if( ao->getProperty( "label1", name ) 
	  && ao->getProperty( "label2", n2 ) )
	{
	  name += " - ";
	  name += n2;
	}
      else
	name = "Unnamed";
    }

  return( name );
}


void AttDescr::initHelpers()
{
  helpers.erase( helpers.begin(), helpers.end() );
  helpers[ "string"          ] = &printString;
  helpers[ "int"             ] = &printInt;
  helpers[ "float"           ] = &printFloat;
  helpers[ "double"          ] = &printDouble;
  //helpers[ "float_vector"    ] = &printFloatVector;
  //helpers[ "double_vector"   ] = &printDoubleVector;
  //helpers[ "int_vector"      ] = &printIntVector;
  //helpers[ "string_vector"   ] = &printStringVector;
  //helpers[ "string_vector_p" ] = &printStringVectorPtr;
}


void AttDescr::printInt( QObjectBrowserWidget*, 
			 const GenericObject & ao, 
			 const string & semantic, string & output )
{
  char	value[20];
  int	val;
  if( !ao.getProperty( semantic, val ) )
    output = "** error **";
  else
    {
      sprintf( value, "%d", val );
      output = string( value );
    }
}


void AttDescr::printFloat( QObjectBrowserWidget*, 
			   const GenericObject & ao, 
			   const string & semantic, string & output )
{
  char	value[20];
  float	val;
  if( !ao.getProperty( semantic, val ) )
    output = "** error **";
  else
    {
      sprintf( value, "%g", val );
      output = string( value );
    }
}


void AttDescr::printDouble( QObjectBrowserWidget*, 
			    const GenericObject & ao, 
			    const string & semantic, string & output )
{
  char		value[20];
  double	val;
  if( !ao.getProperty( semantic, val ) )
    output = "** error **";
  else
    {
      sprintf( value, "%g", val );
      output = string( value );
    }
}


void AttDescr::printString( QObjectBrowserWidget*, 
			    const GenericObject & ao, 
			    const string & semantic, string & output )
{
  string	val;
  if( !ao.getProperty( semantic, val ) )
    output = "** error **";
  else
    output = val;
}


void AttDescr::printFloatVector( QObjectBrowserWidget*, 
				 const GenericObject & ao, 
				 const string & semantic, string & output )
{
  char	value[20];
  vector<float>	val;
  if( !ao.getProperty( semantic, val ) )
    output = "** error **";
  else
    {
      output = "";
      unsigned	i;
      for( i=0; i<val.size(); ++i )
	{
	  sprintf( value, "%g", val[i] );
	  output += string( value ) + " ";
	}
    }
}


void AttDescr::printDoubleVector( QObjectBrowserWidget*, 
				  const GenericObject & ao, 
				  const string & semantic, string & output )
{
  char	value[20];
  vector<double>	val;
  if( !ao.getProperty( semantic, val ) )
    output = "** error **";
  else
    {
      output = "";
      unsigned	i;
      for( i=0; i<val.size(); ++i )
	{
	  sprintf( value, "%g", val[i] );
	  output += string( value ) + " ";
	}
    }
}


void AttDescr::printIntVector( QObjectBrowserWidget*, 
			       const GenericObject & ao, 
			       const string & semantic, string & output )
{
  char	value[20];
  vector<int>	val;
  if( !ao.getProperty( semantic, val ) )
    output = "** error **";
  else
    {
      output = "";
      unsigned	i;
      for( i=0; i<val.size(); ++i )
	{
	  sprintf( value, "%d", val[i] );
	  output += string( value ) + " ";
	}
    }
}


void AttDescr::printStringVector( QObjectBrowserWidget*, 
				  const GenericObject & ao, 
				  const string & semantic, string & output )
{
  vector<string>	val;
  if( !ao.getProperty( semantic, val ) )
    output = "** error **";
  else
    {
      output = "";
      unsigned	i;
      for( i=0; i<val.size(); ++i )
	output += val[i] + "   ";
    }
}


void AttDescr::printStringVectorPtr( QObjectBrowserWidget*, 
				     const GenericObject & ao, 
				     const string & semantic, 
				     string & output )
{
  vector<string>	*val;
  if( !ao.getProperty( semantic, val ) )
    output = "** error **";
  else
    {
      output = "";
      unsigned	i;
      for( i=0; i<val->size(); ++i )
	output += (*val)[i] + "   ";
    }
}
