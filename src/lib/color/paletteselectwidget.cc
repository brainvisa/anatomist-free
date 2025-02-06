
#include <anatomist/color/paletteselectwidget.h>
#include <anatomist/color/paletteList.h>
#include <anatomist/application/Anatomist.h>
#include <qheaderview.h>


using namespace anatomist;
using namespace carto;
using namespace std;


PaletteSelectWidget::PaletteSelectWidget( QWidget *parent,
                                          const string & selected,
                                          bool allow_none )
  : QTableWidget( parent ),
    _init_selected( selected ),
    _allow_none( allow_none )
{
  setMinimumSize( 200, 200 );
  setSortingEnabled( true );
  setCornerButtonEnabled( false );
  setAlternatingRowColors( true );
  setSelectionBehavior( QTableWidget::SelectRows );
  setSelectionMode( QTableWidget::SingleSelection );
  setEditTriggers( QTableWidget::NoEditTriggers );
  verticalHeader()->setVisible( false );
  setIconSize( QSize( 64, 20 ) );

  fillPalettes();
  selectPalette( selected );

  connect( this, SIGNAL( itemSelectionChanged() ), SLOT( paletteChanged() ) );
}


PaletteSelectWidget::~PaletteSelectWidget()
{
}


void PaletteSelectWidget::fillPalettes()
{
  const list<rc_ptr<APalette> >	& pal = theAnatomist->palettes().palettes();
  list<rc_ptr<APalette> >::const_iterator	ip, fp=pal.end();
  int				i = 0;
  int ps = pal.size();

  if( _allow_none )
    ++ps;

  setRowCount( ps );
  setColumnCount( 2 );
  setHorizontalHeaderItem( 0, new QTableWidgetItem( tr( "name" ) ) );
  setHorizontalHeaderItem( 1, new QTableWidgetItem( tr( "look" ) ) );
  map<string, int> cols;

  if( _allow_none )
  {
    QTableWidgetItem *item = new QTableWidgetItem( "<None>" );
    setItem( 0, 0, item );
  }

  for( i=int(_allow_none), ip=pal.begin(); ip!=fp; ++ip, ++i )
  {
    QTableWidgetItem *item = new QTableWidgetItem( (*ip)->name().c_str() );
    setItem( i, 0, item );
    // icon
    QPixmap pix;
    fillPalette( *ip, pix );
    QTableWidgetItem *iconitem = new QTableWidgetItem;
    iconitem->setIcon( QIcon( pix ) );
    setItem( i, 1, iconitem );
    PropertySet & ph = (*ip)->header();
    if( ph.hasProperty( "display_properties" ) )
    {
      // fill columns
      Object props = ph.getProperty( "display_properties" );
      Object piter = props->objectIterator();
      while( piter->isValid() )
      {
        string pname = piter->key();
        string pval;
        try
        {
          pval = piter->currentValue()->getString();
        }
        catch( ... )
        {
          piter->next();
          continue;
        }
        map<string, int>::iterator ic = cols.find( pname );
        int col;
        if( ic == cols.end() )
        {
          col = columnCount();
          cols[ pname ] = col;
          setColumnCount( col + 1 );
          setHorizontalHeaderItem(
            col, new QTableWidgetItem( tr( pname.c_str() ) ) );
        }
        else
          col = ic->second;
        setItem( i, col, new QTableWidgetItem( pval.c_str() ) );
        piter->next();
      }
    }
  }
  setColumnWidth( 0, 160 );
  resizeColumnToContents( 1 );
  int ncol = columnCount();
  for( i=2; i<ncol; ++i )
    setColumnWidth( i, 20 );
//   cout << "pal width: " << width() << endl;
//   cout << "sizehint: " << sizeHint().width() << endl;
//   resizeRowsToContents();
//   if( columnWidth( 0 ) > sizeHint().width() - columnWidth( 1 ) )
//     setColumnWidth( 0, sizeHint().width() - columnWidth( 1 ) );
}


void PaletteSelectWidget::fillPalette( const rc_ptr<APalette> pal,
                                       QPixmap & pm )
{
  unsigned		dimpx = pal->getSizeX(), dimpy = pal->getSizeY();
  unsigned		dimx = dimpx, dimy = dimpy;
  unsigned		x, y;

  if( dimy < 32 )
    dimy = 32;
  if( dimx > 256 )
    dimx = 256;
  else if( dimx == 0 )
    dimx = 1;
  if( dimy > 256 )
    dimy = 256;

  float		facx = ((float) dimpx) / dimx;
  float		facy = ((float) dimpy) / dimy;
  AimsRGBA	rgb;

  QImage	im( dimx, dimy, QImage::Format_ARGB32 );

  for( y=0; y<dimy; ++y )
    for( x=0; x<dimx; ++x )
    {
      rgb = (*pal)( (unsigned) ( facx * x), (unsigned) ( facy * y ) );
      im.setPixel( x, y, qRgb( rgb.red(), rgb.green(), rgb.blue() ) );
    }
  pm.convertFromImage( im );
}


string PaletteSelectWidget::selectedPalette() const
{
  QTableWidgetItem* item = 0;
  QList<QTableWidgetItem *> selected = selectedItems();
  if( selected.count() == 0 )
    return "";
  item = selected.front();

  string		name = item->text().toStdString();
  if( _allow_none && name == "<None>" )
    return "";

  PaletteList		& pallist = theAnatomist->palettes();
  const rc_ptr<APalette> pal = pallist.find( name );

  if( !pal )
    return "";

  return name;
}


void PaletteSelectWidget::paletteChanged()
{
  string name = selectedPalette();

  emit paletteSelected( name );
}


void PaletteSelectWidget::selectPalette( const string & name )
{
  blockSignals( true );

  QTableWidgetItem *item = 0;
  QList<QTableWidgetItem *> items;
  if( _allow_none && ( name == "" || name == "<None>" ) )
  {
    item = QTableWidget::item( 0, 0 );
  }
  else
  {
    int i, n = rowCount();
    QString	qname( name.c_str() );
    items = findItems( qname, Qt::MatchCaseSensitive | Qt::MatchExactly );
    if( items.count() != 0 )
      item = items.front();
  }
  if( item )
  {
    setCurrentItem( item );
    item->setSelected( true );
    scrollToItem( item );
  }
  else
  {
    setCurrentItem( 0 );
    items = selectedItems();
    if( items.count() != 0 )
      item = items.front();
    if( item )
      item->setSelected( false );
  }
  blockSignals( false );
}

