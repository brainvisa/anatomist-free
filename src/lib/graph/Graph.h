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


#ifndef ANA_GRAPH_GRAPH_H
#define ANA_GRAPH_GRAPH_H


#include <anatomist/mobject/MObject.h>
#include <anatomist/graph/attribAObject.h>
#include <anatomist/graph/AGraphIterator.h>
#include <anatomist/surface/glcomponent.h>
#include <graph/graph/graph.h>
#include <cartodata/volume/volume.h>


class QGraphProperties;

namespace anatomist
{

  class AGraphObject;

  /**	AGraph object class
   */
  class AGraph : public MObject, public GLComponent, public AttributedAObject
  {
  public:
    enum ColorMode
      {
        Normal, PropertyMap,
      };
    AGraph( Graph *dataGraph, const std::string & filename,
            bool init,
            const Point3d& labelDimension );
    AGraph( Graph *dataGraph, const std::string & filename,
            bool init = true,
            const Point3dl& labelDimension = Point3dl(64, 64, 64) );
    AGraph( carto::rc_ptr<Graph> dataGraph, const std::string & filename,
            bool init,
            const Point3d& labelDimension );
    AGraph( carto::rc_ptr<Graph> dataGraph, const std::string & filename,
            bool init = true,
            const Point3dl& labelDimension = Point3dl(64, 64, 64) );
    
    virtual ~AGraph();

    virtual AObject* clone( bool shallow = true );

    virtual int MType() const { return( AObject::GRAPH ); }
    virtual size_t size() const;
    virtual iterator begin();
    virtual const_iterator begin() const;
    virtual iterator end();
    virtual const_iterator end() const;
    virtual void insert( AObject * );
    virtual void insert( AObject* obj, std::string syntacticAtt );
    virtual void insert( const carto::shared_ptr<AObject> & obj,
                         std::string syntacticAtt = "fold" );
    void insert( AObject* obj, carto::GenericObject* vertex );
    virtual const_iterator find( const AObject * ) const;
    virtual void erase( iterator & );
    ///	low-level data acces
    Graph* graph() const;
    void setGraph( carto::rc_ptr<Graph> );

    virtual std::list<AObject *> renderedSubObjects( const ViewState & ) const;
    virtual bool render( PrimList &, const ViewState & );

    virtual void setGeomExtrema();
    virtual bool boundingBox( std::vector<float> & bmin,
                              std::vector<float> & bmax ) const;
    virtual bool boundingBox2D( std::vector<float> & bmin,
                                std::vector<float> & bmax ) const
    { return boundingBox( bmin, bmax ); }
    /// Can be display in 2D windows.
    bool Is2DObject() { return(true); }
    /// Can be display in 3D windows.
    bool Is3DObject() { return(true); }
    virtual AObject* objectAt( const std::vector<float> & pos, float tol = 0 );
    virtual void SetMaterial( const Material & mat );
    virtual bool shouldRemoveChildrenWithMe() const;

    virtual carto::GenericObject* attributed();
    virtual const carto::GenericObject* attributed() const;

    static AObject* LoadGraph( const std::string & filename,
                               carto::Object options=carto::none() );
    virtual bool save( const std::string & filename );
    virtual bool savable() const { return true; }
    void loadSubObjects( int mask );
    ColorMode colorMode() const;
    void setColorMode( ColorMode, bool update = true );
    int colorPropertyMask() const;
    /// bitwise combination of 1: nodes, 2: relations
    void setColorPropertyMask( int, bool update = true );
    /// property mapped with colormap
    std::string colorProperty() const;
    void setColorProperty( const std::string &, bool update = true );
    void updateColors(void);

    /**	Used to map special colors on graph objects while changing the
  	graph material. If this pointer is left null, no function is called
	@return	true if mat has been changed
    */
    static bool (*specialColorFunc)( AGraph* ag, AGraphObject* go,
				     Material & mat );

    const carto::rc_ptr<std::map<std::string, std::vector<int> > >
    objAttColors() const;
    carto::rc_ptr<std::map<std::string, std::vector<int> > >
    objAttColors();
    void copyAttributes( const std::string & oldatt,
                         const std::string & newatt, bool removeOld = false,
                         bool dorels = false );
    /** property used to store the current label.
        \param allowDefault take the default global setting (see
        GraphParams::attribute) into account rather than return an empty string
        \return "name" or "label", or an empty string if none is specified in
        the graph ("label_property" property) and allowDefault is true
    */
    std::string labelProperty( bool allowDefault = true ) const;
    void setLabelProperty( const std::string & prop );

    virtual std::vector<float> voxelSize() const;
    virtual void setVoxelSize( const std::vector<float> & vs );
    void clearLabelsVolume();
    void setLabelsVolumeDimension( unsigned dx, unsigned dy, unsigned dz );
    void setLabelsVolumeDimension( const Point3d & vd );
    void setLabelsVolumeDimension( const Point3dl & vd );
    Point3dl labelsVolumeDimension() const;
    carto::VolumeRef<AObject *>& volumeOfLabels( int t = 0 );
    virtual void createDefaultPalette( const std::string & name  );
    virtual void setPalette( const AObjectPalette & pal );
    /// to be called when there has been a structural modif on the AIMS graph
    void updateAfterAimsChange();

    virtual GLComponent* glAPI();
    virtual const GLComponent* glAPI() const;
    virtual const AObjectPalette* glPalette( unsigned tex = 0 ) const;
    virtual void setProperties( carto::Object options );
    std::set<std::string> mappableVertexProperties() const;
    std::set<std::string> mappableEdgeProperties() const;

  protected:
    void saveSubObjects( bool filenamechanged = false );
    ///	Fills a volume of labels for a given time
    virtual void fillVol( carto::VolumeRef<AObject *> & vol, int t, float mx,
			  float my, float mz, float Mx, float My, float Mz );
    void recolor();
    void updateExtrema();

    ///	Data storage type, to be redefined by children classes
    typedef std::set<carto::shared_ptr<AObject> > datatype;

  private:
    friend class ::QGraphProperties;

    void initialize( const std::string & filename, bool init,
                     const Point3dl& labelDimension );

    struct Private;
    Private *d;
  };

}


namespace carto
{
  DECLARE_GENERIC_OBJECT_TYPE( anatomist::AGraph * )
}

#endif
