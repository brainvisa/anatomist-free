/*
 *  Copyright (C) 2002-2006 CEA
 *
 *  This software and supporting documentation were developed by
 *  	CEA/DSV/SHFJ
 *  	4 place du General Leclerc
 *  	91401 Orsay cedex
 *  	France
 *
 */


#include <anatomist/control/qObjTree.h>
#include <anafold/fgraph/afgraph.h>
#include <anafold/fgraph/fgMethod.h>
#include <anafold/fgraph/afgHelpers.h>
#include <anafold/fgraph/qwFFusionCtrl.h>
#include <anafold/fgraph/qwAnnealParams.h>
#include <si/fold/frgReader.h>
#include <si/fold/foldReader.h>
#include <si/fold/domainBox.h>
#include <si/fold/domainRbf.h>
#include <si/fold/labelsTranslator.h>
#include <si/graph/vertexclique.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/application/settings.h>
#include <anatomist/object/oReader.h>
#include <anatomist/fusion/fusionFactory.h>
#include <anatomist/color/objectPalette.h>
#include <anatomist/object/actions.h>
#include <anatomist/graph/GraphObject.h>
#include <anatomist/surface/triangulated.h>
#include <anatomist/misc/error.h>
#include <anatomist/control/graphParams.h>
#include <graph/tree/tree.h>
#include <aims/rgb/rgb.h>
#include <neur/gauss/gaussnet.h>
#include <aims/mesh/meshMerge.h>
#include <si/graph/anneal.h>
// rien que pour extraire le vecteur d'entrée...
#include <si/model/adaptiveLeaf.h>
#include <si/model/adaptiveTree.h>
#include <si/model/topAdaptive.h>
#include <si/finder/modelFinder.h>
#include <qpixmap.h>
#include <stdio.h>
#include <float.h>

using namespace anatomist;
using namespace sigraph;
using namespace aims;
using namespace carto;
using namespace std;

int 		AFGraph::_classType = AFGraph::registerClass();
Tree		*AFGraph::_optionTree = 0;


struct AFGraph::Private
{
  Private() : setcolorRecurs( false ), hasedges( false ), pot0alpha( 1. ),
    pot0alphaused( false ), nopotalpha( 0.25 ), nopotalphaused( true )
  {}

  bool	setcolorRecurs;
  bool  hasedges;
  float pot0alpha;
  bool  pot0alphaused;
  float nopotalpha;
  bool  nopotalphaused;
};


int AFGraph::registerClass()
{
  int type = registerObjectType( "AFGRAPH" );
  FusionMethod	*m = new FGraphMethod;
  if( !FusionFactory::registerMethod( m ) )
    delete m;
  _classType = type;
  //	register syntax and helpers
  AFGHelpers::init();

  return type;
}


//	Now the real AFGraph class


AFGraph::AFGraph( AGraph* model, AGraph* folds ) 
  : ObjectList(), _model( model ), _folds( folds ), _normInpVec( true ), 
    _mapMode( NODEPOT ), _weighted( false ),
    _pot0Col( true ), _pot0Center( false ), _relPot( false ), _p0red( 0.5 ), 
    _p0green( 0.7 ), _p0blue( 0.5 ), _nopotRed( 0.2 ), _nopotGreen( 0.5 ), 
    _nopotBlue( 0.2 ), _translator( 0 ), d( new AFGraph::Private )
{
  _type = _classType;

  if( !dynamic_cast<MGraph *>( model->graph() ) )
    {
      if( dynamic_cast<MGraph *>( folds->graph() ) )
	{
	  _model = folds;
	  _folds = model;
	}
      else
	AError( "Model graph is not a real MGraph" );
    }

  insert( _model );
  insert( _folds );
  setReferential( _folds->getReferential() );

  if( !dynamic_cast<CGraph *>( _folds->graph() ) )
    AError( "Fold graph is not a real CGraph" );

  glAddTextures( 1 );
  GLComponent::TexExtrema & ex = glTexExtrema();
  ex.min.push_back( 0.F );
  ex.max.push_back( 1.F );
  ex.minquant.push_back( 0.F );
  ex.maxquant.push_back( 1.F );

  _material.SetDiffuse( 0., 0.5, 1., 1. );

  if( QObjectTree::TypeNames.find( _type ) == QObjectTree::TypeNames.end() )
    {
      string	str = Settings::globalPath() + "/icons/list_afgraph.xpm";
      if( !QObjectTree::TypeIcons[ _type ].load( str.c_str() ) )
	{
	  QObjectTree::TypeIcons.erase( _type );
	  cerr << "Icon " << str << " not found\n";
	}

      QObjectTree::TypeNames[ _type ] = "GraphInteraction";
    }
  initCliques();
}


AFGraph::~AFGraph()
{
  delete _translator;

  iterator	i = find( _model );
  erase( i );
  i = find( _folds );
  erase( i );
  
  AObject	*obj;

  while( size() > 0 )
    {
      obj = *begin();
      i = begin();
      erase( i );
      delete obj;
    }

  ((CGraph *) _folds->graph())->deleteCliques();

  if( !_model->Visible() ) 
    theAnatomist->mapObject( _model );
 
  if( !_folds->Visible() ) 
    theAnatomist->mapObject( _folds );

  delete d;
}


float AFGraph::pot0Alpha() const
{
  return d->pot0alpha;
}


bool AFGraph::pot0AlphaUsed() const
{
  return d->pot0alphaused;
}


void AFGraph::setPot0Color( float r, float g, float b, float a, bool usea )
{
  _p0red = r;
  _p0green = g;
  _p0blue = b;
  d->pot0alpha = a;
  d->pot0alphaused = usea;
}


float AFGraph::noPotAlpha() const
{
  return d->nopotalpha;
}


bool AFGraph::noPotAlphaUsed() const
{
  return d->nopotalphaused;
}


void AFGraph::setNoPotColor( float r, float g, float b, float a, bool usea )
{
  _nopotRed = r;
  _nopotGreen = g;
  _nopotBlue = b;
  d->nopotalpha = a;
  d->nopotalphaused = usea;
}


float AFGraph::minTexValue() const
{
  return glTexExtrema().minquant[0];
}


float AFGraph::maxTexValue() const
{
  return glTexExtrema().maxquant[0];
}


void AFGraph::initCliques()
{
  //  Anatomist::Cursor	curs = theAnatomist->cursor();
  theAnatomist->setCursor( Anatomist::Working );

  deleteCliques();

  if( !_translator )
  _translator = new FoldLabelsTranslator( *(MGraph *) _model->graph() );

  CGraph	*cg = (CGraph *) _folds->graph();
  ((MGraph *) _model->graph())->modelFinder().initCliques( *cg, true, false, 
                                                           true );
  _folds->setChanged();

  MGraph	*mg = (MGraph *) _model->graph();

  cout << "Processing potentials..." << flush;
  Anneal	an( *cg, *mg );
  cg->setProperty( "potential", (float) an.processAllPotentials() );
  cout << "OK.\n";

  cout << "Processing connectivity..." << flush;
  const set<Clique *>	&cliques = cg->cliques();
  set<Clique *>::const_iterator	ic, fc=cliques.end();
  string	label, label2;

  ModelFinder		&mf = mg->modelFinder();
  Clique		*cl;
  VertexClique		*vcl;
  AttributedObject	*modV;
  Model			*mod;
  Domain		*dom;
  DomainBox		*bdom;
  DomainRBF		*domr;
  AdaptiveLeaf		*al;
  AdaptiveTree		*at;
  TopAdaptive		*ta;
  double		pot;
  double		wt;
  AGraphObject		*ago;
  string		goname, tname;
  ATriangulated		*tri;

  setColors();

  for( ic=cliques.begin(); ic!=fc; ++ic )
    {
      cl = *ic;
      vcl = dynamic_cast<VertexClique *>( cl );

      ago = new AGraphObject( cl );

      if( cl->getProperty( "label", label ) )
	{
	  if( vcl )
	    cl->setProperty( "connectivity_number", 
			      (int) vcl->connectivity( label ) );
	  goname = string( "Clique_" ) + label;
	  tname = string( "Domain_" ) + label;
	}
      else if( cl->getProperty( "label1", label ) 
	       && cl->getProperty( "label2", label2 ) )
	goname = string( "Clique_" ) + label + "-" + label2;
      else
	goname = string( "Clique_without_label" );
      ago->setName( theAnatomist->makeObjectName( goname ) );
      theAnatomist->registerObject( ago, false );
      cl->setProperty( "ana_object", shared_ptr<AObject>(
                       shared_ptr<AObject>::WeakShared, ago ) );
      insert( ago );

      if( cliqueWeight( cl, wt ) )
	{
	  cl->setProperty( "output_factor", wt );
	  if( wt != 0 && cl->getProperty( "potential", pot ) )
	    cl->setProperty( "potential_unweighted", pot/wt );
	}

      modV = mf.selectModel( cl );
      mod = 0;
      dom = 0;
      if( modV )
	{
	  modV->getProperty( "model", mod );
	  modV->getProperty( "domain", dom );
	}
      while( mod && mod->isAdaptive() )
	{
	  al = dynamic_cast<AdaptiveLeaf *>( mod );
	  if( al )
	    {
	      vector<double>	vec;
	      al->cliqueDescr().makeVector( cl, vec, modV );
	      al->cliqueDescr().preProcess( vec, modV );
	      cl->setProperty( "pot_vector", vec );
	      if( _normInpVec )	// normer les entrées ?
		{
		  SubAdaptive	& wk = al->workEl();
		  const map<unsigned, SubAdaptive::Stat> & st = wk.stats();
		  map<unsigned, SubAdaptive::Stat>::const_iterator 
		    ism, fsm=st.end();
		  for( ism=st.begin(); ism!=fsm; ++ism )
		    if( (*ism).second.sigma != 0 )
		      vec[(*ism).first] = ( vec[(*ism).first] - 
					    (*ism).second.mean ) 
			/ (*ism).second.sigma;
		  cl->setProperty( "pot_vector_norm", vec );
		}
	      mod = 0;
	    }
	  else
	    {
	      at = dynamic_cast<AdaptiveTree *>( mod );
	      if( at )
		mod = *at->begin();
	      else
		{
		  ta = dynamic_cast<TopAdaptive *>( mod );
		  if( ta )
		    mod = ta->model();
		  else
		    mod = 0;
		}
	    }
	}

      if( dom )
	{
	  bdom = dynamic_cast<DomainBox *>( dom );
	  if( bdom )
	    {
	      tri = new ATriangulated( tname.c_str() );
	      tri->setName( theAnatomist->makeObjectName( tname ) );
	      theAnatomist->registerObject( tri, false );
	      ago->insert( tri );
	      makeDomTriang( tri, bdom, (FGraph *) _folds->graph() );
	    }
	  else
	    {
	      domr = dynamic_cast<DomainRBF *>( dom );
	      if( domr )
		{
		  tri = new ATriangulated( tname.c_str() );
		  tri->setName( theAnatomist->makeObjectName( tname ) );
		  theAnatomist->registerObject( tri, false );
		  ago->insert( tri );
		  makeDomTriang( tri, domr, (FGraph *) _folds->graph() );
		}
	    }
	}
      ago->GetMaterial().SetDiffuse( 1., 1., 0., 0.7 );
    }
  cout << "OK.\n";

  // look if the data graph has viewable relations
  d->hasedges = false;
  const set<Edge *> & edg = cg->edges();
  set<Edge *>::const_iterator ie, ee = edg.end();
  for( ie=edg.begin(); ie!=ee; ++ie )
    if( (*ie)->hasProperty( "ana_object" ) )
    {
      d->hasedges = true;
      break;
    }

  setColors();
  theAnatomist->setCursor( Anatomist::Normal );
}


void AFGraph::setPalette( const AObjectPalette & pal )
{
  if( palette() )
    {
      ObjectList::setPalette( pal );
      setColors();
    }
  else
    ObjectList::setPalette( pal );
}


void AFGraph::SetMaterial( const Material & mat )
{
  AObject::SetMaterial( mat );

  setColors();
}


void AFGraph::setColors()
{
  if( d->setcolorRecurs )
    return;
  d->setcolorRecurs = true;

  switch( _mapMode )
    {
    case NODEPOT:
    case TOTALPOT:
      setColorsNodePot();
      setChanged();
      break;
    case LABELS:
      setColorsLabels();
      break;
    case CONSTANT:
    case TOTALCONST:
      setColorsNodePot();
      setChanged();
      break;
    default:
      cerr << "I can't map that mode so far. Try again with a later version\n";
      break;
    }
  d->setcolorRecurs = false;
}


void AFGraph::setColorsNodePot()
{
  //cout << "AFGraph::setColorsNodePot()\n";

  CGraph		*cg = (CGraph *) _folds->graph();
  Graph::iterator	iv, fv=cg->end();
  shared_ptr<AObject>	ao;
  double		pot;
  int			ind = 0;
  getOrCreatePalette();
  AObjectPalette	*objpal = palette();
  AimsData<AimsRGBA>	*cols = objpal->colors();
  float			cmin = objpal->min1(), cmax = objpal->max1();
  float			scale, mp, Mp, fmin, fmax;
  unsigned		ncol = cols->dimX();
  GLComponent::TexExtrema & ex = glTexExtrema();

  float minPot = FLT_MAX;
  float maxPot = -FLT_MAX;

  for( iv=cg->begin(); iv!=fv; ++iv )
    if( (*iv)->getProperty( "ana_object", ao ) && nodePotential( *iv, pot ) )
    {
      if( minPot > pot )
        minPot = pot;
      else if( maxPot < pot )
        maxPot = pot;
    }
  const set<Edge *> & edg = cg->edges();
  set<Edge *>::const_iterator ie, ee = edg.end();
  if( d->hasedges )
    for( ie=edg.begin(); ie!=ee; ++ie )
    {
      if( (*ie)->getProperty( "ana_object", ao ) && edgePotential( *ie, pot ) )
      {
        if( minPot > pot )
          minPot = pot;
        else if( maxPot < pot )
          maxPot = pot;
      }
    }

  if( isPot0Centered() )
  {
    if( maxPot < 0 || minPot < -maxPot )
      maxPot = -minPot;
    else
      minPot = -maxPot;
  }

  // set GLComponent extrema
  ex.minquant[0] = minPot;
  ex.maxquant[0] = maxPot;

  mp = minPot;
  Mp = maxPot;

  if( mp >= Mp )
    {
      mp = 0;
      Mp = 1;
    }

  //cout << "mp: " << mp << ", Mp: " << Mp << endl;

  if( cmin == cmax )
    {
      cmin = 0;
      cmax = 1;
    }

  scale = ((float) ncol) / ( ( cmax - cmin ) * ( Mp - mp ) );
  fmin = mp + cmin * ( Mp - mp );
  fmax = mp + cmax * ( Mp - mp );

  for( iv=cg->begin(); iv!=fv; ++iv )
    if( (*iv)->getProperty( "ana_object", ao ) )
      {
	if( nodePotential( *iv, pot ) )
	  {
	    // change material according to palette
	    if( pot <= fmin )
	      ind = 0;
	    else if( pot >= fmax )
	      ind = ncol - 1;
	    else
	      ind = (int) ( ( pot - fmin ) * scale );
	    //cout << "pot : " << pot << ", ind : " << ind << endl;
	    Material & mat = ao->GetMaterial();
	    if( _pot0Col && pot == 0 )
              mat.SetDiffuse( _p0red, _p0green, _p0blue,
                              pot0AlphaUsed() ? pot0Alpha() : mat.Diffuse( 3 ) );
	    else
	      {
		const AimsRGBA	& rgb = (*cols)( ind );
		mat.SetDiffuse( (float) rgb.red()   / 256, 
				(float) rgb.green() / 256, 
				(float) rgb.blue()  / 256, 
				mat.Diffuse( 3 ) );
	      }
	    ao->SetMaterial( mat );
	  }
	else
	  {
	    // no clique or no label: show in black (error code for dead folds)
	    Material & mat = ao->GetMaterial();
	    mat.SetDiffuse( _nopotRed, _nopotGreen, _nopotBlue, 
                            noPotAlphaUsed() ? noPotAlpha() : mat.Diffuse( 3 ) );
	    ao->SetMaterial( mat );
	  }
      }

  if( d->hasedges )
    for( ie=edg.begin(); ie!=ee; ++ie )
      if( (*ie)->getProperty( "ana_object", ao ) )
      {
        if( edgePotential( *ie, pot ) )
        {
            // change material according to palette
          if( pot <= fmin )
            ind = 0;
          else if( pot >= fmax )
            ind = ncol - 1;
          else
            ind = (int) ( ( pot - fmin ) * scale );
            //cout << "pot : " << pot << ", ind : " << ind << endl;
          Material & mat = ao->GetMaterial();
          if( _pot0Col && pot == 0 )
            mat.SetDiffuse( _p0red, _p0green, _p0blue,
                            pot0AlphaUsed() ? pot0Alpha() : mat.Diffuse( 3 ) );
          else
          {
            const AimsRGBA  & rgb = (*cols)( ind );
            mat.SetDiffuse( (float) rgb.red()   / 256,
                             (float) rgb.green() / 256,
                             (float) rgb.blue()  / 256,
                             mat.Diffuse( 3 ) );
          }
          ao->SetMaterial( mat );
        }
        else
        {
            // no clique or no label: show in black (error code for dead folds)
          Material & mat = ao->GetMaterial();
          mat.SetDiffuse( _nopotRed, _nopotGreen, _nopotBlue,
                          noPotAlphaUsed() ? noPotAlpha() : mat.Diffuse( 3 ) );
          ao->SetMaterial( mat );
        }
      }
}


void AFGraph::setColorsLabels()
{
  string	mapattrib = QGraphParam::colorsAttribute();
  bool		(* oldfunc )( AGraph* ag, AGraphObject* go, Material & mat ) 
    = AGraph::specialColorFunc;

  QGraphParam::setColorsAttribute( "label" );
  AGraph::specialColorFunc = &GraphParams::recolorLabelledGraph;

  _folds->SetMaterial( _folds->GetMaterial() );

  QGraphParam::setColorsAttribute( mapattrib );
  AGraph::specialColorFunc = oldfunc;
  setChanged();
}


Tree* AFGraph::optionTree() const
{
  if( !_optionTree )
    {
      Tree	*t, *t2;
      _optionTree = new Tree( true, "option tree" );
      t = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu", "Color" ) );
      _optionTree->insert( t );
      t2 = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu", "Palette" ) );
      t2->setProperty( "callback", &ObjectActions::colorPalette );
      t->insert( t2 );
      t2 = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu", "Material" ) );
      t2->setProperty( "callback", &ObjectActions::colorMaterial );
      t->insert( t2 );
      t = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu", "Fusion" ) );
      _optionTree->insert( t );
      t2 = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu", 
					      "Control graphs fusion" ) );
      t2->setProperty( "callback", &fusionControl );
      t->insert( t2 );
      t2 = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu", "Annealing" ) );
      t2->setProperty( "callback", &annealWin );
      t->insert( t2 );
    }
  return( _optionTree );
}


void AFGraph::fusionControl( const set<AObject *> & obj )
{
  //cout << "AFGraph::fusionControl\n";

  if( obj.size() != 1 )
    {
      cerr << "Select only one object to control the graph fusion\n";
      return;
    }
  QFFoldCtrl	*wfc = 
    new QFFoldCtrl( 0, QFFoldCtrl::tr( "Fold Fusion control" ), 
		    (AFGraph *) *obj.begin() );
  wfc->show();
}


bool AFGraph::cliqueWeight( const Clique* cl, double & wt )
{
  AttributedObject	*mao = ((MGraph *) _model->graph())
    ->modelFinder().selectModel( cl );
  Model			*mod;
  TopAdaptive		*ta;

  if( mao && mao->getProperty( "model", mod ) && mod )
    {
      ta = dynamic_cast<TopAdaptive *>( mod );
      if( ta )
	{
          wt = ta->relianceWeight() * ta->weight();
	  return true;
	}
    }
  return false;
}


bool AFGraph::nodePotential( const Vertex* v, double & pot )
{
  switch( _mapMode )
    {
    case NODEPOT:
      return( nodePotentialSimple( v, pot ) );
    case TOTALPOT:
      return( nodePotentialTotal( v, pot ) );
    case LABELS:
      return( nodePotentialLabels( v, pot ) );
    case CONSTANT:
      return( nodePotentialConstant( v, pot ) );
    case TOTALCONST:
      return( nodePotentialTotalConst( v, pot ) );
    default:
      return( false );
    }
}


bool AFGraph::nodePotentialSimple( const Vertex* v, double & pot )
{
  string	label, lc;
  set<Clique *>	*sc;
  set<Clique *>::const_iterator	ic, fc;
  double			wt;

  if( v->getProperty( "label", label ) && v->getProperty( "cliques", sc ) )
    {	// find clique with same label
      for( ic=sc->begin(), fc=sc->end(); ic!=fc; ++ic )
	if( (*ic)->getProperty( "label", lc ) && label == lc )
	  break;
      if( ic == fc || !(*ic)->getProperty( "potential", pot ) )
	return( false );
      else if( !_weighted && cliqueWeight( *ic, wt ) && wt != 0 )
	{
	  pot /= wt;
	}
      return( true );
    }
  return( false );
}


bool AFGraph::nodePotentialConstant( const Vertex* v, double & pot )
{
  string	label, lc;
  set<Clique *>	*sc;
  set<Clique *>::const_iterator	ic, fc;
  double			wt;

  if( v->getProperty( "label", label ) && v->getProperty( "cliques", sc ) )
    {	// find clique with same label
      for( ic=sc->begin(), fc=sc->end(); ic!=fc; ++ic )
	if( (*ic)->getProperty( "label", lc ) && label == lc )
	  break;
      if( ic == fc || !(*ic)->getProperty( "potential", pot ) )
	return( false );
      else if( _weighted && cliqueWeight( *ic, wt ) )
	{
	  pot = wt;
	}
      else
	pot = 1;
      return( true );
    }
  return( false );
}


bool AFGraph::nodePotentialTotal( const Vertex* v, double & pot )
{
  string	label, lc;
  set<Clique *>	*sc;
  set<Clique *>::const_iterator	ic, fc;
  double		tpot;
  double		wt, tw = 0;

  pot = 0;

  if( v->getProperty( "label", label ) && v->getProperty( "cliques", sc ) )
    {	// find clique containing the label
      for( ic=sc->begin(), fc=sc->end(); ic!=fc; ++ic )
	if( (*ic)->getProperty( "label", lc ) && label == lc )
	  {
	    if( (*ic)->getProperty( "potential", tpot ) )
	      {
		if( !_weighted && cliqueWeight( *ic, wt ) && wt != 0 )
		  {
		    tpot /= wt;
		    tw += wt;
		  }
		else
		  tw += 1;
		pot += tpot;
	      }
	  }
	else if( ( (*ic)->getProperty( "label1", lc ) && label == lc )
		 || ( (*ic)->getProperty( "label2", lc ) && label == lc ) )
	  {
	    if( (*ic)->getProperty( "potential", tpot ) )
	      {
		if( !_weighted && cliqueWeight( *ic, wt ) && wt != 0 )
		  {
		    tpot /= wt;
		    tw += wt;
		  }
		else
		  tw += 1;
		pot += tpot;
	      }
	  }
    }

  if( tw == 0 )
    return( false );

  pot /= tw;
  return( true );
}


bool AFGraph::nodePotentialTotalConst( const Vertex* v, double & pot )
{
  string	label, lc;
  set<Clique *>	*sc;
  set<Clique *>::const_iterator	ic, fc;
  double		tpot;
  double		wt, tw = 0;

  pot = 0;

  if( v->getProperty( "label", label ) && v->getProperty( "cliques", sc ) )
    {	// find clique containing the label
      for( ic=sc->begin(), fc=sc->end(); ic!=fc; ++ic )
	if( ( (*ic)->getProperty( "label", lc ) && label == lc )
	    || ( (*ic)->getProperty( "label1", lc ) && label == lc )
	    || ( (*ic)->getProperty( "label2", lc ) && label == lc ) )
	  {
	    if( (*ic)->getProperty( "potential", tpot ) )
	      {
		if( _weighted && cliqueWeight( *ic, wt ) )
		  pot += wt;
		else
		  pot += 1;
		tw += 1;
	      }
	  }
    }

  if( tw == 0 )
    return( false );

  pot /= tw;
  return( true );
}


bool AFGraph::nodePotentialLabels( const Vertex* v, double & pot )
{
  string	label, name;

  if( v->getProperty( "label", label ) && v->getProperty( "name", name ) )
    {
      if( label == _translator->lookupLabel( name ) )
        pot = -1;
      else
        pot = 1;
    }
  else
    return( false );
  return( true );
}


bool AFGraph::edgePotential( const Edge* v, double & pot )
{
  switch( _mapMode )
  {
    case NODEPOT:
    case TOTALPOT:
      return edgePotentialSimple( v, pot );
    case LABELS:
      return false;
    case CONSTANT:
    case TOTALCONST:
      return edgePotentialConstant( v, pot );
    default:
      return false;
  }
}


bool AFGraph::edgePotentialSimple( const Edge* e, double & pot )
{
  string        label1, label2, lc;
  set<Clique *> *sc;
  set<Clique *>::const_iterator ic, fc;
  double                        wt;
  Vertex        *v1, *v2;
  Edge::const_iterator  iv = e->begin();

  v1 = *iv;
  ++iv;
  v2 = *iv;
  if( v1->getProperty( "label", label1 ) && v2->getProperty( "label", label2 )
      && v1->getProperty( "cliques", sc ) )
  {   // find clique with same label
    if( label1 == label2 ) // same label: not a model relation
      return false;
    for( ic=sc->begin(), fc=sc->end(); ic!=fc; ++ic )
      if( ( (*ic)->getProperty( "label1", lc ) && label1 == lc
              && (*ic)->getProperty( "label2", lc ) && label2 == lc )
              || ( (*ic)->getProperty( "label1", lc ) && label2 == lc
              && (*ic)->getProperty( "label2", lc ) && label1 == lc ) )
        break;
    if( ic == fc || !(*ic)->getProperty( "potential", pot ) )
      return( false );
    else if( !_weighted && cliqueWeight( *ic, wt ) && wt != 0 )
    {
      pot /= wt;
    }
    return( true );
  }
  return( false );
}


bool AFGraph::edgePotentialConstant( const Edge* e, double & pot )
{
  string        label1, label2, lc;
  set<Clique *> *sc;
  set<Clique *>::const_iterator ic, fc;
  double                        wt;
  Vertex        *v1, *v2;
  Edge::const_iterator  iv = e->begin();

  v1 = *iv;
  ++iv;
  v2 = *iv;
  if( v1->getProperty( "label", label1 ) && v2->getProperty( "label", label2 )
      && v1->getProperty( "cliques", sc ) )
  {   // find clique with same label
    if( label1 == label2 ) // same label: not a model relation
      return false;
    for( ic=sc->begin(), fc=sc->end(); ic!=fc; ++ic )
      if( ( (*ic)->getProperty( "label1", lc ) && label1 == lc
            && (*ic)->getProperty( "label2", lc ) && label2 == lc )
              || ( (*ic)->getProperty( "label1", lc ) && label2 == lc
              && (*ic)->getProperty( "label2", lc ) && label1 == lc ) )
        break;
    if( ic == fc || !(*ic)->getProperty( "potential", pot ) )
      return( false );
    else if( _weighted && cliqueWeight( *ic, wt ) )
    {
      pot = wt;
    }
    else
      pot = 1;
    return( true );
  }
  return( false );
}


void AFGraph::updatePotentials()
{
  CGraph	*cg = (CGraph *) _folds->graph();
  MGraph	*mg = (MGraph *) _model->graph();

  cout << "Processing potentials..." << flush;
  Anneal	an( *cg, *mg );
  cg->setProperty( "potential", (float) an.processAllPotentials() );

  const set<Clique *>		& cliques = cg->cliques();
  set<Clique *>::const_iterator	ic, fc=cliques.end();
  Clique			*cl;
  double			pot, wt;

  for( ic=cliques.begin(); ic!=fc; ++ic )
    {
      cl = *ic;

      if( cliqueWeight( cl, wt ) )
	{
	  cl->setProperty( "output_factor", wt );
	  if( wt != 0 )
	    {
	      if( cl->getProperty( "potential", pot ) )
		cl->setProperty( "potential_unweighted", pot/wt );
	    }
	  else
	    cl->setProperty( "potential_unweighted", 0 );
	}
    }

  // look if the data graph has viewable relations
  d->hasedges = false;
  const set<Edge *> & edg = cg->edges();
  set<Edge *>::const_iterator ie, ee = edg.end();
  for( ie=edg.begin(); ie!=ee; ++ie )
    if( (*ie)->hasProperty( "ana_object" ) )
  {
    d->hasedges = true;
    break;
  }

  cout << "OK.\n";
}


void AFGraph::internalUpdate()
{
  if( _folds->hasChanged() || _model->hasChanged() )
    {
      updatePotentials();
      _folds->notifyObservers();
      _model->notifyObservers();
      _folds->clearHasChangedFlags();
      _model->clearHasChangedFlags();
    }
  setColors();

  MObject::internalUpdate();
}


void AFGraph::deleteCliques()
{
  iterator	i, j, f=end();
  AObject	*obj;

  for( i=begin(); i!=f; )
    {
      if( *i != _model && *i != _folds )
	{
	  j = i;
	  ++i;
	  obj = *j;
	  erase( j );
	  delete obj;
	}
      else
	++i;
    }
}


void AFGraph::makeDomTriang( ATriangulated* tri, DomainBox* dom, FGraph* fg )
{
  AimsSurfaceTriangle		*surf = new AimsSurfaceTriangle;
  vector<vector<double> >	pts;
  vector<Point3df>		& vx = surf->vertex();
  vector<Point3df>		& nrm = surf->normal();
  vector<AimsVector<uint,3> >	& tr = surf->polygon();
  vector<float>			trot, tscal, ttransl; //, vsz;
  Point3df			vert;
  Point3df			norm;
  AimsVector<uint,3>		triang;
  double			v1x, v1y, v1z, v2x, v2y, v2z, nx, ny, nz, d;
  unsigned			i;

  fg->getProperty( "Talairach_rotation", trot );
  fg->getProperty( "Talairach_scale", tscal );
  fg->getProperty( "Talairach_translation", ttransl );
  //fg->getProperty( "voxel_size", vsz );

  dom->cubeTalairach( pts );

  //	Talairach inverse
  for( i=0; i<8; ++i )
    {
      pts[i][0] /= tscal[0];
      pts[i][1] /= tscal[1];
      pts[i][2] /= tscal[2];
      v1x = trot[0] * pts[i][0] + trot[3] * pts[i][1] 
	+ trot[6] * pts[i][2];
      v1y = trot[1] * pts[i][0] + trot[4] * pts[i][1] 
	+ trot[7] * pts[i][2];
      v1z = trot[2] * pts[i][0] + trot[5] * pts[i][1] 
	+ trot[8] * pts[i][2];
      pts[i][0] = v1x - ttransl[0];
      pts[i][1] = v1y - ttransl[1];
      pts[i][2] = v1z - ttransl[2];
    }

  //	vertices, face par face

  vert[0] = pts[0][0];
  vert[1] = pts[0][1];
  vert[2] = pts[0][2];
  vx.push_back( vert );

  vert[0] = pts[1][0];
  vert[1] = pts[1][1];
  vert[2] = pts[1][2];
  vx.push_back( vert );

  vert[0] = pts[2][0];
  vert[1] = pts[2][1];
  vert[2] = pts[2][2];
  vx.push_back( vert );

  vert[0] = pts[3][0];
  vert[1] = pts[3][1];
  vert[2] = pts[3][2];
  vx.push_back( vert );

  v1x = pts[1][0] - pts[0][0];
  v1y = pts[1][1] - pts[0][1];
  v1z = pts[1][2] - pts[0][2];
  v2x = pts[3][0] - pts[0][0];
  v2y = pts[3][1] - pts[0][1];
  v2z = pts[3][2] - pts[0][2];
  nx = v1y * v2z - v1z * v2y;
  ny = v1z * v2x - v1x * v2z;
  nz = v1x * v2y - v1y * v2x;
  d = sqrt( nx * nx + ny * ny + nz * nz );
  nx /= d;
  ny /= d;
  nz /= d;

  norm[0] = nx;
  norm[1] = ny;
  norm[2] = nz;
  nrm.push_back( norm );
  nrm.push_back( norm );
  nrm.push_back( norm );
  nrm.push_back( norm );

  //	face 2

  vert[0] = pts[3][0];
  vert[1] = pts[3][1];
  vert[2] = pts[3][2];
  vx.push_back( vert );

  vert[0] = pts[2][0];
  vert[1] = pts[2][1];
  vert[2] = pts[2][2];
  vx.push_back( vert );

  vert[0] = pts[7][0];
  vert[1] = pts[7][1];
  vert[2] = pts[7][2];
  vx.push_back( vert );

  vert[0] = pts[4][0];
  vert[1] = pts[4][1];
  vert[2] = pts[4][2];
  vx.push_back( vert );

  v1x = pts[2][0] - pts[3][0];
  v1y = pts[2][1] - pts[3][1];
  v1z = pts[2][2] - pts[3][2];
  v2x = pts[4][0] - pts[3][0];
  v2y = pts[4][1] - pts[3][1];
  v2z = pts[4][2] - pts[3][2];
  nx = v1y * v2z - v1z * v2y;
  ny = v1z * v2x - v1x * v2z;
  nz = v1x * v2y - v1y * v2x;
  d = sqrt( nx * nx + ny * ny + nz * nz );
  nx /= d;
  ny /= d;
  nz /= d;

  norm[0] = nx;
  norm[1] = ny;
  norm[2] = nz;
  nrm.push_back( norm );
  nrm.push_back( norm );
  nrm.push_back( norm );
  nrm.push_back( norm );

  //	face 3

  vert[0] = pts[1][0];
  vert[1] = pts[1][1];
  vert[2] = pts[1][2];
  vx.push_back( vert );

  vert[0] = pts[6][0];
  vert[1] = pts[6][1];
  vert[2] = pts[6][2];
  vx.push_back( vert );

  vert[0] = pts[7][0];
  vert[1] = pts[7][1];
  vert[2] = pts[7][2];
  vx.push_back( vert );

  vert[0] = pts[2][0];
  vert[1] = pts[2][1];
  vert[2] = pts[2][2];
  vx.push_back( vert );

  v1x = pts[6][0] - pts[1][0];
  v1y = pts[6][1] - pts[1][1];
  v1z = pts[6][2] - pts[1][2];
  v2x = pts[2][0] - pts[1][0];
  v2y = pts[2][1] - pts[1][1];
  v2z = pts[2][2] - pts[1][2];
  nx = v1y * v2z - v1z * v2y;
  ny = v1z * v2x - v1x * v2z;
  nz = v1x * v2y - v1y * v2x;
  d = sqrt( nx * nx + ny * ny + nz * nz );
  nx /= d;
  ny /= d;
  nz /= d;

  norm[0] = nx;
  norm[1] = ny;
  norm[2] = nz;
  nrm.push_back( norm );
  nrm.push_back( norm );
  nrm.push_back( norm );
  nrm.push_back( norm );

  //	face 4

  vert[0] = pts[0][0];
  vert[1] = pts[0][1];
  vert[2] = pts[0][2];
  vx.push_back( vert );

  vert[0] = pts[3][0];
  vert[1] = pts[3][1];
  vert[2] = pts[3][2];
  vx.push_back( vert );

  vert[0] = pts[4][0];
  vert[1] = pts[4][1];
  vert[2] = pts[4][2];
  vx.push_back( vert );

  vert[0] = pts[5][0];
  vert[1] = pts[5][1];
  vert[2] = pts[5][2];
  vx.push_back( vert );

  v1x = pts[3][0] - pts[0][0];
  v1y = pts[3][1] - pts[0][1];
  v1z = pts[3][2] - pts[0][2];
  v2x = pts[5][0] - pts[0][0];
  v2y = pts[5][1] - pts[0][1];
  v2z = pts[5][2] - pts[0][2];
  nx = v1y * v2z - v1z * v2y;
  ny = v1z * v2x - v1x * v2z;
  nz = v1x * v2y - v1y * v2x;
  d = sqrt( nx * nx + ny * ny + nz * nz );
  nx /= d;
  ny /= d;
  nz /= d;

  norm[0] = nx;
  norm[1] = ny;
  norm[2] = nz;
  nrm.push_back( norm );
  nrm.push_back( norm );
  nrm.push_back( norm );
  nrm.push_back( norm );

  //	face 5

  vert[0] = pts[0][0];
  vert[1] = pts[0][1];
  vert[2] = pts[0][2];
  vx.push_back( vert );

  vert[0] = pts[5][0];
  vert[1] = pts[5][1];
  vert[2] = pts[5][2];
  vx.push_back( vert );

  vert[0] = pts[6][0];
  vert[1] = pts[6][1];
  vert[2] = pts[6][2];
  vx.push_back( vert );

  vert[0] = pts[1][0];
  vert[1] = pts[1][1];
  vert[2] = pts[1][2];
  vx.push_back( vert );

  v1x = pts[5][0] - pts[0][0];
  v1y = pts[5][1] - pts[0][1];
  v1z = pts[5][2] - pts[0][2];
  v2x = pts[1][0] - pts[0][0];
  v2y = pts[1][1] - pts[0][1];
  v2z = pts[1][2] - pts[0][2];
  nx = v1y * v2z - v1z * v2y;
  ny = v1z * v2x - v1x * v2z;
  nz = v1x * v2y - v1y * v2x;
  d = sqrt( nx * nx + ny * ny + nz * nz );
  nx /= d;
  ny /= d;
  nz /= d;

  norm[0] = nx;
  norm[1] = ny;
  norm[2] = nz;
  nrm.push_back( norm );
  nrm.push_back( norm );
  nrm.push_back( norm );
  nrm.push_back( norm );

  //	face 6

  vert[0] = pts[7][0];
  vert[1] = pts[7][1];
  vert[2] = pts[7][2];
  vx.push_back( vert );

  vert[0] = pts[6][0];
  vert[1] = pts[6][1];
  vert[2] = pts[6][2];
  vx.push_back( vert );

  vert[0] = pts[5][0];
  vert[1] = pts[5][1];
  vert[2] = pts[5][2];
  vx.push_back( vert );

  vert[0] = pts[4][0];
  vert[1] = pts[4][1];
  vert[2] = pts[4][2];
  vx.push_back( vert );

  v1x = pts[4][0] - pts[5][0];
  v1y = pts[4][1] - pts[5][1];
  v1z = pts[4][2] - pts[5][2];
  v2x = pts[6][0] - pts[5][0];
  v2y = pts[6][1] - pts[5][1];
  v2z = pts[6][2] - pts[5][2];
  nx = v1y * v2z - v1z * v2y;
  ny = v1z * v2x - v1x * v2z;
  nz = v1x * v2y - v1y * v2x;
  d = sqrt( nx * nx + ny * ny + nz * nz );
  nx /= d;
  ny /= d;
  nz /= d;

  norm[0] = nx;
  norm[1] = ny;
  norm[2] = nz;
  nrm.push_back( norm );
  nrm.push_back( norm );
  nrm.push_back( norm );
  nrm.push_back( norm );

  //tri->SetVertex( vx, 0 );
  //tri->SetNormals( nrm, 0 );

  //	triangles 

  triang[0] = 2;
  triang[1] = 1;
  triang[2] = 0;
  tr.push_back( triang );

  triang[0] = 3;
  triang[1] = 2;
  triang[2] = 0;
  tr.push_back( triang );

  triang[0] = 6;
  triang[1] = 5;
  triang[2] = 4;
  tr.push_back( triang );

  triang[0] = 7;
  triang[1] = 6;
  triang[2] = 4;
  tr.push_back( triang );

  triang[0] = 10;
  triang[1] = 9;
  triang[2] = 8;
  tr.push_back( triang );

  triang[0] = 11;
  triang[1] = 10;
  triang[2] = 8;
  tr.push_back( triang );

  triang[0] = 14;
  triang[1] = 13;
  triang[2] = 12;
  tr.push_back( triang );

  triang[0] = 15;
  triang[1] = 14;
  triang[2] = 12;
  tr.push_back( triang );

  triang[0] = 18;
  triang[1] = 17;
  triang[2] = 16;
  tr.push_back( triang );

  triang[0] = 19;
  triang[1] = 18;
  triang[2] = 16;
  tr.push_back( triang );

  triang[0] = 22;
  triang[1] = 21;
  triang[2] = 20;
  tr.push_back( triang );

  triang[0] = 20;
  triang[1] = 22;
  triang[2] = 23;
  tr.push_back( triang );

  //tri->SetTriangles( tr, 0 );
  tri->setSurface( surf );
  //tri->UpdateMinAndMax();
}


void AFGraph::makeDomTriang( ATriangulated* tri, DomainRBF* dom, FGraph* fg )
{
  AimsSurfaceTriangle		*surf = new AimsSurfaceTriangle;
  vector<float>			trot, tscal, ttransl;
  double			v1x, v1y, v1z;
  float				sigma, thresh, radius;
  Point3df			center;
  const double			*c;
  unsigned			i, n = dom->nGauss();
  const GaussNet		& gnet = dom->gaussNet();
  const Gaussian		*gs;

  fg->getProperty( "Talairach_rotation", trot );
  fg->getProperty( "Talairach_scale", tscal );
  fg->getProperty( "Talairach_translation", ttransl );

  sigma = dom->sigma();
  thresh = dom->threshold();
  // radius corresponding to the threshold on a gaussian
  radius = sigma * sqrt( 2 * ( - log( thresh ) ) );

  for( i=0; i<n ; ++i )
    {
      gs = gnet.gauss( i );
      c = gs->center();

      // inverse Talairach
      center[0] = c[0] / tscal[0];
      center[1] = c[1] / tscal[1];
      center[2] = c[2] / tscal[2];
      v1x = trot[0] * center[0] + trot[3] * center[1] 
	+ trot[6] * center[2];
      v1y = trot[1] * center[0] + trot[4] * center[1] 
	+ trot[7] * center[2];
      v1z = trot[2] * center[0] + trot[5] * center[1] 
	+ trot[8] * center[2];
      center[0] = v1x - ttransl[0];
      center[1] = v1y - ttransl[1];
      center[2] = v1z - ttransl[2];

      // make a mesh of a sphere
      meshMerge( *surf, createIcosahedron( center, radius ) );
    }

  tri->setSurface( surf );
  tri->UpdateMinAndMax();
}


void AFGraph::annealWin( const set<AObject *> & obj )
{
  if( obj.size() != 1 )
    {
      cerr << "Select only one object for annealing window\n";
      return;
    }
  QAnnealParams	*anp = new QAnnealParams( 0, "Annealing ", 
					  (AFGraph *) *obj.begin() );
  anp->show();
}


void AFGraph::setModelWeights( double factor )
{
  FRGraph	*mg = (FRGraph *) _model->graph();

  //cout << "AFGraph::setModelWeights\n";
  if( factor > 0 )
    mg->setWeights( factor );
  else
    mg->removeWeights();
  _model->setChanged();
  _model->internalUpdate();
  _model->notifyObservers( this );
  setColors();
  notifyObservers( this );
}


void AFGraph::createDefaultPalette( const string & name  )
{
  if( name.empty() )
    ObjectList::createDefaultPalette( "Blue-Red" );
  else
    ObjectList::createDefaultPalette( name );
}


bool AFGraph::render( PrimList & prim, const ViewState & state )
{
  return _folds->render( prim, state );
}


GLComponent* AFGraph::glAPI()
{
  return this;
}


const GLComponent* AFGraph::glAPI() const
{
  return this;
}


const AObjectPalette* AFGraph::glPalette( unsigned ) const
{
  return palette();
}


