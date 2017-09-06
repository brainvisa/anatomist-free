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
#include <anatomist/reference/wChooseReferential.h>
#include <anatomist/object/actions.h>
#include <anatomist/object/objectmenu.h>
#include <anatomist/color/wMaterial.h>
#include <anatomist/color/wRendering.h>
#include <anatomist/object/texturewin.h>
#include <anatomist/mobject/wFusion2D.h>
#include <anatomist/mobject/wFusion3D.h>
#include <anatomist/commands/cReloadObject.h>
#include <anatomist/commands/cSaveObject.h>
#include <anatomist/commands/cExportTexture.h>
#include <anatomist/commands/cExtractTexture.h>
#include <anatomist/commands/cGenerateTexture.h>
#include <anatomist/commands/cAddObject.h>
#include <anatomist/commands/cCreateWindow.h>
#include <anatomist/commands/cAssignReferential.h>
#include <anatomist/commands/cLoadGraphSubObjects.h>
#include <anatomist/color/wObjPalette.h>
#include <anatomist/processor/Processor.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/browser/stringEdit.h>
#include <anatomist/control/wControl.h>
#include <anatomist/graph/pythonAObject.h>
#include <anatomist/graph/Graph.h>
#include <anatomist/reference/Referential.h>
#include <anatomist/reference/Transformation.h>
#include <anatomist/reference/transfSet.h>
#include <anatomist/application/globalConfig.h>
#include <anatomist/object/objectutils.h>
#include <aims/resampling/standardreferentials.h>
#include <aims/graph/graphmanip.h>
#include <cartobase/stream/fileutil.h>
#include <qfiledialog.h>
#include <qcursor.h>
#include <qmessagebox.h>


using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;


void ObjectActions::fileReload( const set<AObject *> & obj )
{
  set<AObject *>			ol;
  set<AObject *>::const_iterator	io, fo=obj.end();

  for( io=obj.begin(); io!=fo; ++io )
    ol.insert( *io );

  ReloadObjectCommand *cmd = new ReloadObjectCommand( ol );
  theProcessor->execute( cmd );
  if( !cmd->failedReloads().empty() )
  {
    QString names;
    set<AObject *>::const_iterator ifl, efl = cmd->failedReloads().end();
    for( ifl=cmd->failedReloads().begin(); ifl!=efl; ++ifl )
      names += QString( "<br/>" ) + (*ifl)->fileName().c_str();
    QMessageBox::warning( 0, ControlWindow::tr( "Reload failed" ),
      QString( "<html>" ) +
      ControlWindow::tr( "Could not reload the following objects:" )
      + names + "</html>", QMessageBox::Ok, QMessageBox::Ok );
  }
}


void ObjectActions::colorPalette( const set<AObject *> & obj )
{
  /*set<AObject *>	ol;
  AObject		*o;

  ol.insert( o = *obj.begin() );*/
 APaletteWinFactory::newPaletteWin( obj );
}


void ObjectActions::colorMaterial( const set<AObject *> & obj )
{
  set<AObject *>			ol;
  set<AObject *>::const_iterator	io, fo=obj.end();

  for( io=obj.begin(); io!=fo; ++io )
    ol.insert( *io );

  MaterialWindow* mw = new MaterialWindow( ol, theAnatomist->getQWidgetAncestor(), "Material Window", Qt::Window );
  mw->show();
}


void ObjectActions::colorRendering( const set<AObject *> & obj )
{
  set<AObject *>			ol;
  set<AObject *>::const_iterator	io, fo=obj.end();

  for( io=obj.begin(); io!=fo; ++io )
    ol.insert( *io );

  RenderingWindow* mw = new RenderingWindow( ol, theAnatomist->getQWidgetAncestor(), "Rendering Window", Qt::Window );
  mw->show();
}


void ObjectActions::referentialLoad( const set<AObject *> & obj )
{
  ChooseReferentialWindow *w 
    = new ChooseReferentialWindow( obj, "Choose Referential Window" );
  w->setAttribute( Qt::WA_DeleteOnClose );
  w->show();
}


void ObjectActions::fusion2DControl( const set<AObject *> & obj )
{
  int MAXLEN = 300;
  theAnatomist->config()->getProperty( "object_names_list_max_size", MAXLEN );

  Fusion2DWindow	*fw
    = new Fusion2DWindow( obj, theAnatomist->getQWidgetAncestor(),
      ObjectUtils::catObjectNames( obj, MAXLEN ).c_str(), Qt::Window );
  fw->show();
}


void ObjectActions::fusion3DControl( const set<AObject *> & obj )
{
  int MAXLEN = 300;
  theAnatomist->config()->getProperty( "object_names_list_max_size", MAXLEN );

  Fusion3DWindow	*fw
    = new Fusion3DWindow( obj, theAnatomist->getQWidgetAncestor(),
                          ObjectUtils::catObjectNames( obj, MAXLEN ).c_str() );
  fw->setWindowFlags(Qt::Window);
  fw->show();
}


void ObjectActions::textureControl( const set<AObject *> & obj )
{
  int MAXLEN = 300;
  theAnatomist->config()->getProperty( "object_names_list_max_size", MAXLEN );

  QTextureWin	*tp
    = new QTextureWin( obj, theAnatomist->getQWidgetAncestor(),
                       ObjectUtils::catObjectNames( obj, MAXLEN ).c_str(),
                       Qt::Window );
  tp->show();
}


void ObjectActions::saveStatic( const set<AObject *> & obj )
{
  specificSaveStatic(obj, theAnatomist->objectsFileFilter(), "Save object" ) ;
}

string 
ObjectActions::specificSaveStatic( const set<AObject *> & obj, 
                                   const string& filter, 
                                   const string & caption )
{
  //cerr << "ObjectActions::specificSaveStatic\n";
//   if( obj.size() > 1 )
//   {
//     QMessageBox::critical( 0, ControlWindow::tr( "Save object" ),
//                             ControlWindow::tr( "Save one object at a time !"
//                                               ) );
//     return "";
//   }

  AObject *object = 0;
  if( obj.size() == 1 )
    object = *obj.begin();
  
  QString	filt = filter.c_str() ;
  QString	capt = caption.c_str() ;
  QString	initial = QString::null;
  if( object )
  {
    if( !object->fileName().empty() )
      initial = object->fileName().c_str();
    else if( !object->name().empty() )
      initial = object->name().c_str();
  }
  /* cout << "specificSaveStatic filename: "
     << ( initial.isNull() ? "<Null>" : initial.toStdString() ) << endl; */

  QString filename = QFileDialog::getSaveFileName( 0, "Save object file",
    initial, filt );
  if( filename != QString::null )
  {
    if( FileUtil::fileStat( filename.toStdString() ).find( '+' ) != string::npos
        && QMessageBox::information
        ( 0, ControlWindow::tr( "Overwrite File ?" ),
          ControlWindow::tr( "A file called %1 already exists."
                              "Do you want to overwrite it?").arg( filename ),
          ControlWindow::tr("&Yes"), ControlWindow::tr("&No"),
          QString::null, 0, 1 ) )
      return "";
    SaveObjectCommand	*c
      = new SaveObjectCommand( obj, filename.toStdString() );
    theProcessor->execute( c );
    return filename.toStdString() ;
  }
  return "" ;
}

void ObjectActions::renameObject( const set<AObject *> & obj )
{
  if( !obj.empty() )
    {
      AObject	*o = *obj.begin();
      string	newname;
      if( !askName( newname, AObject::objectTypeName( o->type() ), 
		    o->name() ) )
	return;

      set<AObject *>::const_iterator	io, eo = obj.end();

      for( io=obj.begin(); io!=eo; ++io )
	{
	  o = *io;
	  o->setName( newname );
	  theAnatomist->NotifyObjectChange( o ) ;
	}
    }
}


bool ObjectActions::askName( string & newname, const string & type, 
			     const string& originalName )
{
  string message( "Enter new " );
  message += type ;
  message += string( " name" ) ;
  QPoint	pos = QCursor::pos();
  QStringEdit	*nameSetter 
    = new QStringEdit( originalName.c_str(), pos.x(), pos.y(), -1, -1, theAnatomist->getQWidgetAncestor(), 
		       message.c_str() );

  if( nameSetter->exec() )
    {
      newname = nameSetter->text();
      return( true );
    }
  return( false );
}


void ObjectActions::saveTexture( const set<AObject *> & obj )
{
  specificSaveTexture(obj, "Texture (*.tex)", "Save texture" ) ;
}

string 
ObjectActions::specificSaveTexture( const set<AObject *> & obj, 
                                    const string & filter, 
                                    const string & caption )
{
  if( obj.size() > 1 ) 
    {
      cerr << "Save one object at a time !" << endl;
      return "";
    }
  AObject *object = *obj.begin();
  
  QString filt = filter.c_str() ;
  QString capt = caption.c_str() ;
  
  QString filename = QFileDialog::getSaveFileName( 0, capt, QString::null,
                                                   filt );
  if ( filename != QString::null )
    {
      ExportTextureCommand	*c 
        = new ExportTextureCommand( object, filename.toStdString() );
      theProcessor->execute( c );
      return filename.toStdString() ;
    }
  return "" ;
}


void ObjectActions::extractTexture( const set<AObject *> & obj )
{
  if( obj.size() != 1 ) 
    {
      cerr << "Save one object at a time !" << endl;
      return;
    }

  ExtractTextureCommand	*c 
    = new ExtractTextureCommand( *obj.begin() );
  theProcessor->execute( c );
}


void ObjectActions::generateTexture1D( const std::set<AObject *> & obj )
{
  AObject	*ao = 0;
  if( !obj.empty() )
    ao = *obj.begin();

  GenerateTextureCommand	*c 
    = new GenerateTextureCommand( ao );
  theProcessor->execute( c );
}


void ObjectActions::generateTexture2D( const std::set<AObject *> & obj )
{
  AObject	*ao = 0;
  if( !obj.empty() )
    ao = *obj.begin();

  GenerateTextureCommand	*c 
    = new GenerateTextureCommand( ao, -1, 2 );
  theProcessor->execute( c );
}


void ObjectActions::addGraphWithoutChildren(const std::set<AObject *> &obj)
{
  using carto::shared_ptr;

  theAnatomist->setCursor( Anatomist::Working );

  set<AObject *>::const_iterator        io, eo = obj.end();
  set<AWindow*> winL = theAnatomist->getControlWindow()->selectedWindows();
  if (winL.size() == 0) winL = theAnatomist->getWindows();
  set<AWindow*>::iterator iw=winL.begin(), ew = winL.end(), iw2;
  // filter out non-3D windows
  while( iw!=ew )
  {
    if( (*iw)->type() != AWindow::WINDOW_3D
          && (*iw)->type() != AWindow::WINDOW_2D )
    {
      iw2 = iw;
      ++iw;
      winL.erase( iw2 );
    }
    else
      ++iw;
  }
  if( winL.empty() )
  {
    // cout << "displayGraphWithoutChildren - new window\n";
    CreateWindowCommand *c = new CreateWindowCommand( "3D" );
    theProcessor->execute( c );
    winL.insert( c->createdWindow() );
  }

  shared_ptr<AObject> ao;

  AddObjectCommand    *c = new AddObjectCommand(obj, winL, false, false);
  theProcessor->execute(c);

  theAnatomist->setCursor( Anatomist::Normal );
}


void ObjectActions::displayGraphChildren(const std::set<AObject *> &obj)
{
  using carto::shared_ptr;

  theAnatomist->setCursor( Anatomist::Working );

  set<AObject *>::const_iterator	io, eo = obj.end();
  set<AWindow*> winL = theAnatomist->getControlWindow()->selectedWindows();
  if (winL.size() == 0)	winL = theAnatomist->getWindows();
  set<AWindow*>::iterator iw=winL.begin(), ew = winL.end(), iw2;
  // filter out non-3D windows
  while( iw!=ew )
  {
    if( (*iw)->type() != AWindow::WINDOW_3D
          && (*iw)->type() != AWindow::WINDOW_2D )
    {
      iw2 = iw;
      ++iw;
      winL.erase( iw2 );
    }
    else
      ++iw;
  }
  if( winL.empty() )
  {
    // cout << "displayGraphChildren - new window\n";
    CreateWindowCommand *c = new CreateWindowCommand( "3D" );
    theProcessor->execute( c );
    winL.insert( c->createdWindow() );
  }

  shared_ptr<AObject> ao;

  for( io=obj.begin(); io!=eo; ++io )
  {
    AGraph  *ag = dynamic_cast<AGraph *>( *io );
    if( !ag )
      continue;
    Graph *g = ag->graph();
    if( !g )
      continue;

    Graph::const_iterator iv, ev = g->end();
    set<AObject *>      objL;
    objL.insert( *io ); // insert graph too
    for( iv=g->begin(); iv!=ev; ++iv )
      if( (*iv)->getProperty( "ana_object", ao ) )
        objL.insert( ao.get() );

    AddObjectCommand	*c = new AddObjectCommand(objL, winL);
    theProcessor->execute(c);
  }

  theAnatomist->setCursor( Anatomist::Normal );
}


void ObjectActions::displayGraphRelations(const std::set<AObject *> &obj)
{
  using carto::shared_ptr;

  theAnatomist->setCursor( Anatomist::Working );

  set<AObject *>::const_iterator        io, eo = obj.end();
  set<AWindow*> winL = theAnatomist->getControlWindow()->selectedWindows();
  if (winL.size() == 0) winL = theAnatomist->getWindows();
  set<AWindow*>::iterator iw=winL.begin(), ew = winL.end(), iw2;
  // filter out non-3D windows
  while( iw!=ew )
  {
    if( (*iw)->type() != AWindow::WINDOW_3D
          && (*iw)->type() != AWindow::WINDOW_2D )
    {
      iw2 = iw;
      ++iw;
      winL.erase( iw2 );
    }
    else
      ++iw;
  }
  if( winL.empty() )
  {
    theAnatomist->setCursor( Anatomist::Normal );
    return;
  }

  shared_ptr<AObject> ao;

  for( io=obj.begin(); io!=eo; ++io )
  {
    AGraph  *ag = dynamic_cast<AGraph *>( *io );
    if( !ag )
      continue;
    Graph *g = ag->graph();
    if( !g )
      continue;

    // take windows ag is in
    set<AWindow *>  win2;
    for( iw=winL.begin(); iw!=ew; ++iw )
      if( (*iw)->hasObject( ag ) )
        win2.insert( *iw );
    if( win2.empty() )
      continue;

    const set<Edge *> & edg = g->edges();
    set<Edge *>::const_iterator ie, ee = edg.end();
    set<AObject *>      objL;
    for( ie=edg.begin(); ie!=ee; ++ie )
      if( (*ie)->getProperty( "ana_object", ao ) )
        objL.insert( ao.get() );
    if( !objL.empty() )
    {
      AddObjectCommand    *c = new AddObjectCommand(objL, win2);
      theProcessor->execute(c);
    }
  }

  theAnatomist->setCursor( Anatomist::Normal );
}


void ObjectActions::loadGraphSubObjects(const std::set<AObject *> &obj)
{
  LoadGraphSubObjectsCommand *c = new LoadGraphSubObjectsCommand( obj, 3 );
  theProcessor->execute( c );
}


void ObjectActions::setAutomaticReferential( const set<AObject*> & obj )
{
  int userLevel = theAnatomist->userLevel();
  set<AObject *>::const_iterator        io, eo = obj.end();

  bool commonScannerRef = false;
  try
  {
    Object x = theAnatomist->config()->getProperty(
      "commonScannerBasedReferential" );
    if( !x.isNull() )
      commonScannerRef = (bool) x->getScalar();
  }
  catch( ... )
  {
  }

  for( io=obj.begin(); io!=eo; ++io )
  {
    PythonAObject *go = dynamic_cast<PythonAObject *>( *io );
    GenericObject  *ps = 0;
    unsigned  i, n, j, nj;
    Referential *ownref = 0;

    if( go && (ps = go->attributed() ) )
    {
      vector<Motion>        vmot;
      vector<Referential*>  vref;
      Referential           *ref = 0;
      int                   mniref = -1;
      int                   acpcref = -1;

      string uid;
      if( ps->getProperty( "referential", uid ) )
        ownref = Referential::referentialOfUUID( uid );

      // Nifti-like transformations
      vector<string>    refs;
      Object            transs;

      if( ps->getProperty( "referentials", refs )
          && ( transs = ps->getProperty( "transformations" ) )
          && refs.size() >= 1 && transs->size() >= 1 )
      {
        // check that transformations is a list of trans and not a single trans
        try
        {
          if( transs->size() != 0 )
          {
            Object it = transs->objectIterator();
            if( it->isValid() )
            {
              Object o = it->currentValue();
              it = o->objectIterator();
              if( !it->isValid() )
              {
                Object transs1 = transs;
                transs = Object::value( vector<Object>( 1, transs ) );
                cout << "Warning: non-conform transformations list in volume "
                  "header (single transformation)\n";
              }
            }
          }
        }
        catch( runtime_error )
        {
          Object transs1 = transs;
          transs = Object::value( vector<Object>( 1, transs ) );
          cout << "Warning: non-conform transformations list in volume header "
            "(single transformation)\n";
        }
        Object      it;
        n = refs.size();
        // cout << "nifti transfo: " << n << endl;
        for( it=transs->objectIterator(), i=0; i<n && it->isValid();
             ++i, it->next() )
        {
          bool        refcreated = false;
          // find referential
          ref = 0;
          string sref = refs[i];
          if( sref == StandardReferentials::mniTemplateReferential()
              || ( i == 0 && ownref == Referential::mniTemplateReferential() )
            )
          {
            // cout << "MNI\n";
            if( mniref < 0 )
            {
              // cout << "assign MNI\n";
              ref = Referential::mniTemplateReferential();
              mniref = vref.size();
            }
            else
            {
              string n = StandardReferentials::mniTemplateReferential()
                  + string( "_other_" ) + (*io)->name();
              ref = Referential::referentialOfName( n );
              if( !ref )
              {
                ref = new Referential;
                ref->header().setProperty( "name", n );
                refcreated = true;
              }
            }
          }
          else if( sref == StandardReferentials::acPcReferential() )
          {
            // cout << "ACPC\n";
            if( acpcref < 0 )
            {
              ref = Referential::acPcReferential();
              acpcref = vref.size();
            }
            else
            {
              string n = StandardReferentials::acPcReferential()
                  + string( "_other_" ) + (*io)->name();
              ref = Referential::referentialOfName( n );
              if (!ref )
              {
                ref = new Referential;
                ref->header().setProperty( "name", n );
                refcreated = true;
              }
            }
          }
          else
          {
            if( commonScannerRef && sref
              == StandardReferentials::commonScannerBasedReferential() )
              sref = StandardReferentials::commonScannerBasedReferentialID();

            // cout << "unspecified ref\n";
            carto::UUID uid( sref );
            if( sref
              == StandardReferentials::commonScannerBasedReferentialID() )
            {
              sref = "Scanner-based anatomical coordinates";
              ref = Referential::referentialOfUUID( sref );
            }
            else if( sref == "Talairach" )
            {
              // GIFTI Talairach
              ref = Referential::giftiTalairachReferential();
            }
            else if( uid.toString() != sref )
            {
                // sref doesn't correspond to an UUID, so it is not unique
              sref = sref + " for " + (*io)->name();
              ref = 0;
              uid = carto::UUID();
            }
            else
              ref = Referential::referentialOfUUID( sref );
            if( !ref )
            {
              ref = Referential::referentialOfName( sref );
            }
            if( !ref )
            {
              ref = new Referential;
              ref->header().setProperty( "name", sref );
              refcreated = true;
              if( !uid.isNull() )
                ref->header().setProperty( "uuid", uid.toString() );
            }
          }
          if( ref )
          {
            // if the current (target) referential is the same as the
            // referential property (ownref), then don't assign ownref to
            // the object
            if( ref == ownref )
            {
              ownref = 0;
              uid.clear();
            }
            // cout << "ref OK, nj: " << vmot.size() << endl;
            // cout << "vref: " << vref.size() << endl;
            Motion  m( it->currentValue() );
            // look if an identical transformation has already been specified
            for( j=0, nj=vmot.size(); j<nj; ++j )
              if( m == vmot[j] )
              {
                // cout << "identical motion already used\n";
                if( ref == Referential::mniTemplateReferential() )
                {
                  // cout << "MNI\n";
                  // MNI ref has priority: replace the older one
                  if( vref[j] != Referential::mniTemplateReferential()
                      && vref[j] != Referential::acPcReferential() )
                    delete vref[j];
                  vref[j] = ref;
                  mniref = j;
                }
                else if( ref == Referential::acPcReferential() )
                {
                  // cout << "ACPC\n";
                  if( vref[j] != Referential::mniTemplateReferential() )
                  {
                    // replace the older ref with acpc
                    if( vref[j] != Referential::mniTemplateReferential()
                        && vref[j] != Referential::acPcReferential() )
                      delete vref[j];
                    vref[j] = ref;
                    acpcref = j;
                  }
                  else
                    if( acpcref == (int) i )
                      // forget the current acpc ref
                      acpcref = -1;
                }
                else
                {
                  // cout << "unspecified, bis\n";
                  // unspecified ref: just forget about it
                  if( refcreated )
                    delete ref;
                  ref = 0;
                }
                // in any case, remove the last ref in list
                break;
              }
            // cout << "j: " << j << endl;
            if( j == nj )
            {
              // no identical transformation: store it
              vmot.push_back( m );
              vref.push_back( ref );
            }
          }
        }
      }
      else // not referentials/transformations properties
      {
        AGraph *ag = dynamic_cast<AGraph *>( go );
        if( ag && ps->hasProperty( "Talairach_translation" ) )
        {
          AffineTransformation3d m = GraphManip::talairach( *ag->graph() );
          if( !m.isIdentity() )
          {
            if( !ownref )
              ownref = new Referential;
            vref.push_back( Referential::acPcReferential() );
            vmot.push_back( m );
          }
        }
      }

      if( ownref )
        (*io)->setReferential( ownref );

      ATransformSet *ts = ATransformSet::instance();

      /* at this point all vmot should be different and lead to different
      refs. But if two common refs are involved, then a loop in the
      transformation graph will be made. We must break it by duplicating
      referentials and only link one of them to a common referential.
      */
      if( mniref >= 0 && acpcref >= 0 )
      {
        // cout << "duplicate MNI/Tal/ACPC refs\n";
        // duplicate refs
        set<AObject *> so;
        set<AWindow *> sw;
        ref = new Referential;
        ref->header().setProperty( "name",
          Referential::mniTemplateReferential()->header().getProperty(
            "name" )->getString() + "_" + (*io)->name() );
        // link MNI refs
        Transformation *t
            = theAnatomist->getTransformation( ref, vref[ mniref ] );
        if( !t )
        {
          t = new Transformation( ref, vref[ mniref ] );
          t->registerTrans();
        }
        vref[ mniref ] = ref;
        ref = new Referential;
        ref->header().setProperty( "name",
          Referential::acPcReferential()->header().getProperty(
            "name" )->getString() + "_" + (*io)->name() );
        vref[ acpcref ] = ref;
      }

      n = vmot.size();
      if( vref.size() < n )
        n = vref.size();
      /* cout << "n: " << n << ", vmot: " << vmot.size() << ", vref: "
      << vref.size() << endl; */
      for( i=0; i<n; ++i )
      {
        // cout << "extract transfo\n";
        ref = vref[i];
        set<Transformation *> trs = ts->transformationsWith( ref );
        set<Transformation *>::iterator       it, et = trs.end();
        Transformation        *t;

        for( it=trs.begin(); it!=et; ++it )
        {
          t = *it;
          if( !t->isGenerated() && t->destination() == ref
              && t->motion() == vmot[i] )
            {
              // found existing transformation
              (*io)->setReferential( (*it)->source() );
              (*io)->setChanged();
              (*io)->notifyObservers( theAnatomist );
              continue;
            }
        }

        if( !ownref )
        {
          ownref = Referential::referentialOfName( (*io)->name() );
          if( !ownref )
          {
            ownref = new Referential;
            ownref->header().setProperty( "name", (*io)->name() );
          }
          if( !uid.empty() )
            ownref->header().setProperty( "uuid", uid );
          (*io)->setReferential( ownref );
        }
        t = theAnatomist->getTransformation( ownref, ref );
        if( !t || !t->isGenerated() )
        {
          if( t )
            delete t;
          t = new Transformation( ownref, ref );
          t->motion() = vmot[i];
          t->registerTrans();
          (*io)->setChanged();
          (*io)->notifyObservers( theAnatomist );
        }
      }

      if( userLevel >= 2 )
        try
        {
          Object stom = ps->getProperty( "storage_to_memory" );
          Motion  m( stom ), mi = m.inverse();
          Point3df vs = (*io)->VoxelSize(),
          vss = mi.transform( vs ) - mi.transform( Point3df( 0, 0, 0 ) );
          vss = Point3df( fabs( vss[0] ), fabs( vss[1] ), fabs( vss[2] ) );
          Motion vm;
          vm.rotation()(0,0) = vs[0];
          vm.rotation()(1,1) = vs[1];
          vm.rotation()(2,2) = vs[2];
          m = vm * m;
          vm.rotation()(0,0) = 1. / vss[0];
          vm.rotation()(1,1) = 1. / vss[1];
          vm.rotation()(2,2) = 1. / vss[2];
          m *= vm;
          if( !ownref )
          {
            ownref = Referential::referentialOfName( (*io)->name() );
            if( !ownref )
            {
              ownref = new Referential;
              ownref->header().setProperty( "name", (*io)->name() );
            }
            (*io)->setReferential( ownref );
          }
          string n = string( "disk ref of " ) + (*io)->name();
          ref = Referential::referentialOfName( n );
          if( !ref )
          {
            ref = new Referential;
            ref->header().setProperty( "name", n );
          }
          Transformation *t = theAnatomist->getTransformation( ref, ownref );
          if( !t || !t->isGenerated() )
          {
            if( t )
              delete t;
            t = new Transformation( ref, ownref );
            t->motion() = m;
            t->registerTrans();
            (*io)->setChanged();
            (*io)->notifyObservers( theAnatomist );
          }
        }
        catch( ... )
        {
        }
    }
  }
}


void ObjectActions::graphLabelToName( const set<AObject*> & obj )
{
  set<AObject*>::const_iterator i, e = obj.end();
  for( i=obj.begin(); i!=e; ++i )
  {
    AGraph *g = dynamic_cast<AGraph *>( *i );
    if( g )
      g->copyAttributes( "label", "name", true );
  }
}


void ObjectActions::graphUseLabel( const set<AObject*> & obj )
{
  set<AObject*>::const_iterator i, e = obj.end();
  for( i=obj.begin(); i!=e; ++i )
  {
    AGraph *g = dynamic_cast<AGraph *>( *i );
    if( g )
      g->setLabelProperty( "label" );
  }
}


void ObjectActions::graphUseName( const set<AObject*> & obj )
{
  set<AObject*>::const_iterator i, e = obj.end();
  for( i=obj.begin(); i!=e; ++i )
  {
    AGraph *g = dynamic_cast<AGraph *>( *i );
    if( g )
      g->setLabelProperty( "name" );
  }
}


void ObjectActions::graphUseDefaultLabelProperty( const set<AObject*> & obj )
{
  set<AObject*>::const_iterator i, e = obj.end();
  for( i=obj.begin(); i!=e; ++i )
  {
    AGraph *g = dynamic_cast<AGraph *>( *i );
    if( g )
      g->setLabelProperty( "" );
  }
}


ObjectMenuCallback* ObjectActions::fileReloadMenuCallback()
{
  return new ObjectMenuCallbackFunc( &fileReload );
}


ObjectMenuCallback* ObjectActions::colorPaletteMenuCallback()
{
  return new ObjectMenuCallbackFunc( &colorPalette );
}


ObjectMenuCallback* ObjectActions::colorMaterialMenuCallback()
{
  return new ObjectMenuCallbackFunc( &colorMaterial );
}


ObjectMenuCallback* ObjectActions::referentialLoadMenuCallback()
{
  return new ObjectMenuCallbackFunc( &referentialLoad );
}


ObjectMenuCallback* ObjectActions::fusion2DControlMenuCallback()
{
  return new ObjectMenuCallbackFunc( &fusion2DControl );
}


ObjectMenuCallback* ObjectActions::fusion3DControlMenuCallback()
{
  return new ObjectMenuCallbackFunc( &fusion3DControl );
}


ObjectMenuCallback* ObjectActions::textureControlMenuCallback()
{
  return new ObjectMenuCallbackFunc( &textureControl );
}


ObjectMenuCallback* ObjectActions::saveStaticMenuCallback()
{
  return new ObjectMenuCallbackFunc( &saveStatic );
}


ObjectMenuCallback* ObjectActions::saveTextureMenuCallback()
{
  return new ObjectMenuCallbackFunc( &saveTexture );
}


ObjectMenuCallback* ObjectActions::extractTextureMenuCallback()
{
  return new ObjectMenuCallbackFunc( &extractTexture );
}


ObjectMenuCallback* ObjectActions::renameObjectMenuCallback()
{
  return new ObjectMenuCallbackFunc( &renameObject );
}


ObjectMenuCallback* ObjectActions::generateTexture1DMenuCallback()
{
  return new ObjectMenuCallbackFunc( &generateTexture1D );
}


ObjectMenuCallback* ObjectActions::generateTexture2DMenuCallback()
{
  return new ObjectMenuCallbackFunc( &generateTexture2D );
}


ObjectMenuCallback* ObjectActions::displayGraphChildrenMenuCallback()
{
  return new ObjectMenuCallbackFunc( &displayGraphChildren );
}


ObjectMenuCallback* ObjectActions::setAutomaticReferentialMenuCallback()
{
  return new ObjectMenuCallbackFunc( &setAutomaticReferential );
}


ObjectMenuCallback* ObjectActions::graphLabelToNameMenuCallback()
{
  return new ObjectMenuCallbackFunc( &graphLabelToName );
}


ObjectMenuCallback* ObjectActions::graphUseLabelMenuCallback()
{
  return new ObjectMenuCallbackFunc( &graphUseLabel );
}


ObjectMenuCallback* ObjectActions::graphUseNameMenuCallback()
{
  return new ObjectMenuCallbackFunc( &graphUseName );
}


ObjectMenuCallback* ObjectActions::graphUseDefaultLabelPropertyMenuCallback()
{
  return new ObjectMenuCallbackFunc( &graphUseDefaultLabelProperty );
}


#include <cartobase/object/object_d.h>
INSTANTIATE_GENERIC_OBJECT_TYPE( anatomist::ObjectActions::ActionObjectCallback )
INSTANTIATE_GENERIC_OBJECT_TYPE( anatomist::ObjectActions::ActionUntypedCallback )


