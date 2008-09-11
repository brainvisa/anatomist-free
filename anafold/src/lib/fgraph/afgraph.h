/*
 *  Copyright (C) 1999-2006 CEA
 *
 *  This software and supporting documentation were developed by
 *  	CEA/DSV/SHFJ
 *  	4 place du General Leclerc
 *  	91401 Orsay cedex
 *  	France
 *
 */

#ifndef ANAFOLD_FGRAPH_AFGRAPH_H
#define ANAFOLD_FGRAPH_AFGRAPH_H


#include <anatomist/graph/Graph.h>
#include <anatomist/mobject/globjectlist.h>
#include <iostream>


namespace sigraph
{
  class DomainBox;
  class DomainRBF;
  class FGraph;
  class FoldLabelsTranslator;
  class Clique;
}


namespace anatomist
{
  template<int D> class ASurface;
  typedef ASurface<3> ATriangulated;
  //class ATriangulated;


  class AFGraph : public ObjectList, public GLComponent
  {
  public:
    enum Mode
    {
      NODEPOT,
      TOTALPOT,
      LABELS, 
      CONSTANT, 
      TOTALCONST, 
    };

    AFGraph( AGraph* model, AGraph* folds );
    virtual ~AFGraph();

    virtual int MType() const { return( type() ); }

    virtual bool CanRemove( AObject* obj );

    virtual AObject* ObjectAt( float x, float y, float z, float t, 
			       float tol = 0 )
      { return( _folds->ObjectAt( x, y, z, t, tol ) ); }
    virtual void SetMaterial( const Material & mat );
    virtual void setPalette( const AObjectPalette & pal );
    virtual void internalUpdate();
    virtual bool render( PrimList &, const ViewState & );

    AGraph* model() const { return( _model ); }
    AGraph* folds() const { return( _folds ); }

    /**	maps the appropriate color on the child fold graph objects.
	According to the render mode, maps either potentials, labels, etc. */
    virtual void setColors();
    /**	maps the node cliques potentials (with or without the weight, 
	according to the map mode) */
    virtual void setColorsNodePot();
    /**	maps the labels colors from hierarchy (like in graph params 
	settings) */
    virtual void setColorsLabels();
    ///	mode for information mapping
    Mode mapMode() const { return( _mapMode ); }
    void setMapMode( Mode mode ) 
      { if( mode != _mapMode ) { _mapMode = mode; setChanged(); } }
    bool isMapWeighted() const { return( _weighted ); }
    void setMapWeighted( bool wt ) 
      { if( wt != _weighted ) { _weighted = wt; setChanged(); } }
    bool isRelPotentials() const { return( _relPot ); }
    void setRelPotentials( bool onoff )
      { if( _relPot != onoff ) { _relPot = onoff; setChanged(); } }
    bool pot0HasColor() const { return( _pot0Col ); }
    void setPot0HasCol( bool onoff )
      { if( _pot0Col != onoff ) { _pot0Col = onoff; setChanged(); } }
    bool isPot0Centered() const { return( _pot0Center ); }
    void setPot0Centered( bool onoff )
      { if( _pot0Center != onoff ) { _pot0Center = onoff; setChanged(); } }
    float pot0Red() const { return( _p0red ); }
    float pot0Green() const { return( _p0green ); }
    float pot0Blue() const { return( _p0blue ); }
    float pot0Alpha() const;
    bool pot0AlphaUsed() const;
    void setPot0Color( float r, float g, float b, float a = 1.,
                       bool usea = false );
    float noPotRed() const { return( _nopotRed ); }
    float noPotGreen() const { return( _nopotGreen ); }
    float noPotBlue() const { return( _nopotBlue ); }
    float noPotAlpha() const;
    bool noPotAlphaUsed() const;
    void setNoPotColor( float r, float g, float b, float a = 0.2,
                        bool usea = true );

    virtual Tree* optionTree() const;

    static int classType() { return( _classType ); }
    ///	Opens the fusion control window (menu callback)
    static void fusionControl( const std::set<AObject *> & );
    ///	Opens the annealing window (menu callback)
    static void annealWin( const std::set<AObject *> & );
    ///	Modifies the model graph
    void setModelWeights( double szfactor );
    void updatePotentials();
    virtual float minTexValue() const;
    virtual float maxTexValue() const;

    virtual GLComponent* glAPI();
    virtual const GLComponent* glAPI() const;
    virtual const AObjectPalette* glPalette( unsigned tex = 0 ) const;

    static Tree*	_optionTree;

  protected:
    void initCliques();
    void deleteCliques();
    bool cliqueWeight( const sigraph::Clique* cl, double & wt );
    bool nodePotential( const Vertex* v, double & pot );
    bool nodePotentialSimple( const Vertex* v, double & pot );
    bool nodePotentialTotal( const Vertex* v, double & pot );
    bool nodePotentialLabels( const Vertex* v, double & pot );
    bool nodePotentialConstant( const Vertex* v, double & pot );
    bool nodePotentialTotalConst( const Vertex* v, double & pot );
    bool edgePotential( const Edge* v, double & pot );
    bool edgePotentialSimple( const Edge* v, double & pot );
    bool edgePotentialConstant( const Edge* v, double & pot );
    void makeDomTriang( ATriangulated* tri, sigraph::DomainBox* dom,
			sigraph::FGraph* fg );
    void makeDomTriang( ATriangulated* tri, sigraph::DomainRBF* dom, 
			sigraph::FGraph* fg );

    virtual void createDefaultPalette( const std::string & name  );

    AGraph				*_model;
    AGraph				*_folds;
    bool				_normInpVec;
    Mode				_mapMode;
    bool				_weighted;
    bool				_pot0Col;
    bool				_pot0Center;
    bool				_relPot;
    float				_p0red;
    float				_p0green;
    float				_p0blue;
    float				_nopotRed;
    float				_nopotGreen;
    float				_nopotBlue;
    sigraph::FoldLabelsTranslator	*_translator;

  private:
    struct Private;

    ///	ensures the object class is registered in Anatomist
    static int registerClass();

    Private	*d;
    static int	_classType;
  };



//	Code


inline bool AFGraph::CanRemove( AObject* )
{
  std::cout << "AFGraph::CanRemove\n je reponds false !";
  return( false );
}


}

#endif


