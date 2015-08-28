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


#ifndef ANATOMIST_PERFUSION_PERFPROCQTDECO_H
#define ANATOMIST_PERFUSION_PERFPROCQTDECO_H

#include <aims/perfusion/perfProcCenter.h>
#include <anatomist/volume/Volume.h>
#include <QObject>
#include <list>


class QAPerfusionWindow;

namespace anatomist
{
  class Bucket;
}

class QTreeWidgetItem;

class PerfusionProcessingQtDecorator : public QObject
{
  Q_OBJECT

  typedef void (*pMethod)( void *, int );

public:

  PerfusionProcessingQtDecorator( aims::PerfusionProcessingCenter *, 
                                  QAPerfusionWindow * );
  virtual ~PerfusionProcessingQtDecorator() { }

  void initialize();

  static void doMaskCbk( void *, int );
  static void doSkipCbk( void *, int );
  static void doAifPointsCbk( void *, int );
  static void doInjectionCbk( void *, int );
  static void doQuantificationCbk( void *, int );
  static void doAifCbk( void *, int );
  static void doFitCbk( void *, int );
  static void doDeconvolutionCbk( void *, int );
  static void doMapsCbk( void *, int );

public slots:

  void apply( int );

  void setTr( const QString& );
  void setTe( const QString& );

  void setMaskFilter( bool );
  void setBThreshold( int );
  void setLVThreshold( int );
  void setSkipThreshold( float );
  void setSkip( int );
  void setNAif( int );
  void setAifThreshold( int );
  void setPreInj( int );
  void setAifType( int );
  void setNAvg( int );
  void setFitType( int );
  void setCorrection( bool );
  void setSVDType( int );
  void setSVDThreshold( float );
  void setDose( const QString& );
  void setPhiGd( const QString& );

  void linkedCursor( QTreeWidgetItem *, int );

  void mapClicked( int );
  void saveMaps();

signals:

  void skipChanged( int );
  void preInjChanged( int );

private:

  static std::map< int, pMethod > mMeth;

  std::list< Point4dl > _selList;

  aims::PerfusionProcessingCenter *_ppBase;

  QAPerfusionWindow *_parent;
  
  carto::rc_ptr<carto::Volume< short > > volIn;
  anatomist::AObject *initial;
  
  anatomist::Bucket *bckMask;
  anatomist::AVolume< float > *volQuant;
  anatomist::AVolume< float > *volAif;
  anatomist::AVolume< float > *volAifFit;
  anatomist::AVolume< float > *volTSmooth;
  anatomist::AVolume< float > *volFit;
  anatomist::AVolume< float > *volDeconv;
  anatomist::AVolume< float > *volCbf;
  anatomist::AVolume< float > *volCbv;
  anatomist::AVolume< float > *volMtt;
  anatomist::AVolume< float > *volTtp;
  anatomist::AVolume< float > *volDelay;
  anatomist::AVolume< float > *volh;
  anatomist::AVolume< float > *volBbb;

  void doMask( int );
  void doSkip( int );
  void doAifPoints( int );
  void doInjection( int );
  void doQuantification( int );
  void doAif( int );
  void doFit( int );
  void doDeconvolution( int );
  void doMaps( int );
};

#endif
