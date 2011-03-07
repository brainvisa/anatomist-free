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


#ifndef ANATOMIST_SURFMATCHER_SURFMATCHER_H
#define ANATOMIST_SURFMATCHER_SURFMATCHER_H


#include <anatomist/mobject/objectVector.h>
#include <cartobase/object/syntax.h>
#include <vector>


namespace anatomist
{
  template<int D> class ASurface;
  typedef ASurface<3> ATriangulated;
  //class ATriangulated;
  ///	internal data structure
  struct ASurfMatcher_processData;
  ///	internal data structure
  struct ASurfMatcher_ctrlPts;


  ///	Fusion object to match 2 surfaces
  class ASurfMatcher : public ObjectVector
  {
  public:
    ASurfMatcher( AObject* o1, AObject* o2 );
    virtual ~ASurfMatcher();

    virtual int MType() const { return type(); }
    virtual bool CanRemove( AObject* obj );

    virtual Tree* optionTree() const;

    static int classType() { return _classType; }
    ///	Opens the surfmatcher control window
    static void surfMatchControl( const std::set<AObject *> & );

    /// true if direction is matching ascending (object 1 -> object 2)
    bool ascending() const { return _ascending; }
    virtual void setAscending( bool asc );
    virtual void processStep();
    bool processFinished() const { return _processFinished; }
    bool record() const { return _record; }
    virtual void setRecord( bool r ) { _record = r; }
    virtual Tree parameters();
    virtual Tree parameters() const;
    virtual void setParameters( const Tree & par );
    virtual carto::SyntaxSet paramSyntax() const;
    /// resets the moving surface and associated cached data
    virtual void resetProcess();
    /// sets control points on the source surface (given in vertex indices)
    void setOrgControlPoints( const std::vector<unsigned> & pts );
    /// sets control points on the destination surface (given in 3D coords)
    void setDestControlPoints( const std::vector<Point3df> & pts );
    const std::vector<unsigned> & orgControlPoints() const;
    std::vector<unsigned> & orgControlPoints();
    const std::vector<Point3df> & destControlPoints() const;
    std::vector<Point3df> & destControlPoints();
    ATriangulated* orgSurface() const;
    ATriangulated* destSurface() const;
    ATriangulated* movingSurface() const;
    void deleteOrgCtrlPointsSurf();
    void deleteDstCtrlPointsSurf();
    void moveOrgCtrlPoints( unsigned oldtime, unsigned time );

    static Tree*	_optionTree;

  protected:
    void prepareNeighbourhood( ATriangulated* s, unsigned time );
    void computeLength( ATriangulated* s, unsigned time );

    bool				_ascending;
    bool				_processFinished;
    bool				_record;
    ASurfMatcher_processData	*_mdata;
    ASurfMatcher_ctrlPts		*_ctrlPts;

  private:
    ///	ensures the object class is registered in Anatomist
    static int registerClass();

    static int	_classType;
  };

}


#endif
