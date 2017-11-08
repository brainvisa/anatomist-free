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

#ifndef ANATOMIST_SURFACE_SHOWNORMALS_H
#define ANATOMIST_SURFACE_SHOWNORMALS_H

#include <anatomist/mobject/objectVector.h>
#include <anatomist/surface/surface.h>
#include <anatomist/fusion/fusionFactory.h>
#include <anatomist/application/module.h>


namespace anatomist
{

  /// A mesh displaying normals for other meshes
  class ANormalsMesh : public ObjectVector
  {
  public:
    ANormalsMesh( const std::vector<AObject *> & ameshes );
    virtual ~ANormalsMesh();

    ASurface<2> *normalMesh() { return _nmesh; }
    const ASurface<2> *normalMesh() const { return _nmesh; }

    void rebuild();
    void setLength( float length );
    float length() const { return _length; }

    virtual bool render( PrimList & plist, const ViewState & vs );
    virtual void update( const Observable* observable, void* arg );
    virtual ObjectMenu* optionMenu() const;
    virtual Tree* optionTree() const;

    static void editNormalsProperties( const std::set<AObject *> & );

  private:
    std::vector<ASurface<3> *> _ameshes;
    ASurface<2> * _nmesh;
    float _length;
    static float _default_length;
  };


  class NormalsFusionMethod : public FusionMethod
  {
  public:
    NormalsFusionMethod() : FusionMethod() {}
    virtual ~NormalsFusionMethod();

    virtual std::string ID() const { return "NormalsFusionMethod"; }
    virtual int canFusion( const std::set<AObject *> & objects );
    virtual AObject* fusion( const std::vector<AObject *> & objects );
    virtual std::string generatedObjectType() const;
  };

}

#endif

