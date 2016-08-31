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


#ifndef ANA_SPARSEMATRIX_CONNECTIVITYMATRIX_H
#define ANA_SPARSEMATRIX_CONNECTIVITYMATRIX_H


#include <anatomist/mobject/objectVector.h>
#include <anatomist/surface/triangulated.h>
#include <QObject>


namespace anatomist
{

  class ATexture;
  class ConnectivityMatrixProcessingThread;


  /** Connectivity matix object: fusions sparse matrix, white mesh,
  and optionally a gyrus patch texture.
  The gyrus patch texture may be:
  - a binary texture
  - a label texture where the patch is one label
  - a label texture where the patch is all but one label (typically, 
    all but 0)
    In this latter case, sub-patch connectivity may be displayed
    using ctrl+left click
  */
  class AConnectivityMatrix : public QObject, public ObjectVector
  {
    Q_OBJECT

  public:
    enum PatchMode
    {
      ONE,
      ALL_BUT_ONE,
      ALL_MESH,
    };

    AConnectivityMatrix( const std::vector<AObject *> & obj );
    virtual ~AConnectivityMatrix();

    virtual int MType() const { return( type() ); }
    virtual bool render( PrimList &, const ViewState & );
    virtual void update( const Observable *observable, void *arg );

    void buildTexture( int mesh_index, uint32_t vertex, float time_pos = 0 );
    void buildColumnTexture( int mesh_index, uint32_t vertex,
                             float time_pos = 0 );
    void buildPatchTexture( int mesh_index, uint32_t vertex,
                            float time_pos = 0 );
    void buildColumnPatchTexture( int mesh_index, uint32_t vertex,
                                  float time_pos = 0 );

    std::vector<carto::rc_ptr<ATriangulated> > meshes() const;
    std::vector<carto::rc_ptr<ATexture> > textures() const;
    const carto::rc_ptr<ATriangulated> marker() const;

    static bool checkObjects( const std::set<AObject *> & objects,
                              AObject * & matrix,
                              std::list<ATriangulated *> & meshes,
                              std::list<ATexture *> & patch_textures,
                              std::list<ATexture *> & basin_textures,
                              PatchMode & pmode, std::set<int> & patches );
    void cancelThread();

  private:
    struct Private;
    Private *d;
    friend class anatomist::ConnectivityMatrixProcessingThread;

    void buildPatchIndices();
    void buildPatchTextureThread();

  private slots:
    void releaseAnaCursor();

  signals:
    void texturesUpdated( AConnectivityMatrix* );
  };


}

#endif


