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


#ifndef ANA_FUSION_DEFFUSIONMETHODS_H
#define ANA_FUSION_DEFFUSIONMETHODS_H


#include <anatomist/fusion/fusionFactory.h>


namespace anatomist
{

  /**	Default fusion method for Fusion2D objects
   */
  class Fusion2dMethod : public FusionMethod
  {
  public:
    Fusion2dMethod() {}
    virtual ~Fusion2dMethod() {}

    virtual int canFusion( const std::set<AObject *> & );
    ///	creates the fusion
    virtual AObject* fusion( const std::vector<AObject *> & );
    ///	identifier for the method
    virtual std::string ID() const;
    virtual std::string generatedObjectType() const;

  protected:

  private:
  };


  /**	Default fusion method for Fusion3D objects.

        <b>!!! WARNING !!!</b>

	Do not confuse with Fusion3DMethod enum (which should NOT be 
	global !!), see in anatomist/mobject/Fusion3D.h
  */
  class Fusion3dMethod : public FusionMethod
  {
  public:
    Fusion3dMethod() {}
    virtual ~Fusion3dMethod() {}
    virtual int canFusion( const std::set<AObject *> & );
    ///	creates the fusion
    virtual AObject* fusion( const std::vector<AObject *> & );
    ///	identifier for the method
    virtual std::string ID() const;
    virtual std::string generatedObjectType() const;

  protected:

  private:
  };


  class PlanarFusion3dMethod : public FusionMethod
  {
  public:
    PlanarFusion3dMethod() {}
    virtual ~PlanarFusion3dMethod() {}
    virtual int canFusion( const std::set<AObject *> & );
    ///	creates the fusion
    virtual AObject* fusion( const std::vector<AObject *> & );
    ///	identifier for the method
    virtual std::string ID() const;
    virtual std::string generatedObjectType() const;

  protected:

  private:
  };


  class FusionTextureMethod : public FusionMethod
  {
  public:
    FusionTextureMethod() {}
    virtual ~FusionTextureMethod() {}
    virtual int canFusion( const std::set<AObject *> & );
    virtual AObject* fusion( const std::vector<AObject *> & );
    virtual std::string ID() const;
    virtual std::string generatedObjectType() const;
  };


  class FusionMultiTextureMethod : public FusionMethod
  {
  public:
    FusionMultiTextureMethod() {}
    virtual ~FusionMultiTextureMethod() {}
    virtual int canFusion( const std::set<AObject *> & );
    virtual AObject* fusion( const std::vector<AObject *> & );
    virtual std::string ID() const;
    virtual std::string generatedObjectType() const;
  };


  class FusionCutMeshMethod : public FusionMethod
  {
  public:
    FusionCutMeshMethod() {}
    virtual ~FusionCutMeshMethod() {}
    virtual int canFusion( const std::set<AObject *> & );
    virtual AObject* fusion( const std::vector<AObject *> & );
    virtual std::string ID() const;
    virtual std::string generatedObjectType() const;
  };

  class Fusion2DMeshMethod : public FusionMethod
  {
  public:
    Fusion2DMeshMethod() {}
    virtual ~Fusion2DMeshMethod() {}
    virtual int canFusion( const std::set<AObject *> & );
    virtual AObject* fusion( const std::vector<AObject *> & );
    virtual std::string ID() const;
    virtual std::string generatedObjectType() const;
  };

  class FusionSliceMethod : public FusionMethod
  {
  public:
    FusionSliceMethod() {}
    virtual ~FusionSliceMethod() {}
    virtual int canFusion( const std::set<AObject *> & );
    virtual AObject* fusion( const std::vector<AObject *> & );
    virtual std::string ID() const;
    virtual std::string generatedObjectType() const;
  };


  class FusionRGBAVolumeMethod : public FusionMethod
  {
  public:
    FusionRGBAVolumeMethod() {}
    virtual ~FusionRGBAVolumeMethod() {}
    virtual int canFusion( const std::set<AObject *> & );
    virtual AObject* fusion( const std::vector<AObject *> & );
    virtual std::string ID() const;
    virtual std::string generatedObjectType() const;
  };


  class FusionClipMethod : public FusionMethod
  {
  public:
    FusionClipMethod() {}
    virtual ~FusionClipMethod() {}
    virtual int canFusion( const std::set<AObject *> & );
    virtual AObject* fusion( const std::vector<AObject *> & );
    virtual std::string ID() const;
    virtual std::string generatedObjectType() const;
  };


  class FusionTesselationMethod : public FusionMethod
  {
  public:
    FusionTesselationMethod() {}
    virtual ~FusionTesselationMethod() {}
    virtual int canFusion( const std::set<AObject *> & );
    virtual AObject* fusion( const std::vector<AObject *> & );
    virtual std::string ID() const;
    virtual std::string generatedObjectType() const;
  };


  class ConnectivityMatrixFusionMethod : public FusionMethod
  {
  public:
    ConnectivityMatrixFusionMethod() {}
    virtual ~ConnectivityMatrixFusionMethod() {}
    virtual int canFusion( const std::set<AObject *> & );
    virtual AObject* fusion( const std::vector<AObject *> & );
    virtual std::string ID() const;
    virtual bool orderingMatters() const { return false; }
    virtual std::string generatedObjectType() const;
  };

  class VectorFieldFusionMethod : public FusionMethod
  {
  public:
    VectorFieldFusionMethod() {}
    virtual ~VectorFieldFusionMethod() {}
    virtual int canFusion( const std::set<AObject *> & );
    virtual AObject* fusion( const std::vector<AObject *> & );
    virtual std::string ID() const;
    virtual std::string generatedObjectType() const;
  };

}


#endif
