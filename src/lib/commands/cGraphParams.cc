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

#include <anatomist/commands/cGraphParams.h>
#include <anatomist/selection/selectFactory.h>
#include <anatomist/processor/Registry.h>
#include <anatomist/processor/unserializer.h>
#include <anatomist/processor/Serializer.h>
#include <anatomist/graph/Graph.h>
#include <anatomist/graph/GraphObject.h>
#include <anatomist/hierarchy/hierarchy.h>
#include <anatomist/window/Window.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/control/graphParams.h>
#include <anatomist/processor/context.h>
#include <anatomist/selection/selectFactory.h>
#include <cartobase/object/syntax.h>
#include <graph/tree/tree.h>
#include <vector>

using namespace anatomist;
using namespace carto;
using namespace std;


GraphParamsCommand::GraphParamsCommand( const std::string & dispmode, 
					bool change_usenom, 
					bool usenom, 
					const std::string & labelattrib, 
					bool ch_showttips, bool showttips, 
					bool ch_selcolorinv, bool selcolorinv, 
					const std::vector<int> & selcolor, 
					const std::string & savemode, 
					bool ch_saveonlymod, 
					bool saveonlymod, bool ch_setbasedir, 
					bool setbasedir )
  : RegularCommand(), _displaymode( dispmode ), 
    _ch_usenomenclature( change_usenom ), _usenomenclature( usenom ), 
    _labelattrib( labelattrib ), _ch_showttips( ch_showttips ), 
    _showttips( showttips ), _ch_selcolorinv( ch_selcolorinv ), 
    _selcolorinv( selcolorinv ), _selcolor( selcolor ), _savemode( savemode ), 
    _ch_saveonlymodif( ch_saveonlymod ), _saveonlymodif( saveonlymod ), 
    _ch_setbasedir( ch_setbasedir ), _setbasedir( setbasedir )
{
}


GraphParamsCommand::~GraphParamsCommand()
{
}


bool GraphParamsCommand::initSyntax()
{
  SyntaxSet	ss;
  Syntax	& s = ss[ "GraphParams" ];
  
  s[ "display_mode"            ].type = "string";
  s[ "use_nomenclature"        ].type = "int";
  // deprecated
  s[ "use_hierarchy"           ].type = "int";
  s[ "label_attribute"         ].type = "string";
  s[ "show_tooltips"           ].type = "int";
  s[ "selection_color_inverse" ].type = "int";
  s[ "selection_color"         ].type = "int_vector";
  s[ "saving_mode"             ].type = "string";
  s[ "save_only_modified"      ].type = "int";
  s[ "set_base_directory"      ].type = "int";
  Registry::instance()->add( "GraphParams", &read, ss );
  return( true );
}


void GraphParamsCommand::doit()
{
  GraphParams	*gp = GraphParams::graphParams();

  if( _displaymode == "mesh" )
    AGraphObject::setShowType( AGraphObject::TRIANG );
  else if( _displaymode == "bucket" )
    AGraphObject::setShowType( AGraphObject::BUCKET );
  else if( _displaymode == "all" )
    AGraphObject::setShowType( AGraphObject::ALL );
  else if( _displaymode == "first" )
    AGraphObject::setShowType( AGraphObject::FIRST );

  if( _ch_usenomenclature )
    gp->colorsActive = _usenomenclature;
  if( !_labelattrib.empty() )
    gp->attribute = _labelattrib;
  if( _ch_showttips )
    gp->toolTips = _showttips;
  if( _ch_selcolorinv )
    SelectFactory::selectColorInverse() = _selcolorinv;
  if( _selcolor.size() >= 3 )
    {
      SelectFactory::selectColor().r = ((float) _selcolor[0]) / 255.99;
      SelectFactory::selectColor().g = ((float) _selcolor[1]) / 255.99;
      SelectFactory::selectColor().b = ((float) _selcolor[2]) / 255.99;
      if( _selcolor.size() > 3 && _selcolor[3] >= 0 )
	SelectFactory::selectColor().a = ((float) _selcolor[3]) / 255.99;
      if( _selcolor.size() > 4 )
	SelectFactory::selectColor().na = (bool) _selcolor[4];
    }
  if( _savemode == "unchanged" )
    gp->saveMode = 0;
  else if( _savemode == "global_file" )
    gp->saveMode = 1;
  else if( _savemode == "individual_file" )
    gp->saveMode = 2;
  if( _ch_saveonlymodif )
    gp->saveOnlyModified = _saveonlymodif;
  if( _ch_setbasedir )
    gp->autoSaveDir = _setbasedir;

  if( _ch_usenomenclature || _ch_showttips || _ch_selcolorinv 
      || _ch_saveonlymodif || _ch_setbasedir || !_displaymode.empty() 
      || !_labelattrib.empty() || !_selcolor.empty() || !_savemode.empty() )
    gp->updateGraphs();
}


Command* GraphParamsCommand::read( const Tree & com, CommandContext* )
{
  string	dispmode, labelatt, savemode;
  int		usehie = 1, showtip = 1, selinv = 0, savemodif = 1, 
    setbase = 1;
  bool		chusehie, chshowtip, chselinv, chsavemodif, chsetbase;
  vector<int>	selcol;

  com.getProperty( "display_mode", dispmode );
  com.getProperty( "label_attribute", labelatt );
  com.getProperty( "saving_mode", savemode );
  com.getProperty( "selection_color", selcol );
  chusehie = com.getProperty( "use_nomenclature", usehie ) 
    || com.getProperty( "use_hierarchy", usehie );
  chshowtip = com.getProperty( "show_tooltips", showtip );
  chselinv = com.getProperty( "selection_color_inverse", selinv );
  chsavemodif = com.getProperty( "save_only_modified", savemodif );
  chsetbase = com.getProperty( "set_base_directory", setbase );

  return( new GraphParamsCommand( dispmode, (bool) chusehie, (bool) usehie, 
				  labelatt, (bool) chshowtip, (bool) showtip, 
				  (bool) chselinv, (bool) selinv, 
				  selcol, savemode, (bool) chsavemodif, 
				  (bool) savemodif, (bool) chsetbase, 
				  (bool) setbase ) );
}


void GraphParamsCommand::write( Tree & com, Serializer* ) const
{
  Tree				*t = new Tree( true, name() );

  if( !_displaymode.empty() )
    t->setProperty( "display_mode", _displaymode );
  if( _ch_usenomenclature )
    t->setProperty( "use_nomenclature", (int) _usenomenclature );
  if( !_labelattrib.empty() )
    t->setProperty( "label_attribute", _labelattrib );
  if( _ch_showttips )
    t->setProperty( "show_tooltips", (int) _showttips );
  if( _ch_selcolorinv )
    t->setProperty( "selection_color_inverse", (int) _selcolorinv  );
  if( !_selcolor.empty() )
    t->setProperty( "selection_color", _selcolor );
  if( !_savemode.empty() )
    t->setProperty( "saving_mode", _savemode );
  if( _ch_setbasedir )
    t->setProperty( "set_base_directory", (int) _setbasedir );

  com.insert( t );
}


