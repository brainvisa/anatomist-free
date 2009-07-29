/* Copyright (c) 1995-2005 CEA
 *
 *  This software and supporting documentation were developed by
 *      CEA/DSV/SHFJ
 *      4 place du General Leclerc
 *      91401 Orsay cedex
 *      France
 *
 * This software is governed by the CeCILL license version 2 under 
 * French law and abiding by the rules of distribution of free software.
 * You can  use, modify and/or redistribute the software under the 
 * terms of the CeCILL license version 2 as circulated by CEA, CNRS
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
 * knowledge of the CeCILL license version 2 and that you accept its terms.
 */


#include <anatomist/erpio/erpWraper.h>
// this include must be first to prevent a compiling error
#include <qslider.h>

#include <anatomist/surface/texture.h>
#include <anatomist/erpio/erpR.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/object/actions.h>
#include <graph/tree/tree.h>
#include <aims/qtcompat/qvbox.h>
#include <aims/qtcompat/qhbox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <iostream>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>


using namespace anatomist;
using namespace std;


namespace anatomist
{

  struct ErpWraper_data
  {
    ErpWraper_data() : deleting( false ) {}

    map<unsigned, map<unsigned, string> >	cells;
    QLineEdit				*celledit;
    QLineEdit				*obsedit;
    QSlider				*cellsl;
    QSlider				*obssl;
    unsigned				cell;
    unsigned				obs;
    bool                                deleting;
  };


}


static bool ErpWraper_initialized = ErpWraper::initTexOptions();


ErpWraper::ErpWraper( ATexture* obj, const string & dirname, QWidget* parent ) 
  : QWidget( parent, "ErpWraper", Qt::WDestructiveClose ), 
    _dirname( dirname ), _texture( obj ), _data( new ErpWraper_data )
{
  setCaption( tr( "ERP loader : " ) + dirname.c_str() );
  obj->addObserver( this );
  scanDir();

  QHBoxLayout	*mainLay = new QHBoxLayout( this );
  QVBox	*leftPanel = new QVBox( this );
  leftPanel->setSpacing( 5 );
  leftPanel->setMargin( 10 );
  mainLay->addWidget( leftPanel );

  QHBox	*cellbox = new QHBox( leftPanel );
  cellbox->setSpacing( 10 );
  QLabel	*l1 = new QLabel( tr( "Cell:" ), cellbox );
  _data->celledit = new QLineEdit( cellbox );
  _data->celledit->setFixedWidth( 50 );
  _data->cellsl = new QSlider( Qt::Horizontal, leftPanel );
  _data->cellsl->setLineStep( 1 );
  _data->cellsl->setPageStep( 1 );
  _data->cellsl->setTracking( false );

  QHBox	*obsbox = new QHBox( leftPanel );
  obsbox->setSpacing( 10 );
  QLabel	*l2 = new QLabel( tr( "Observation:" ), obsbox );
  l1->setFixedSize( l2->sizeHint() );
  l2->setFixedSize( l2->sizeHint() );
  _data->obsedit = new QLineEdit( obsbox );
  _data->obsedit->setFixedWidth( 50 );
  _data->obssl = new QSlider( Qt::Horizontal, leftPanel );
  _data->obssl->setLineStep( 1 );
  _data->obssl->setPageStep( 1 );
  _data->obssl->setTracking( false );

  fillEdits();

  connect( _data->cellsl, SIGNAL( valueChanged( int ) ), 
	   this, SLOT( cellSliderChanged( int ) ) );
  connect( _data->obssl, SIGNAL( valueChanged( int ) ), 
	   this, SLOT( obsSliderChanged( int ) ) );
}


ErpWraper::~ErpWraper()
{
  _data->deleting = true;
  _texture->deleteObserver( this );
  delete _data;
}


ObjectMenu* ErpWraper::textureMenus( const AObject* objtype, ObjectMenu* menu )
{
  if( menu && objtype->type() == AObject::TEXTURE )
  {
    vector<string> vs;
    vs.reserve(1);
    vs.push_back( QT_TRANSLATE_NOOP( "QSelectMenu", "File" ) );
    menu->insertItem( vs, QT_TRANSLATE_NOOP( "QSelectMenu", "ERP loader" ),
                      &openWraper );

  }
  return menu;
}


bool ErpWraper::initTexOptions()
{
  AObject::addObjectMenuRegistration( textureMenus );

  return true;
}


void ErpWraper::openWraper( const set<AObject *> & obj )
{
  set<AObject *>::const_iterator	io, fo=obj.end();
  ATexture				*tex;

  for( io=obj.begin(); io!=fo; ++io )
    {
      tex = dynamic_cast<ATexture *>( *io );
      if( tex )
	{
	  if( tex->fileName().empty() )
	    tex->setFileName( tex->name() );
	  ErpWraper	*erpw = new ErpWraper( tex, tex->fileName() );
	  erpw->show();
	}
    }
}


void ErpWraper::update( const Observable*, void* arg )
{
  if( arg == 0 )
    {
      cout << "called obsolete ErpWraper::update( obs, NULL )\n";
      delete this;
    }
}


void ErpWraper::unregisterObservable( Observable* o )
{
  Observer::unregisterObservable( o );
  if( !_data->deleting )
    delete this;
}


void ErpWraper::scanDir()
{
  DIR	*dir = opendir( _dirname.c_str() );
  string::size_type	pos;

  if( !dir )
    {
      // maybe _dirname is a filename inside directory ? try this...
      pos = _dirname.rfind( '/' );
      if( pos == string::npos )
	pos = 0;

      string	dirname = _dirname.substr( 0, pos );

      if( dirname.empty() )
	dirname = ".";
      dir = opendir( dirname.c_str() );
      if( dir )
	_dirname = dirname;
      else
	{
	  cerr << "Cannot open directory " << _dirname << endl;
	  return;
	}
    }

  _data->cells.clear();

  struct dirent	*dent;
  string	fname;
  unsigned	cell, obs;

  for( dent=readdir( dir ); dent; dent=readdir( dir ) )
    {
      fname = dent->d_name;
      pos = fname.rfind( '.' );
      if( pos != string::npos 
	  && fname.substr( pos+1, fname.length() - pos - 1 ) == "erp" )
	{
	  pos = fname.find( "cell" );
	  if( pos != string::npos )
	    {
	      sscanf( fname.c_str() + pos + 4, "%u", &cell );
	      pos = fname.find( "obs" );
	      if( pos != string::npos )
		{
		  sscanf( fname.c_str() + pos + 3, "%u", &obs );
		  _data->cells[cell][obs] = fname;
		}
	    }
	}
    }

  closedir( dir );

  //	current settings

  fname = _texture->fileName();

  pos = fname.find( "cell" );
  if( pos != string::npos )
    {
      sscanf( fname.c_str() + pos + 4, "%u", &cell );
      pos = fname.find( "obs" );
      if( pos != string::npos )
	{
	  sscanf( fname.c_str() + pos + 3, "%u", &obs );
	  _data->cell = cell;
	  _data->obs = obs;
	}
    }
  else	// fileName seems not part of the .erp files...
    {
      _data->cell = 0;
      _data->obs = 0;
    }
}


void ErpWraper::fillEdits()
{
  _data->celledit->setText( QString::number( _data->cell ) );
  _data->obsedit->setText( QString::number( _data->obs ) );

  unsigned	mc, Mc, mo, Mo;
  map<unsigned, map<unsigned, string> >::const_iterator 
    ic = _data->cells.begin(), fc = _data->cells.end();

  if( ic == fc )
    mc = 0;
  else
    mc = (*ic).first;

  map<unsigned, map<unsigned, string> >::reverse_iterator 
    icr = _data->cells.rbegin();

  if( icr == _data->cells.rend() )
    Mc = 0;
  else
    Mc = (*icr).first;

  if( _data->cell < mc )
    _data->cell = mc;
  else if( _data->cell > Mc )
    _data->cell = Mc;

  _data->cellsl->setMinValue( mc );
  _data->cellsl->setMaxValue( Mc );
  _data->cellsl->setValue( _data->cell );

  if( ic == fc )
    {
      mo = 0;
      Mo = _data->obs;
    }
  else
    {
      ic = _data->cells.lower_bound( _data->cell );

      const map<unsigned, string>	& obsm = (*ic).second;
      map<unsigned, string>::const_iterator	io = obsm.begin(), 
	fo = obsm.end();

      if( io == fo )
	{
	  mo = 0;
	  Mo = 0;
	}
      else
	{
	  mo = (*io).first;
	  map<unsigned, string>::const_reverse_iterator	ior = obsm.rbegin();
	  Mo = (*ior).first;
	}
    }
  if( _data->obs < mo )
    _data->obs = mo;
  else if( _data->obs > Mo )
    _data->obs = Mo;

  _data->obssl->setMinValue( mo );
  _data->obssl->setMaxValue( Mo );
  _data->obssl->setValue( _data->obs );
}


void ErpWraper::cellSliderChanged( int val )
{
  //_data->celledit->setText( QString::number( val ) );
  _data->cell = val;
  fillEdits();

  loadErp();
}


void ErpWraper::obsSliderChanged( int val )
{
  _data->obsedit->setText( QString::number( val ) );
  _data->obs = val;

  loadErp();
}


void ErpWraper::loadErp()
{
  string	name = _data->cells[_data->cell][_data->obs];
  ErpReader	er( _dirname + "/" + name );

  er >> *_texture;

  _texture->setName( "tmp" );
  _texture->setName( theAnatomist->makeObjectName( name ) );
  theAnatomist->NotifyObjectChange( _texture );

  _texture->notifyObservers( this );
}
