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


#include <anatomist/application/fileDialog.h>
#include <anatomist/application/filedialogextension.h>
#include <qcombobox.h>
#include <aims/def/path.h>
#include <anatomist/application/globalConfig.h>
#include <anatomist/application/Anatomist.h>
#include <aims/io/finder.h>
#include <cartobase/stream/fileutil.h>
#include <soma-io/writer/pythonwriter.h>
#include <QGridLayout>
#include <QLabel>
#include <QTextBrowser>
#include <QPushButton>
#include <QComboBox>
#include <QVBoxLayout>
#include <QLineEdit>

using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;


struct AimsFileDialogExtension::Private
{
  Private();

  QTextBrowser* browser;
  QComboBox* type_combo;
  QString current_path;
  bool current_item_recognized;
  string options_url;
  Object options;
  map<pair<string, string>, vector<string> > possible_types_map;
  map<string, pair<string, string> > selected_types;
};


AimsFileDialogExtension::Private::Private()
  : browser( 0 ), type_combo( 0 ), current_item_recognized( false )
{
}


AimsFileDialogExtension::AimsFileDialogExtension( QWidget* parent)
  : QWidget( parent ), d( new Private )
{
  setupCustom( parent );
}


AimsFileDialogExtension::~AimsFileDialogExtension()
{
  delete d;
}


void AimsFileDialogExtension::setupCustom( QWidget* filedialog )
{
  QGridLayout *layout = new QGridLayout;
  setLayout( layout );
  int row = layout->rowCount();
  layout->addWidget( new QLabel( tr( "selected:" ) ), row, 0 );
  QTextBrowser *browser = new QTextBrowser;
  layout->addWidget( browser, row, 1 );
  QPushButton *editbt = new QPushButton( tr( "options..." ) );
  QComboBox *typecb = new QComboBox;
  typecb->setSizeAdjustPolicy( QComboBox::AdjustToContents );
  QVBoxLayout *rlay = new QVBoxLayout;
  layout->addLayout( rlay, row, 2 );
  rlay->addWidget( new QLabel( tr( "Load as:" ) ) );
  rlay->addWidget( typecb );
  rlay->addWidget( editbt );
  rlay->addStretch();
  d->browser = browser;
  d->type_combo = typecb;
  d->current_path = "";
  d->current_item_recognized = false;
  d->options_url = "";

  connect( filedialog, SIGNAL( currentChanged( const QString & ) ),
           this, SLOT( currentFileChanged( const QString & ) ) );
  connect( editbt, SIGNAL( clicked() ), this, SLOT( optionsClicked() ) );
}


void AimsFileDialogExtension::currentFileChanged( const QString & path )
{
  if( path == d->current_path )
    return;
  d->current_path = path;
  d->type_combo->clear();
  d->selected_types.clear();
  if( path == "" )
  {
    d->browser->setText( "" );
    return;
  }

  QString text = "<p><h4>" + path + "</h4></p>";
  Finder fi;
  bool ok = fi.check( path.toStdString() );
  d->current_item_recognized = ok;

  if( ok )
  {
    const GenericObject *header = dynamic_cast<const GenericObject *>(
      fi.header() );
    text += "<p><h4>" + tr( "Header information:" )
      + "</h4></p><p><table>  <thead>    <tr>      <th>"
      + tr( "Attribute:" )
      + "</th>      <th>"
      + tr( "Value:" )
      + "</th>    </tr>  </thead>  <tbody>";

    Object oit = header->objectIterator();
    for( ; oit->isValid(); oit->next() )
    {
      PythonWriter pw;
      stringstream sst;
      pw.attach( sst );
      pw.write( oit->currentValue(), false, false );
      text += QString( "<tr><td>" ) + oit->key().c_str() + "</td><td>"
        + sst.str().c_str() + "</td></tr>";
    }

    text += "  </tbody></table></p>";

    string ot = fi.objectType();
    string dt = fi.dataType();
    vector<string> pt = fi.possibleDataTypes();
    vector<string>::iterator is, es = pt.end(), it, et;
    for( is=pt.begin(); is!=es; ++is )
    {
      vector<string> types = typeIds( ot, *is );
      for( it=types.begin(), et=types.end(); it!=et; ++it )
      {
        d->type_combo->addItem( it->c_str() );
        d->selected_types[ *it ] = make_pair( ot, *is );
      }
      d->type_combo->setCurrentIndex( 0 );
    }
  }
  else
  {
    text += "<p>" + tr( "Unrecognized file." ) + "</p>";
  }
  d->browser->setText( text );
}


void AimsFileDialogExtension::optionsClicked()
{
  if( !d->current_item_recognized )
    return;
  QDialog opt_box;
  QVBoxLayout* layout = new QVBoxLayout;
  layout->addWidget( new QLabel( tr( "Load options (appended to URL):" ) ) );
  opt_box.setLayout( layout );
  QLineEdit *opt_line = new QLineEdit;
  opt_line->setText( d->options_url.c_str() );
  layout->addWidget( opt_line );
  connect( opt_line, SIGNAL( returnPressed() ), &opt_box, SLOT( accept() ) );
  bool res = opt_box.exec();
  if( res )
  {
    d->options_url = opt_line->text().toStdString();
  }
}


vector<string> AimsFileDialogExtension::typeIds( const string & ot,
                                                 const string & dt ) const
{
  map<pair<string, string>, vector<string> >::const_iterator im
    = d->possible_types_map.find( make_pair( ot, dt ) );
  if( im != d->possible_types_map.end() )
    return im->second;

  string aims_type = typeId( ot, dt );
  vector<string> ana_types;
  ana_types.push_back( aims_type );
  return ana_types;
}


string AimsFileDialogExtension::typeId( const string & ot,
                                        const string & dt ) const
{
  if( dt == "VOID" || dt == "any" )
    return ot;
  return ot + " of " + dt;
}


void AimsFileDialogExtension::setPossibleTypesMap(
  const AimsFileDialog::TypesMap & poss_types )
{
  d->possible_types_map = poss_types;
}


bool AimsFileDialogExtension::optionsValid() const
{
  return d->current_item_recognized;
}


string AimsFileDialogExtension::selectedTypeId() const
{
  return d->type_combo->currentText().toStdString();
}


pair<string, string> AimsFileDialogExtension::selectedType() const
{
  return d->selected_types[ selectedTypeId() ];
}


string AimsFileDialogExtension::selectedUrlOptions() const
{
  return d->options_url;
}


Object AimsFileDialogExtension::selectedOptions() const
{
  return d->options;
}


// ---

struct AimsFileDialog::Private
{
  Private();

  AimsFileDialogExtension *aims_ext;
  QPushButton *expand_button;
};


AimsFileDialog::Private::Private()
  : aims_ext( 0 ), expand_button( 0 )
{
}


AimsFileDialog::AimsFileDialog( QWidget *parent, Qt::WindowFlags flags )
  : QFileDialog( parent, flags ), d( new Private )
{
  setupCustom();
}


AimsFileDialog::~AimsFileDialog()
{
  delete d;
}


void AimsFileDialog::setupCustom()
{
  // setOptions( QFileDialog::DontUseNativeDialog );
  QGridLayout *layout = dynamic_cast<QGridLayout *>( QFileDialog::layout() );
  d->aims_ext = new AimsFileDialogExtension( this );
  d->aims_ext->hide();
  QPushButton *editoptbt = new QPushButton( "... v" );
  d->expand_button = editoptbt;
  layout->addWidget( editoptbt, layout->rowCount(), 2 );
  editoptbt->setSizePolicy( QSizePolicy::Preferred,
                            QSizePolicy::Fixed );
  editoptbt->setFixedHeight( 15 );
  layout->addWidget( d->aims_ext, layout->rowCount(), 0, 1, 3 );

  connect( editoptbt, SIGNAL( clicked() ), this, SLOT( showHideOptions() ) );
}


void AimsFileDialog::showHideOptions()
{
  if( d->aims_ext->isVisible() )
  {
    d->aims_ext->hide();
    d->expand_button->setText( "... v" );
  }
  else
  {
    d->aims_ext->show();
    d->expand_button->setText( "... ^" );
  }
}


void AimsFileDialog::setPossibleTypesMap( const TypesMap & poss_types )
{
  d->aims_ext->setPossibleTypesMap( poss_types );
}


bool AimsFileDialog::optionsValid() const
{
  return d->aims_ext->optionsValid();
}


string AimsFileDialog::selectedTypeId() const
{
  return d->aims_ext->selectedTypeId();
}


pair<string, string> AimsFileDialog::selectedType() const
{
  return d->aims_ext->selectedType();
}


string AimsFileDialog::selectedUrlOptions() const
{
  return d->aims_ext->selectedUrlOptions();
}


Object AimsFileDialog::selectedOptions() const
{
  return d->aims_ext->selectedOptions();
}


// ---

QFileDialog & anatomist::fileDialog()
{
  static QFileDialog *_fdialog = 0;
  QString path;
  QRect geom;
  bool oldone = false;

  if( _fdialog )
  {
    oldone = true;
    QStringList filenames = _fdialog->selectedFiles();
    if( !filenames.empty() )
    {
      path = FileUtil::dirname( filenames.begin()->toUtf8().data() ).c_str();
      geom = _fdialog->geometry();
    }
    delete _fdialog;
  }

  _fdialog = new AimsFileDialog( 0, Qt::Dialog );
  _fdialog->setWindowModality( Qt::ApplicationModal );
  if( oldone )
  {
    if( !path.isEmpty() )
      _fdialog->setDirectory( path );
    _fdialog->setGeometry( geom );
  }
  else
    _fdialog->setDirectory( "." );

  GlobalConfiguration       *cfg = theAnatomist->config();
  string            cpath;

  QStringList hist;
  string            p = Path::singleton().hierarchy() + '/';
  hist += p.c_str();

  if( cfg && cfg->getProperty( "path_list", cpath ) )
  {
    string::size_type     pos;
    while( !cpath.empty() )
    {
      pos = cpath.find( FileUtil::pathSeparator() );
      if( pos == string::npos )
        pos = cpath.length();
      if( pos != 0 )
      {
        p = cpath.substr( 0, pos );
        if( p[p.length() - 1] != '/' )
          p += '/';
        hist += p.c_str();
      }
      cpath.erase( 0, pos+1 );
    }
  }
  _fdialog->setHistory( hist );

  return( *_fdialog );
}
