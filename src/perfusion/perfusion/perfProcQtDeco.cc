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



#include <anatomist/perfusion/perfProcQtDeco.h>
#include <anatomist/processor/Processor.h>
#include <anatomist/commands/cLinkedCursor.h>
#include <anatomist/bucket/Bucket.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/winperf/perfWin.h>

#include <aims/utility/converter_volume.h>
#include <aims/perfusion/perfMask.h>
#include <aims/perfusion/perfSkip.h>
#include <aims/perfusion/perfAifPoints.h>
#include <aims/perfusion/perfPreInj.h>
#include <aims/perfusion/perfQuantif.h>
#include <aims/perfusion/perfAif.h>
#include <aims/perfusion/perfFit.h>
#include <aims/perfusion/perfDeconv.h>
#include <aims/perfusion/perfMaps.h>

#include <qmessagebox.h>
#include <aims/qtcompat/qfiledialog.h>
#include <qapplication.h>
#include <aims/qtcompat/qbutton.h>
#include <qcheckbox.h>

#include <stdio.h>
#include <string>


using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;


map< int, PerfusionProcessingQtDecorator::pMethod > 
PerfusionProcessingQtDecorator::mMeth;


PerfusionProcessingQtDecorator::PerfusionProcessingQtDecorator( 
  PerfusionProcessingCenter *pBase, QAPerfusionWindow *pwin )
  : QObject()
{
  _ppBase = pBase;
  _parent = pwin;

  volIn = (AVolume< int16_t > *)0;

  bckMask = new anatomist::Bucket;
  volQuant = new AVolume< float >;
  volAif = new AVolume< float >;
  volAifFit = new AVolume< float >;
  volTSmooth = new AVolume< float >;
  volFit = new AVolume< float >;
  volDeconv = new AVolume< float >;

  string palette = "Rainbow";
  volCbf = new AVolume< float >;
  volCbf->createDefaultPalette( palette );
  volCbv = new AVolume< float >;
  volCbv->createDefaultPalette( palette );
  volMtt = new AVolume< float >;
  volMtt->createDefaultPalette( palette );
  volTtp = new AVolume< float >;
  volTtp->createDefaultPalette( palette );
  volDelay = new AVolume< float >;
  volDelay->createDefaultPalette( palette );
  volh = new AVolume< float >;
  volBbb = new AVolume< float >;
  volBbb->createDefaultPalette( palette );

  initialize();
}


void PerfusionProcessingQtDecorator::initialize()
{
  map< int, PerfusionProcessing * >::iterator it;
  it  = _ppBase->processings().begin();

  mMeth[ (*it++).first ] = &doSkipCbk;
  mMeth[ (*it++).first ] = &doMaskCbk;
  mMeth[ (*it++).first ] = &doAifPointsCbk;
  mMeth[ (*it++).first ] = &doInjectionCbk;
  mMeth[ (*it++).first ] = &doQuantificationCbk;
  mMeth[ (*it++).first ] = &doAifCbk;
  mMeth[ (*it++).first ] = &doFitCbk;  
  mMeth[ (*it++).first ] = &doDeconvolutionCbk;
  mMeth[ (*it++).first ] = &doMapsCbk;
}


void PerfusionProcessingQtDecorator::apply( int btn )
{
  if ( _ppBase->parameters().tr() > 0.0f && _ppBase->parameters().te() > 0.0f )
    {
      map< int, PerfusionProcessingQtDecorator::pMethod >::iterator itm, itf;
      map< int, PerfusionProcessing * >::iterator it1, itd, itb, its, itt;

      itm = mMeth.begin();
      itf = mMeth.find( btn );
      it1 = itd = _ppBase->processings().begin();
      its = _ppBase->processings().end();
      itb = _ppBase->processings().find( btn );

      // Look for the first processing to apply
      while( itd != itb && ((*itd).second)->isDone() ) 
	{
	  ++itm;
	  ++itd;
	}

      // Set all the following processing to be as if they weren't be performed
      for ( ; itb != its; ++itb )  (*itb).second->setDone( false );

      // Perform all the processing up to the selected one
      QApplication::setOverrideCursor( Qt::waitCursor );
      itf++;
      while ( itm != itf )
	{
	  if ( itd == it1 )  (*itm).second( this, (*itm).first );
	  else
	    {
	      itt = itd;
	      itt--;
	      if ( itt->second->isDone() )
		(*itm).second( this, (*itm).first );
	      else
		cout << "preceding step not done\n";
	    }
	  ++itm;
	  ++itd;
	}
      QApplication::restoreOverrideCursor();
    }
  else
    QMessageBox::warning( NULL, "Warning", "Invalid Tr and/or Te", 
			  QMessageBox::Ok, 0 );
}


void PerfusionProcessingQtDecorator::doSkip( int btn )
{
  AObject *aobj = *(_parent->objects().begin());

  AVolume< byte > *volb = dynamic_cast< AVolume< byte > * >( aobj );
  if ( volb )
    {
      AimsData< uint8_t > *db = (AimsData< uint8_t > *)volb;
      Converter< AimsData<byte>, AimsData<int16_t> > conv;
      AimsData< int16_t > *adout = conv( *db );
      *((AimsData< int16_t > *)volIn) = *adout;
      delete adout;
    }
  else volIn = dynamic_cast< AVolume< int16_t > * >( aobj );

  if ( volIn )
    {
      _ppBase->registerData( (AimsData< int16_t > *)volIn );
      _ppBase->doSkip( btn );
  
      emit skipChanged( _ppBase->parameters().skip() );
    }
}


void PerfusionProcessingQtDecorator::doMask( int btn )
{
  _ppBase->doMask( btn );
 
  bckMask->setBucket( _ppBase->bucketMask() );

  set< AObject * > sobj = theAnatomist->getObjects();
  if ( sobj.find( bckMask ) == sobj.end() )
    {
      bckMask->setName( "Brain mask" );
      theAnatomist->registerObject( bckMask );
    }
  else  bckMask->notifyObservers( this );
}


void PerfusionProcessingQtDecorator::doAifPoints( int btn )
{
  _ppBase->doAifPoints( btn );
  
  char t[6], delta[10], point[20];
  
  _parent->listAIF()->clear();
  
  if ( !_ppBase->aifPointList().empty() )
    {
      int skip = _ppBase->parameters().skip();
      list< Point4d >::reverse_iterator it = _ppBase->aifPointList().rbegin();
      while( it != _ppBase->aifPointList().rend() )
        {
	  Point4d pt = *it;
	  Point4d pt2( pt[0], pt[1], pt[2], skip );
	  int16_t delt = (int16_t)abs( (*volIn->volume())( pt2 ) 
                                       - (*volIn->volume())( pt ) );
	  sprintf( delta, "%d", delt );
	  sprintf( t, "%d", pt[3] );
	  sprintf( point, "(%d,%d,%d)", pt[ 0 ], pt[ 1 ], pt[ 2 ] );
	  new Q3ListViewItem( _parent->listAIF(), delta, t, point );
	  ++it;
	}
    }
  else
    cerr << "No Aif points found\n";
}


void PerfusionProcessingQtDecorator::doInjection( int btn )
{
  int delta, p1, p2, p3;
  Point4d pt;
  string pos;

  _selList.clear();
  Q3ListViewItemIterator it( _parent->listAIF() );

  for ( ; it.current(); ++it )
    if ( it.current()->isSelected() )
      {
	delta = atoi( it.current()->text( 0 ).utf8().data() );

	pos = it.current()->text( 2 ).utf8().data();
	p1 = pos.find( "," );
	p2 = pos.rfind( "," );
	p3 = pos.find( ")" );

	pt[0] = atoi( pos.substr( 1, p1 - 1 ).c_str() );
	pt[1] = atoi( pos.substr( p1 + 1, p2 - p1 - 1 ).c_str() );
	pt[2] = atoi( pos.substr( p2 + 1, p3 - p2 - 1 ).c_str() );
	pt[3] = atoi( it.current()->text( 1 ).utf8().data() );

	_selList.push_back( pt );
      }

  if ( !_selList.empty() )
    {
      _ppBase->setSelection( _selList );
      _ppBase->doInjection( btn );
      emit preInjChanged( _ppBase->parameters().preInj() );
    }
  else
    QMessageBox::warning( NULL, "Warning", "No AIF(s) selected", 
			  QMessageBox::Ok, 0 );
}


void PerfusionProcessingQtDecorator::doQuantification( int btn )
{
  _ppBase->doQuantification( btn );
  
  *((AimsData< float > *)volQuant) = _ppBase->quantifiedData();
  volQuant->SetExtrema();
  volQuant->adjustPalette();

  set< AObject * > sobj = theAnatomist->getObjects();
  if ( sobj.find( volQuant ) == sobj.end() )
    {
      volQuant->setName( "Quantified" );
      theAnatomist->registerObject( volQuant );
    }
  else  volQuant->notifyObservers( this );
}


void PerfusionProcessingQtDecorator::doAif( int btn )
{
  _ppBase->doAif( btn );

  *((AimsData< float > *)volAif) = _ppBase->aif();
  volAif->SetExtrema();
  volAif->adjustPalette();

  *((AimsData< float > *)volAifFit) = _ppBase->fittedAif();
  volAifFit->SetExtrema();
  volAifFit->adjustPalette();

  set< AObject * > sobj = theAnatomist->getObjects();
  if ( sobj.find( volAif ) == sobj.end() )
    {
      volAif->setName( "AIF" );
      theAnatomist->registerObject( volAif );
    }
  else  volAif->notifyObservers( this );
  
  if ( sobj.find( volAifFit ) == sobj.end() )
    {
      volAifFit->setName( "Fitted AIF" );
      theAnatomist->registerObject( volAifFit );
    }
  else  volAifFit->notifyObservers( this );
}


void PerfusionProcessingQtDecorator::doFit( int btn )
{
  _ppBase->doFit( btn );

  *((AimsData< float > *)volFit) = _ppBase->fittedData();
  volFit->SetExtrema();
  volFit->adjustPalette();

  set< AObject * > sobj = theAnatomist->getObjects();
  if ( sobj.find( volFit ) == sobj.end() )
    {
      volFit->setName( "Fitted" );
      theAnatomist->registerObject( volFit );
    }
  else  volFit->notifyObservers( this );
}


void PerfusionProcessingQtDecorator::doDeconvolution( int btn )
{
  _ppBase->doDeconvolution( btn );
  
  *((AimsData< float > *)volDeconv) = _ppBase->deconvolvedData();
  volDeconv->SetExtrema();
  volDeconv->adjustPalette();

  set< AObject * > sobj = theAnatomist->getObjects();
  if ( sobj.find( volDeconv ) == sobj.end() )
    {
      volDeconv->setName( "Deconvolved" );
      theAnatomist->registerObject( volDeconv );
    }
  else  volDeconv->notifyObservers( this );
}


void PerfusionProcessingQtDecorator::doMaps( int btn )
{
  bool status = _ppBase->doMaps( btn );

  if ( status )
    {
      set< AObject * > sobj = theAnatomist->getObjects();
      PerfusionMaps *pMap = _ppBase->perfMaps();

      if ( pMap->mapBase( (int)PerfusionMapBase::cbf )->isChecked() )
	{
	  *((AimsData< float > *)volCbf) = _ppBase->CBF();
	  volCbf->SetExtrema();
	  volCbf->adjustPalette();
	  if ( sobj.find( volCbf ) == sobj.end() )
	    {
	      volCbf->setName( "CBF" );
	      theAnatomist->registerObject( volCbf );
	    }
	  else  volCbf->notifyObservers( this );
	}

      if ( pMap->mapBase( (int)PerfusionMapBase::cbv )->isChecked() )
	{
	  *((AimsData< float > *)volCbv) = _ppBase->CBV();
	  volCbv->SetExtrema();
	  volCbv->adjustPalette();
	  if ( sobj.find( volCbv ) == sobj.end() )
	    {
	      volCbv->setName( "CBV" );
	      theAnatomist->registerObject( volCbv );
	    }
	  else  volCbv->notifyObservers( this );
	}

      if ( pMap->mapBase( (int)PerfusionMapBase::mtt )->isChecked() )
	{
	  *((AimsData< float > *)volMtt) = _ppBase->MTT();
	  volMtt->SetExtrema();
	  volMtt->adjustPalette();
	  if ( sobj.find( volMtt ) == sobj.end() )
	    {
	      volMtt->setName( "MTT" );
	      theAnatomist->registerObject( volMtt );
	    }
	  else  volMtt->notifyObservers( this );
	}

      if ( pMap->mapBase( (int)PerfusionMapBase::ttp )->isChecked() )
	{
	  *((AimsData< float > *)volTtp) = _ppBase->TTP();
	  volTtp->SetExtrema();
	  volTtp->adjustPalette();
	  if ( sobj.find( volTtp ) == sobj.end() )
	    {
	      volTtp->setName( "TTP" );
	      theAnatomist->registerObject( volTtp );
	    }
	  else  volTtp->notifyObservers( this );
	}

      if ( pMap->mapBase( (int)PerfusionMapBase::delay )->isChecked() )
	{
	  *((AimsData< float > *)volDelay) = _ppBase->delay();
	  volDelay->SetExtrema();
	  volDelay->adjustPalette();

	  if ( sobj.find( volDelay ) == sobj.end() )
	    {
	      volDelay->setName( "Delay" );
	      theAnatomist->registerObject( volDelay );
	    }
	  else  volDelay->notifyObservers( this );
	}

      if ( pMap->mapBase( (int)PerfusionMapBase::h )->isChecked() )
	{
	  *((AimsData< float > *)volh) = _ppBase->heterogeneity();
	  volh->SetExtrema();
	  volh->adjustPalette();
	  if ( sobj.find( volh ) == sobj.end() )
	    {
	      volh->setName( "h(t)" );
	      theAnatomist->registerObject( volh );
	    }
	  else  volh->notifyObservers( this );
	}

      if ( pMap->mapBase( (int)PerfusionMapBase::bbb )->isChecked() )
	{
	  *((AimsData< float > *)volBbb) = _ppBase->BBB();
	  volBbb->SetExtrema();
	  volBbb->adjustPalette();
	  if ( sobj.find( volBbb ) == sobj.end() )
	    {
	      volBbb->setName( "BBB" );
	      theAnatomist->registerObject( volBbb );
	    }
	  else  volBbb->notifyObservers( this );
	}
    }
  else 
    QMessageBox::warning( NULL, "Warning", "No map(s) selected",
			  QMessageBox::Ok, 0 );
}


// Linked cursor ( ListView <-> windows )

void PerfusionProcessingQtDecorator::linkedCursor( Q3ListViewItem *item )
{
  vector< float > pt;

  string pos = item->text( 2 ).utf8().data();

  int p1 = pos.find( "," );
  int p2 = pos.rfind( "," );
  int p3 = pos.find( ")" );

  float sx = volIn->volume()->sizeX();
  float sy = volIn->volume()->sizeY();
  float sz = volIn->volume()->sizeZ();

  pt.push_back( atof( pos.substr( 1, p1 - 1 ).c_str() ) * sx );
  pt.push_back( atof( pos.substr( p1 + 1, p2 - p1 - 1 ).c_str() ) * sy );
  pt.push_back( atof( pos.substr( p2 + 1, p3 - p2 - 1 ).c_str() ) * sz );
  pt.push_back( atof( item->text( 1 ).utf8().data() ) );

  const set< AWindow * > winlist = volIn->WinList();
  if ( !winlist.empty() )
    {
      Command *cmd = new LinkedCursorCommand( *winlist.begin(), pt );
      theProcessor->execute( cmd );
    }
}


// Slots

void PerfusionProcessingQtDecorator::setTr( const QString& str )
{
  _ppBase->parameters().setTr( atof( str.utf8().data() ) );
}


void PerfusionProcessingQtDecorator::setTe( const QString& str )
{
  _ppBase->parameters().setTe( atof( str.utf8().data() ) );
}


void PerfusionProcessingQtDecorator::setMaskFilter( bool onOff )
{
  _ppBase->parameters().setVFilter( onOff );
}


void PerfusionProcessingQtDecorator::setBThreshold( int val )
{
  _ppBase->parameters().setBThres( (float)val / 100.0f );
}


void PerfusionProcessingQtDecorator::setLVThreshold( int val )
{
  _ppBase->parameters().setLVThres( (float)val / 100.0f );
}


void PerfusionProcessingQtDecorator::setSkipThreshold( float val )
{
  _ppBase->parameters().setSkipThres( val / 100.0f );
}


void PerfusionProcessingQtDecorator::setSkip( int val )
{
  _ppBase->parameters().setSkip( val );
}


void PerfusionProcessingQtDecorator::setNAif( int val )
{
  _ppBase->parameters().setnAif( val );
}


void PerfusionProcessingQtDecorator::setAifThreshold( int val )
{
  _ppBase->parameters().setAifThreshold( (float)val / 100.0f );
}


void PerfusionProcessingQtDecorator::setPreInj( int val )
{
  _ppBase->parameters().setPreInj( val );
}


void PerfusionProcessingQtDecorator::setAifType( int btn )
{
  _ppBase->parameters().setAifType( btn );
  
  map< int, pMethod >::iterator it = mMeth.begin(), itf = mMeth.end();
  while( it != itf && (*it).second != &doAifCbk )  ++it;
  int i = it->first;
  PerfusionAif *pa;
  pa = dynamic_cast< PerfusionAif * >( _ppBase->processings()[ i ] );
  pa->setAifType( (PerfusionAif::AifType)btn );  
}


void PerfusionProcessingQtDecorator::setNAvg( int val )
{
  _ppBase->parameters().setnAvg( val );
}


void PerfusionProcessingQtDecorator::setFitType( int btn )
{
  _ppBase->parameters().setFitType( btn );
  
  map< int, pMethod >::iterator it = mMeth.begin(), itf = mMeth.end();
  while( it != itf && (*it).second != &doFitCbk )  ++it;
  int i = it->first;
  PerfusionFit *pf;
  pf = dynamic_cast< PerfusionFit * >( _ppBase->processings()[ i ] );
  pf->setFit( (PerfusionFit::FitType)btn );
}


void PerfusionProcessingQtDecorator::setCorrection( bool onOff )
{
  _ppBase->parameters().setCorrection( onOff );
}


void PerfusionProcessingQtDecorator::setSVDType( int id )
{
  _ppBase->parameters().setSVDType( id );
}


void PerfusionProcessingQtDecorator::setSVDThreshold( float val )
{
  _ppBase->parameters().setSVDThres( val / 100.0f );
}


void PerfusionProcessingQtDecorator::setDose( const QString& str )
{
  _ppBase->parameters().setDose( atof( str.utf8().data() ) );
}


void PerfusionProcessingQtDecorator::setPhiGd( const QString& str )
{
  _ppBase->parameters().setPhiGd( atof( str.utf8().data() ) );
}


void PerfusionProcessingQtDecorator::mapClicked( int btn )
{
  PerfusionMaps *pmap = _ppBase->perfMaps();
  bool bState = _parent->mapGroup()->find( btn )->isOn();
  pmap->setState( btn, bState );

  list< int > ldep = pmap->mapBase( btn )->dependencies();
  if ( bState && !ldep.empty() )
    {
      list< int >::const_iterator it = ldep.begin();
      while ( it != ldep.end() )
	{
	  _parent->mapGroup()->setButton( *it );
	  pmap->setState( (int)*it, bState );
	  ++it;
	}
    }

  list< int > bdep = pmap->mapBase( btn )->backDependencies();
  if ( !bState && !bdep.empty() )
    {
      list< int >::const_iterator it = bdep.begin();
      while ( it != bdep.end() )
	{
	  QCheckBox *qcb = 
	    dynamic_cast< QCheckBox * >( _parent->mapGroup()->find( *it ) );
	  if ( qcb )
	    {
	      qcb->setChecked( false );
	      pmap->setState( (int)*it, bState );
	    }
	  ++it;
	}
    }
}


void PerfusionProcessingQtDecorator::saveMaps()
{
  QString fname;

  QString filt = "Float GIS files (*.ima);;All files (*)";

  QFileDialog *fdia = new QFileDialog( QString::null, filt, _parent,
				       "saveMaps", true);
  fdia->setCaption( "Save maps" );
  fdia->setMode( QFileDialog::AnyFile );

  if ( fdia->exec() == QDialog::Accepted )  fname = fdia->selectedFile();

  delete fdia;

  if ( !fname.isEmpty() )
    {
      string filename = fname.utf8().data();
      _ppBase->saveMaps( filename );
    }
}


// Callbacks


void PerfusionProcessingQtDecorator::doMaskCbk( void *parent, int btn )
{
  ((PerfusionProcessingQtDecorator *)parent)->doMask( btn );
}


void PerfusionProcessingQtDecorator::doSkipCbk( void *parent, int btn )
{
  ((PerfusionProcessingQtDecorator *)parent)->doSkip( btn );
}


void PerfusionProcessingQtDecorator::doAifPointsCbk( void *parent, int btn )
{
  ((PerfusionProcessingQtDecorator *)parent)->doAifPoints( btn );
}


void PerfusionProcessingQtDecorator::doInjectionCbk( void *parent, int btn )
{
  ((PerfusionProcessingQtDecorator *)parent)->doInjection( btn );
}


void PerfusionProcessingQtDecorator::doQuantificationCbk( void *parent, int b )
{
  ((PerfusionProcessingQtDecorator *)parent)->doQuantification( b );
}


void PerfusionProcessingQtDecorator::doAifCbk( void *parent, int btn )
{
  ((PerfusionProcessingQtDecorator *)parent)->doAif( btn );
}


void PerfusionProcessingQtDecorator::doFitCbk( void *parent, int btn )
{
  ((PerfusionProcessingQtDecorator *)parent)->doFit( btn );
}


void PerfusionProcessingQtDecorator::doDeconvolutionCbk( void *parent, int b )
{
  ((PerfusionProcessingQtDecorator *)parent)->doDeconvolution( b );
}


void PerfusionProcessingQtDecorator::doMapsCbk( void *parent, int btn )
{
  ((PerfusionProcessingQtDecorator *)parent)->doMaps( btn );
}
