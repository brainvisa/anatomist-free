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
#include <anatomist/control/controlConfig.h>
#include <anatomist/control/graphParams.h>
#include <anatomist/application/globalConfig.h>
#include <anatomist/window/Window.h>
#include <anatomist/selection/selectFactory.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/window3D/cursor.h>
#include <anatomist/graph/GraphObject.h>
#include <vector>


using namespace anatomist;
using namespace std;


ControlConfiguration::~ControlConfiguration()
{
}


void ControlConfiguration::apply( const GlobalConfiguration* cfg )
{
  string	stratt, gratt, grmode;
  int		intatt, grcolact, tips;
  vector<int>	vecatt;
  vector<string>	vecstringatt;

  // selection settings
  if( cfg->getProperty( "selectionColorInverse", intatt ) )
    SelectFactory::selectColorInverse() = intatt;

  if( cfg->getProperty( "selectionColor", vecatt ) )
  {
    if( vecatt.size() == 3 )
    {
      SelectFactory::selectColor().r =((float) vecatt[0]) / 255;
      SelectFactory::selectColor().g = ((float) vecatt[1]) / 255; 
      SelectFactory::selectColor().b = ((float) vecatt[2]) / 255;
      SelectFactory::selectColor().a = 1.0f;
    }
    else
    {
      SelectFactory::selectColor().r =((float) vecatt[0]) / 255;
      SelectFactory::selectColor().g = ((float) vecatt[1]) / 255; 
      SelectFactory::selectColor().b = ((float) vecatt[2]) / 255;
      SelectFactory::selectColor().a = ((float) vecatt[3]) / 255;
    }
  }

  // linked cursor settings
  if( cfg->getProperty( "linkedCursor", intatt ) )
    AWindow::setGlobalHasCursor( intatt );

  if( cfg->getProperty( "cursorShape", stratt ) )
    Cursor::setCurrentCursor( stratt );

  if( cfg->getProperty( "cursorSize", intatt ) )
    AWindow::setCursorSize( intatt );

  if( cfg->getProperty( "cursorColorAuto", intatt ) )
    AWindow::setUseDefaultCursorColor( intatt );

  if( cfg->getProperty( "cursorColor", vecatt ) )
    AWindow::setCursorColor( AimsRGB( vecatt[0], vecatt[1], vecatt[2] ) );

  //	L/R flipping
  if( cfg->getProperty( "leftRightDisplayed", intatt ) )
    AWindow::setLeftRightDisplay( intatt );

  //	L/R display size
  if (cfg->getProperty("leftRightDisplaySize", intatt))
	  AWindow::setLeftRightDisplaySize(intatt);

  if (cfg->getProperty("displayedAnnotations", vecstringatt))
	  AWindow::setDisplayedAnnotations(vecstringatt);

  //	control window look
  if( cfg->getProperty( "controlWindowLogo", intatt ) )
    {
      ControlWindow	*cw = theAnatomist->getControlWindow();
      if( cw )
	cw->enableLogo( intatt );
    }

  // Graph params options
  GraphParams	*gp = GraphParams::graphParams();
  if( cfg->getProperty( "graphUseHierarchy", grcolact ) )
    gp->colorsActive = (bool) grcolact;
  if( cfg->getProperty( "graphHierarchyAttribute", gratt ) )
    gp->attribute = gratt;
  if( cfg->getProperty( "graphUseToolTips", tips ) )
    gp->toolTips = tips;
  if( cfg->getProperty( "graphDisplayMode", grmode ) )
    {
      AGraphObject::ShowType	mode = AGraphObject::TRIANG;
      if( grmode == "voxels" )
        mode = AGraphObject::BUCKET;
      else if( grmode == "all" )
        mode = AGraphObject::ALL;
      else if( grmode == "first" )
        mode = AGraphObject::FIRST;
      AGraphObject::setShowType( mode );
    }
}


void ControlConfiguration::update( GlobalConfiguration* cfg )
{
  int		num;
  string	att, strval;
  vector<int>	vec;
  vector<string> vecstring;

  //	selection
  att = "selectionColorInverse";
  if( SelectFactory::selectColorInverse() )
    cfg->setProperty( att, (int) 1 );
  else if( cfg->hasProperty( att ) )
    cfg->removeProperty( att );

  att = "selectionColor";
  SelectFactory::HColor	hc = SelectFactory::selectColor();
  vec.push_back( (int) ( 255 * hc.r ) );
  vec.push_back( (int) ( 255 * hc.g ) );
  vec.push_back( (int) ( 255 * hc.b ) );
  vec.push_back( (int) ( 255 * hc.a ) );
  if( vec[0] != 229 || vec[1] != 51 || vec[2] != 38 || vec[3] != 255 )
    cfg->setProperty( att, vec );
  else if( cfg->hasProperty( att ) )
    cfg->removeProperty( att );

  //	linked cursor
  att = "linkedCursor";
  if( !AWindow::hasGlobalCursor() )
    cfg->setProperty( att, (int) 0 );
  else if( cfg->hasProperty( att ) )
    cfg->removeProperty( att );

  att = "cursorShape";
  strval = Cursor::currentCursorName();
  if( !strval.empty() && strval != "cross.mesh" )
    cfg->setProperty( att, strval );
  else if( cfg->hasProperty( att ) )
    cfg->removeProperty( att );

  att = "cursorSize";
  num = AWindow::cursorSize();
  if( num != 20 )
    cfg->setProperty( att, num );
  else if( cfg->hasProperty( att ) )
    cfg->removeProperty( att );

  att = "cursorColorAuto";
  num = AWindow::useDefaultCursorColor();
  if( !num )
    cfg->setProperty( att, (int) 0 );
  else if( cfg->hasProperty( att ) )
    cfg->removeProperty( att );

  att = "cursorColor";
  AimsRGB	col = AWindow::cursorColor();
  if( col.red() != 255 || col.green() != 255 || col.blue() != 255 )
    {
      vec.clear();
      vec.push_back( col.red() );
      vec.push_back( col.green() );
      vec.push_back( col.blue() );
      cfg->setProperty( att, vec );
    }
  else if( cfg->hasProperty( att ) )
    cfg->removeProperty( att );

  //	L/R flipping
  att = "leftRightDisplayed";
  num = AWindow::leftRightDisplay();
  if( num )
    cfg->setProperty( att, num );
  else if( cfg->hasProperty( att ) )
    cfg->removeProperty( att );

  //	L/R display size
  att = "leftRightDisplaySize";
  num = AWindow::leftRightDisplaySize();
  if (num)
	  cfg->setProperty(att, num);
  else if (cfg->hasProperty(att))
	  cfg->removeProperty(att);

  att = "displayedAnnotations";
  vecstring = AWindow::displayedAnnotations();
  cfg->setProperty(att, vecstring);

  //	control window look
  ControlWindow	*cw = theAnatomist->getControlWindow();

  att = "controlWindowLogo";
  if( cw )
    {
      if( !cw->logoEnabled() )
	cfg->setProperty( att, (int) 0 );
      else if( cfg->hasProperty( att ) )
	cfg->removeProperty( att );
    }

  //	graph parameters
  GraphParams	*gp = GraphParams::graphParams();
  att = "graphUseHierarchy";
  if( !gp->colorsActive )
    cfg->setProperty( att, (int) 0 );
  else if( cfg->hasProperty( att ) )
    cfg->removeProperty( att );
  att = "graphHierarchyAttribute";
  if( gp->attribute != "name" )
    cfg->setProperty( att, gp->attribute );
  else if( cfg->hasProperty( att ) )
    cfg->removeProperty( att );
  att = "graphUseToolTips";
  if( !gp->toolTips )
    cfg->setProperty( att, (int) 0 );
  else if( cfg->hasProperty( att ) )
    cfg->removeProperty( att );
  att = "graphDisplayMode";
  if( AGraphObject::showType() != AGraphObject::TRIANG )
    {
      string	mode[] = { "meshes", "voxels", "all", "first" };
      cfg->setProperty( att, mode[ AGraphObject::showType() ] );
    }
  else if( cfg->hasProperty( att ) )
    cfg->removeProperty( att );
}
