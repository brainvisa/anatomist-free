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

#include <cstdlib>
#include <anatomist/control/wControl.h>
#include <anatomist/object/actions.h>
#include <anatomist/action/histoplot.h>
#include <anatomist/action/roimanagementaction.h>
#include <anatomist/action/roichangeprocessor.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/hierarchy/hierarchy.h>
#include <anatomist/reference/Transformation.h>
#include <graph/tree/tree.h>
#include <qwt_plot.h>
#if QWT_VERSION >= 0x060200
#include <qwt_spline_curve_fitter.h>
#endif
#include <qspinbox.h>
#include <qstring.h>
#include <qfiledialog.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <QCheckBox>
#include <iomanip>
#if QWT_VERSION >= 0x050000
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>
#include <qwt_legend.h>
#include <qwt_curve_fitter.h>
#if QWT_VERSION >= 0x060100
#include <qwt_point_data.h>
#endif
#endif

using namespace std ;
using namespace aims ;
using namespace anatomist ;


struct RoiHistoPlot::Private 
{
public:
  Private();

  QLabel * myImageLabel ;
  QLabel * myGraphLabel ;
    
  QwtPlot * myPlotArea ;

  QSpinBox * myBinBox ;
  QPushButton * mySaveHistos ;

#if QWT_VERSION >= 0x050000
  QwtPlotMarker	*myMarkerMin;
  QwtPlotMarker	*myMarkerMax;
  QwtPlotCurve	*myImageHistoKey;
#else
  int		myImageHistoKey;
  int		myMinMarkerKey;
  int		myMaxMarkerKey;
#endif
};

RoiHistoPlot::Private::Private() :
#if QWT_VERSION >= 0x050000
  myMarkerMin( 0 ), myMarkerMax( 0 ), myImageHistoKey( 0 )
#else
  myImageHistoKey( -1 ), myMinMarkerKey( -1 ), myMaxMarkerKey( -1 )
#endif
{
}


RoiHistoPlot::RoiHistoPlot( QWidget * parent, int nbOfBins ) :
  QWidget(parent), Observer(), myImage(""), myGraph(""), myIgnoreForMax(0),
  myUseExtrema( false ),
  myNbOfBins(nbOfBins), myHistoMax(0.), myHighLevel(100.), myLowLevel(0.), 
  myActivate(false), myShowImageHisto(true)
{
  RoiChangeProcessor * proc = RoiChangeProcessor::instance() ;
  RoiManagementActionSharedData * man
    = RoiManagementActionSharedData::instance() ;
  proc->addObserver(this) ;
  man->addObserver(this) ;
  
  _private = new Private ;
  
  QVBoxLayout *lay = new QVBoxLayout( this );

  QWidget *myLabels = new QWidget( this ) ;
  lay->addWidget( myLabels );
  QHBoxLayout *lablay = new QHBoxLayout( myLabels );
  lablay->addWidget( new QLabel(tr("Image"), myLabels ) );
  
  _private->myImageLabel = new QLabel(
    RoiManagementActionSharedData::instance()->currentImage().c_str(),
    myLabels );
  lablay->addWidget( _private->myImageLabel );
  lablay->addWidget( new QLabel(tr("ROI Session"), myLabels ) );
  _private->myGraphLabel = new QLabel(
    RoiManagementActionSharedData::instance()->currentGraph().c_str(),
    myLabels );
  lablay->addWidget( _private->myGraphLabel );
  
  _private->myPlotArea = new QwtPlot( this );
  lay->addWidget( _private->myPlotArea );

#if QWT_VERSION >= 0x050000
//   _private->myMarkerMin = new QwtPlotMarker;
//   _private->myMarkerMin->attach( _private->myPlotArea );
//   _private->myMarkerMax = new QwtPlotMarker;
//   _private->myMarkerMax->attach( _private->myPlotArea );

  _private->myPlotArea->insertLegend( new QwtLegend( _private->myPlotArea ) );
#endif
  
  QWidget *myData = new QWidget( this );
  lay->addWidget( myData );
  QHBoxLayout *datalay = new QHBoxLayout( myData );

  datalay->addWidget( new QLabel(tr("Bins"), myData ) );
  _private->myBinBox = new QSpinBox( myData );
  _private->myBinBox->setRange( 10, 1000 );
  _private->myBinBox->setSingleStep( 1 );
  datalay->addWidget( _private->myBinBox );
  _private->myBinBox->setValue( myNbOfBins ) ;
  QCheckBox *myIgnoreUnderLowButton = new QCheckBox( tr("Ignore Under Low"),
                                                     myData );
  datalay->addWidget( myIgnoreUnderLowButton );

  QCheckBox *myComputeSessionHisto = new QCheckBox( tr("Roi Histos"), myData );
  datalay->addWidget( myComputeSessionHisto );
  _private->mySaveHistos = new QPushButton(
    tr("Save Histos"), myData );
  datalay->addWidget( _private->mySaveHistos );

  connect( _private->myBinBox, SIGNAL( valueChanged ( int ) ), 
	   this, SLOT( nbOfBinsChanged( int ) ) ) ;
  connect( myComputeSessionHisto, SIGNAL( stateChanged( int ) ),
	   this, SLOT( showHistoChange( int ) ) ) ;
  connect( _private->mySaveHistos, SIGNAL( clicked ( ) ), 
	   this, SLOT( saveHistos( ) ) ) ;
  connect( myIgnoreUnderLowButton, SIGNAL( stateChanged( int ) ),
	   this, SLOT( ignoreUnderLowChicked( int ) ) ) ;
}

RoiHistoPlot::~RoiHistoPlot()
{
  RoiChangeProcessor * proc = RoiChangeProcessor::instance() ;
  RoiManagementActionSharedData * man = RoiManagementActionSharedData::instance() ;
  proc->deleteObserver(this) ;
  man->deleteObserver(this) ;

  if (_private)
    delete _private;
}

void 
RoiHistoPlot::showHistoChange( int state )
{
  myShowImageHisto = (state != Qt::Checked);
  if( !myShowImageHisto )
  {
    if( myActivate )
      showGraphHisto() ;
  }
  else
  {
    if( myActivate )
      showImageHisto() ;
  }
}

void 
RoiHistoPlot::nbOfBinsChanged( int bins )
{
  myNbOfBins = bins ;
  myImageHistos.clear() ;
  myNbOfPoints.clear() ;

  if( myActivate )
  {
    if( myShowImageHisto )
      showImageHisto() ;
    else
      showGraphHisto() ;
  }

//     if( _private->myPlotArea->curveKeys().size() > 0 ) {
//       if( _private->myPlotArea->curveKeys().size() > 1 )
// 	showGraphHisto() ;
//       showImageHisto() ;
//     }
}

void 
RoiHistoPlot::ignoreUnderLowChicked( int state )
{
  if( state == Qt::Checked )
  {
    myIgnoreForMax = myLowLevel ;
    myUseExtrema = true;
  }
  else
  {
    myIgnoreForMax = 0;
    myUseExtrema = false;
  }

  if( myShowImageHisto )
    showImageHisto();
}

void
RoiHistoPlot::lowChanged( float low )
{
  if( myLowLevel == low )
    return;
  myLowLevel = low;
  if( myUseExtrema )
  {
    myIgnoreForMax = myLowLevel;
    if( myShowImageHisto )
      showImageHisto();
  }
}


void
RoiHistoPlot::highChanged( float high )
{
  myHighLevel = high ;
#if QWT_VERSION >= 0x050000
//   _private->myMarkerMax->setXValue( myHighLevel );
#else
  _private->myPlotArea->setMarkerPos( _private->myMaxMarkerKey, myHighLevel, 0.0) ;
#endif
  _private->myPlotArea->replot() ;
}


void
RoiHistoPlot::activate() 
{ 
  myImage = RoiManagementActionSharedData::instance()->currentImage() ;
  myGraph = RoiManagementActionSharedData::instance()->currentGraph() ;
  
  if( !myActivate ){
    myActivate = true ;
    if( myShowImageHisto )
      showImageHisto() ;
    else
      showGraphHisto() ;
  }
  myActivate = true ; 
}

void RoiHistoPlot::deactivate() 
{
  _private->myPlotArea->detachItems() ;
  myActivate = false ; 
}



void 
RoiHistoPlot::showImageHisto()
{
  if ( !myActivate )
    return ;
  
  _private->myPlotArea->detachItems() ;
  float binSize ;
  vector<float> histo = getImageHisto( myImage, myNbOfPoints[myImage], binSize ) ;
  AObject * img = 
    RoiManagementActionSharedData::instance()->getObjectByName( AObject::VOLUME,
								myImage ) ;
  float imageMin = 0;
  GLComponent  *gl = img->glAPI();
  if( gl && gl->glNumTextures() > 0 )
  {
    GLComponent::TexExtrema  & te = gl->glTexExtrema( 0 );
    if( !te.minquant.empty() )
      imageMin = te.minquant[0] ;
  }

#if QWT_VERSION >= 0x050000
  QwtPlotCurve * imageHisto = new QwtPlotCurve;
  imageHisto->attach( _private->myPlotArea );
#else
  QwtPlotCurve * imageHisto = new QwtPlotCurve( _private->myPlotArea ) ;
#endif
  double * x = new double[myNbOfBins] ;
  double * y = new double[myNbOfBins] ;

  myHistoMax = 0 ;

  int binIgnoredForMax ;

  if( myIgnoreForMax != 0. )
    binIgnoredForMax = int((myIgnoreForMax - imageMin) / binSize) ;
  else
    binIgnoredForMax = 0 ;

  for( int i = binIgnoredForMax ; i < myNbOfBins ; ++i ){
    x[i] = double(i* binSize) + imageMin ;
    y[i] = double(histo[i]) ;

    if( y[i] > myHistoMax )
      myHistoMax = y[i] ;
  }

  for( int i = 0 ; i < binIgnoredForMax ; ++i ){
    x[i] = double(i* binSize) + imageMin ;
    if(myHistoMax)
      y[i] = min( double(histo[i]), myHistoMax ) ;
    else
      y[i] = double(histo[i]) ;
  }


#if QWT_VERSION >= 0x050000
  imageHisto->setStyle( QwtPlotCurve::Lines );
  imageHisto->setCurveFitter( new QwtSplineCurveFitter );
#else
  imageHisto->setStyle( QwtPlotCurve::Spline );
#endif
#if QWT_VERSION >= 0x060200
  imageHisto->setData( new QwtCPointerData<double>( x, y, myNbOfBins ) );
#elif QWT_VERSION >= 0x060000
  imageHisto->setData( new QwtCPointerData( x, y, myNbOfBins ) );
#else
  imageHisto->setData( x, y, myNbOfBins ) ;
#endif
  imageHisto->setTitle( myImage.c_str() ) ;
//   imageHisto->setAxis( 0, 1 ) ;

//   imageHisto->setData( x, y, myNbOfBins ) ;
#if QWT_VERSION >= 0x050000
  _private->myImageHistoKey = imageHisto;
#else
  _private->myImageHistoKey = _private->myPlotArea->insertCurve( imageHisto ) ;
#endif

//    _private->myImageHistoKey = _private->myPlotArea->insertCurve( myImage.c_str() ) ;
//   _private->myPlotArea->setCurveData(_private->myImageHistoKey, x, y, myNbOfBins ) ;

#if QWT_VERSION >= 0x050000
//   if( !_private->myMarkerMax )
//     {
//       _private->myMarkerMax = new QwtPlotMarker;
//       _private->myMarkerMax->attach( _private->myPlotArea );
//       _private->myMarkerMax->setLinePen( QPen( QColor( 20, 0, 127 ) ) );
//       _private->myMarkerMax->setLineStyle( QwtPlotMarker::VLine );
//       _private->myMarkerMax->setXValue( myHighLevel );
// 
//       _private->myMarkerMin = new QwtPlotMarker;
//       _private->myMarkerMin->attach( _private->myPlotArea );
//       _private->myMarkerMin->setLinePen( QPen( QColor( 20, 0, 127 ) ) );
//       _private->myMarkerMin->setLineStyle( QwtPlotMarker::VLine );
//       _private->myMarkerMin->setXValue( myLowLevel );
//     }

#else

  _private->myMaxMarkerKey = _private->myPlotArea->insertLineMarker( tr("High"), QwtPlot::xBottom ) ;
  _private->myPlotArea->setMarkerLinePen(_private->myMaxMarkerKey, QPen( QColor( 20, 0, 127) ) ) ;
  _private->myPlotArea->setMarkerLineStyle( _private->myMaxMarkerKey, QwtMarker::VLine );
  _private->myPlotArea->setMarkerPos( _private->myMaxMarkerKey, myHighLevel, 0.0 ) ;
  
  
  _private->myMinMarkerKey = _private->myPlotArea->insertLineMarker( tr("Low"), QwtPlot::xBottom ) ;
  _private->myPlotArea->setMarkerLinePen(_private->myMinMarkerKey, QPen( QColor( 20, 0, 127) ) ) ;
  _private->myPlotArea->setMarkerLineStyle( _private->myMinMarkerKey, QwtMarker::VLine );
  _private->myPlotArea->setMarkerPos( _private->myMinMarkerKey, myLowLevel, 0.0 ) ;
  _private->myPlotArea->enableLegend(true) ;
#endif

  _private->myPlotArea->replot();
}

void 
RoiHistoPlot::showGraphHisto()
{
  if ( !myActivate )
    return ;
  
  Hierarchy * hie = 0 ;
  set<Hierarchy*> hierarchies ;
  set<AObject*> objs = theAnatomist->getObjects() ;
  set<AObject*>::iterator iterObj( objs.begin() ), lastObj( objs.end() ) ;
  
  while (iterObj != lastObj)
    {
      if( (*iterObj)->type( ) == Hierarchy::classType( ) )
	if( (hie = dynamic_cast<Hierarchy *>(*iterObj)) )
	hierarchies.insert(hie) ;
      
      ++iterObj ;
    }
  
//   QwtPlotCurve * imageHisto = _private->myPlotArea->curve( _private->myImageHistoKey ) ;
  _private->myPlotArea->detachItems() ;
//   if( imageHisto == 0 )
//     showImageHisto() ;
//    else 
//      _private->myImageHistoKey = _private->myPlotArea->insertCurve( imageHisto ) ;
  
  float binSize ;
  
  map< string, float > meanValue ;
  map< string, float > stdDev ;
  
  map<string, vector<float> > histos = getGraphHisto( myImage, myGraph, myNbOfPoints, 
						      meanValue, stdDev, binSize, 0 ) ;
  map<string, vector<float> >::iterator iter( histos.begin() ), last( histos.end() ) ;
  AObject * img = 
    RoiManagementActionSharedData::instance()->getObjectByName( AObject::VOLUME,
								myImage ) ;
  float imageMin = 0;
  GLComponent  *gl = img->glAPI();
  imageMin =  0;
  if( gl && gl->glNumTextures() > 0 )
  {
    GLComponent::TexExtrema  & te = gl->glTexExtrema( 0 );
    if( !te.minquant.empty() )
      imageMin = te.minquant[0] ;
  }
  while( iter != last ){
#if QWT_VERSION >= 0x050000
    QwtPlotCurve * regionHisto = new QwtPlotCurve;
    regionHisto->attach( _private->myPlotArea );
#else
    QwtPlotCurve * regionHisto = new QwtPlotCurve( _private->myPlotArea ) ;
#endif
    double * x = new double[myNbOfBins] ;
    double * y = new double[myNbOfBins] ;
    

//     cout << "Region " << iter->first << endl ;
//     for(int i = 0 ; i < iter->second.size() ; ++ i )
//       cout << "NG " << i*binSize << "\t: " << (iter->second)[i] << " Pts"  << endl ;

    for( int i = 0 ; i < myNbOfBins ; ++i ){
      x[i] = double(i* binSize) + imageMin ;
      y[i] = double((iter->second)[i]) ;
    }
    
    regionHisto->setStyle( QwtPlotCurve::Lines ) ;
#if QWT_VERSION >= 0x060200
    regionHisto->setData( new QwtCPointerData<double>( x, y, myNbOfBins ) );
#elif QWT_VERSION >= 0x060000
    regionHisto->setData( new QwtCPointerData( x, y, myNbOfBins ) );
#else
    regionHisto->setData( x, y, myNbOfBins );
#endif
    regionHisto->setTitle( iter->first.c_str() ) ;
    
    set<Hierarchy*>::iterator hieIter( hierarchies.begin() ), hieLast( hierarchies.end() ) ;
    set<Tree *> res ;
    while( hieIter != hieLast ){
      res = (*hieIter)->tree()->getElementsWith( "name", iter->first ) ;
      if( !res.empty() )
	break ;
      ++hieIter ;
    }
    vector<int> col ;
    if( !res.empty() )
      (*(res.begin()))->getProperty( "color", col ) ;
    if( col.size() < 3 ){
      col.reserve(3) ;
      col.push_back( rand() % 256 ) ;
      col.push_back( rand() % 256 ) ;
      col.push_back( rand() % 256 ) ;
    }
    
    regionHisto->setPen( QColor(col[0], col[1], col[2] ) ) ;

#if QWT_VERSION < 0x050000
    _private->myPlotArea->insertCurve( regionHisto ) ;
#endif
    
    ++iter ;
  }

#if QWT_VERSION >= 0x050000
//   if( !_private->myMarkerMax )
//     {
//       _private->myMarkerMax = new QwtPlotMarker;
//       _private->myMarkerMax->attach( _private->myPlotArea );
//       _private->myMarkerMax->setLinePen( QPen( QColor( 20, 0, 127 ) ) );
//       _private->myMarkerMax->setLineStyle( QwtPlotMarker::VLine );
//       _private->myMarkerMax->setXValue( myHighLevel );
// 
//       _private->myMarkerMin = new QwtPlotMarker;
//       _private->myMarkerMin->attach( _private->myPlotArea );
//       _private->myMarkerMin->setLinePen( QPen( QColor( 20, 0, 127 ) ) );
//       _private->myMarkerMin->setLineStyle( QwtPlotMarker::VLine );
//       _private->myMarkerMin->setXValue( myLowLevel );
//     }

#else

  _private->myMaxMarkerKey = _private->myPlotArea->insertLineMarker( tr("High"), QwtPlot::xBottom ) ;
  _private->myPlotArea->setMarkerLinePen(_private->myMaxMarkerKey, QPen( QColor( 20, 0, 127) ) ) ;
  _private->myPlotArea->setMarkerLineStyle( _private->myMaxMarkerKey, QwtMarker::VLine );
  _private->myPlotArea->setMarkerPos( _private->myMaxMarkerKey, myHighLevel, 0.0 ) ;
  
  
  _private->myMinMarkerKey = _private->myPlotArea->insertLineMarker( tr("Low"), QwtPlot::xBottom ) ;
  _private->myPlotArea->setMarkerLinePen(_private->myMinMarkerKey, QPen( QColor( 20, 0, 127) ) ) ;
  _private->myPlotArea->setMarkerLineStyle( _private->myMinMarkerKey, QwtMarker::VLine );
  _private->myPlotArea->setMarkerPos( _private->myMinMarkerKey, myLowLevel, 0.0 ) ;
  
  _private->myPlotArea->enableLegend(true) ;
#endif

  _private->myPlotArea->replot();
}

void
RoiHistoPlot::saveHistos()
{
  
  QString filt = ControlWindow::tr( "Anatomist Histograms" ) + " (*.anahis)" ;
  QString capt = "Save histos" ;
  
  QString filename = QFileDialog::getSaveFileName( 0, capt, QString(),
    filt, 0 );
  if( !filename.isNull() )
    printHistos( filename.toStdString() ) ;
}

void 
RoiHistoPlot::update( const Observable * obs, void *)
{
//   const RoiChangeProcessor * proc = dynamic_cast< const RoiChangeProcessor *>(obs) ;
  bool show = false ;
//   if( proc ){
//     show = true ;
//   }
  
  const RoiManagementActionSharedData * man = dynamic_cast<const RoiManagementActionSharedData *>(obs) ;
  if(man){
    if( myImage != man->currentImage() ){
      myImage = man->currentImage() ;
      show = true ;
    }
    if( myGraph != man->currentGraph() ){
      myGraph = man->currentGraph() ;
      show = true ;
    }
  }
  if (myActivate && show )
    showImageHisto() ;

}

vector<float> 
RoiHistoPlot::getImageHisto( const string& image, int& nbOfPoints, float& binSize, int time )
{
  map< string, vector<float> >::iterator foundImageHisto( myImageHistos.find(image) ) ;
  AObject * img = 
    RoiManagementActionSharedData::instance()->getObjectByName( AObject::VOLUME,
								image ) ;

  if( !img  || !myNbOfBins )
    return vector<float>() ;
  
  float  imin = 0, imax = 1;
  GLComponent  *gl = img->glAPI();
  if( gl && gl->glNumTextures() > 0 )
  {
    GLComponent::TexExtrema  & te = gl->glTexExtrema( 0 );
    if( !te.minquant.empty() )
      imin = te.minquant[0] ;
    if( !te.maxquant.empty() )
      imax = te.maxquant[0] ;
  }
  float invBinSize = myNbOfBins / (imax - imin ) ;
  binSize = 1./invBinSize ;

  if( foundImageHisto != myImageHistos.end() ){
    nbOfPoints = myNbOfPoints[image] ;
    return foundImageHisto->second ;
  }
  
  vector<float> histo(myNbOfBins+1, 0.) ;
    
  vector<float> p( 4, 0. );
  p[3] = time;
  nbOfPoints = 0 ;

  vector<float> bmin, bmax, vs;
  img->boundingBox2D( bmin, bmax );
  vs = img->voxelSize();
  vector<int> dims( 3 );
  dims[0] = int( rint( bmax[0] / vs[0] - 0.5 ) );
  dims[1] = int( rint( bmax[1] / vs[1] - 0.5 ) );
  dims[2] = int( rint( bmax[2] / vs[2] - 0.5 ) );

  for( p[2] = 0 ; p[2] <= dims[2] ; ++(p[2]) )
    for( p[1] = 0 ; p[1] <= dims[1] ; ++(p[1]) )
      for( p[0] = 0 ; p[0] <= dims[0] ; ++(p[0]) )
      {
        ++(histo[(unsigned int)( (img->mixedTexValue( p ) - imin)
                                 * invBinSize + 0.5) ]);
        ++nbOfPoints ;
      }
  myImageHistos[image] = histo ;
  myNbOfPoints[image] = nbOfPoints ;
  
  return histo ;
}

vector<float> 
RoiHistoPlot::getRegionHisto( const string& image, 
			      const string& graph, const string& region, 
			      int& nbOfPoints,
			      float& meanValue, float& stdDev,
			      float& binSize, 
			      int time )
{
  AObject * img = 
    RoiManagementActionSharedData::instance()->getObjectByName( AObject::VOLUME,
								image ) ;

  
  AObject * grao = RoiManagementActionSharedData::instance()->getGraphObjectByName( graph,
										    region ) ;
  AGraphObject * graphObject = dynamic_cast<AGraphObject *>(grao) ;
  Bucket		*bk = 0;
  if( graphObject != 0 ){
    AGraphObject::iterator	ic, ec = graphObject->end();
    
    for( ic=graphObject->begin(); ic!=ec; ++ic )
      if( ( bk = dynamic_cast<Bucket *>( *ic ) ) )
	      break;
  }
 
  if( !img  || !bk || !myNbOfBins )
    return vector<float>() ;
  
  nbOfPoints = 0 ;
  Referential* imageRef = img->getReferential() ;
  Referential* buckRef = bk->getReferential() ;
  anatomist::Transformation * transf
      = theAnatomist->getTransformation( buckRef, imageRef ) ;

  float        imin = 0, imax = 1;
  GLComponent  *gl = img->glAPI();
  if( gl && gl->glNumTextures() > 0 )
  {
    GLComponent::TexExtrema  & te = gl->glTexExtrema( 0 );
    if( !te.minquant.empty() )
      imin = te.minquant[0] ;
    if( !te.maxquant.empty() )
      imax = te.maxquant[0] ;
  }
  float invBinSize = myNbOfBins / ( imax - imin ) ;

  
  vector<float> histo(myNbOfBins) ;
  
  BucketMap<Void>::Bucket::iterator 
    iter( bk->bucket()[0].begin() ), 
    last( bk->bucket()[0].end() ) ;
  
  Point3df p;
  vector<float> vpos( 4 );
  vpos[3] = time;
  meanValue = 0. ;
  stdDev = 0. ;
  float val ;
  vector<float> vs = graphObject->voxelSize();

  while ( iter != last)
  {
    if( transf )
    {
      p = transf->transform( Point3df( float( iter->first[0] * vs[0] ),
                                       float( iter->first[1] * vs[1] ),
                                       float( iter->first[2] * vs[2] ) ) );
      vpos[0] = p[0];
      vpos[1] = p[1];
      vpos[2] = p[2];
    }
    else
    {
      vpos[0] = iter->first[0] * vs[0];
      vpos[1] = iter->first[1] * vs[1];
      vpos[2] = iter->first[2] * vs[2];
    }
    val = img->mixedTexValue( vpos );
    meanValue += val ;
    stdDev += val * val ;
    ++histo[(unsigned int)( (val - imin) * invBinSize + 0.5 ) ] ;
    ++nbOfPoints ;
    ++iter ;
  }
  meanValue /= bk->bucket()[0].size() ;
  stdDev = sqrt( stdDev / bk->bucket()[0].size() - meanValue * meanValue ) ;
  binSize = 1. / invBinSize ;
  return histo ;
}

map< string, vector<float> > 
RoiHistoPlot::getGraphHisto( const string& image, 
			     const string& graph, 
			     map<string, int>& nbOfPoints,
			     std::map< std::string, float>& meanValue, 
			     std::map< std::string, float>& stdDeviation,
			     float& binSize,
			     int time )
{
  map< string, vector<float> > histos ;

  AObject * img = 
    RoiManagementActionSharedData::instance()->getObjectByName( AObject::VOLUME,
								image ) ;
  AObject * g = 
    RoiManagementActionSharedData::instance()->getObjectByName( AObject::GRAPH,
								graph ) ;
  
  AGraph * gr = dynamic_cast<AGraph *>( g ) ;

  
  if( !img  || !gr|| !myNbOfBins )
    return map< string, vector<float> >() ;

  AGraph::iterator iter( gr->begin() ), last( gr->end() ) ;
  string region ;
  while( iter != last )
    {
      AGraphObject * go = dynamic_cast<AGraphObject*>( *iter ) ;
      if( go ){
	Bucket * bk = RoiChangeProcessor::instance()->getCurrentRegion( 0 ) ;
	if( bk )
	  if( ! bk->bucket().empty() ){
	    go->attributed()->getProperty( "name", region ) ;
	    histos[region] = getRegionHisto( image, graph, region, nbOfPoints[region], 
					     meanValue[region], stdDeviation[region],
					     binSize, time ) ;
	  }
      }

      // Just to see
      int count = 0 ;
      for( vector<float>::size_type i = 0, n =  histos[region].size(); i < n ; 
           ++i )
	if( histos[region][i] != 0 )
	  ++count ;
     
      ++iter ;
    }  
  
  return histos ;
} 

void
RoiHistoPlot::printHistos( const string& filename )
{
  if( myImage == "" )
    return ;
  ofstream os( filename.c_str(), ios::app ) ;
  
  AObject * img = 
    RoiManagementActionSharedData::instance()->getObjectByName( AObject::VOLUME,
								myImage ) ;
  float imageMin = 0;
  GLComponent  *gl = img->glAPI();
  if( gl && gl->glNumTextures() > 0 )
  {
    GLComponent::TexExtrema  & te = gl->glTexExtrema( 0 );
    if( !te.minquant.empty() )
      imageMin = te.minquant[0] ;
  }

  vector<float> histo ;
  map<string, vector<float> >::iterator found( myImageHistos.find( myImage ) ) ;
//   map<string, float >::iterator found( myNbOfPoints.find( myImage ) ) ;
  
  float binSize ;
  int nbPoints ;
  if( found != myImageHistos.end() ){
    histo = found->second ;
    nbPoints = myNbOfPoints[myImage] ;
  }
  else
    histo = getImageHisto( myImage, nbPoints, binSize, 0 ) ;  
  
  float imageMean = 0., imageVar = 0., nbPts = 0 ;
  
  for( vector<float>::size_type i = 0, n = histo.size() ; i < n ; ++i ){
    imageMean += (i * binSize + imageMin) * histo[i] ;
    nbPts += histo[i] ;
  }
  imageMean /= float(nbPts) ;
  
  for( vector<float>::size_type i = 0, n = histo.size() ; i < n ; ++i )
    imageVar += (i * binSize + imageMin - imageMean) * (i * binSize + imageMin - imageMean) * histo[i] ;
  imageVar /= float(nbPts) ;
  
  
  os << myImage << " Mean Value = " << imageMean 
     << "\tSdt Deviation = " << imageVar << endl 
     << " histogram :" << endl  
     << setw(15) << "Bin" << "|"
     << setw(15) << "Color" << "|"
     << setw(15) << "Number" << "|"
     << setw(15) << "Percentage (%)" << endl ;
  for( vector<float>::size_type i = 0, n = histo.size() ; i < n ; ++i )
    os << setw(15) << i << "|"
       << setw(15) << i * binSize + imageMin << "|"
       << setw(15) << histo[i] << "|"
       << setw(15) << histo[i] / nbPoints * 100. << endl ;
  
  if( myGraph == ""){
    os.close();
    return ;
  }
  
  map<string, int  > nbOfPoints ;
  map<string, float > meanValue ;
  map<string, float > stdDev ;
 
  map<string, vector<float> > regionsHisto = getGraphHisto( myImage, myGraph, nbOfPoints, 
							    meanValue, stdDev,
							    binSize ) ;
  map<string, vector<float> >::iterator graphIter(regionsHisto.begin()), graphLast(regionsHisto.end()) ;
  
  os << endl << endl <<  myGraph << " histogram :" << endl ;
  
  while( graphIter != graphLast ){
    int currentNbOfPoints = nbOfPoints[graphIter->first] ;
    os << endl ; ;
    os << graphIter->first << " Mean Value = " << meanValue[graphIter->first] 
       << "\tSdt Deviation = " << stdDev[graphIter->first] << endl
       << "Histogram :" << endl 
       << setw(15) << "Bin" << "|"
       << setw(15) << "Color" << "|"
       << setw(15) << "Number" << "|"
       << setw(15) << "Percentage (%)" << endl ;
    for( vector<float>::size_type i = 0, n = graphIter->second.size(); i < n ; 
         ++i )
      os << setw(15) << i << "|"
	 << setw(15) << i * binSize + imageMin << "|"
	 << setw(15) << graphIter->second[i] << "|"
	 << setw(15) << graphIter->second[i] / currentNbOfPoints * 100. << endl ;
    
    ++graphIter ;
  }
  
  os.close();
}
