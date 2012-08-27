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


#ifndef ANA_SURFACE_SHADER_H
#define ANA_SURFACE_SHADER_H

#include <iostream>

namespace anatomist
{
  class ViewState;
  class GLComponent;
  class AObject;
  class GLMObject;
  class ATexSurface;
  class AVolumeBase;

  class Shader 
  {
  public:
    /// Light model flags
    struct LightingModel_ {
    enum EnumType
      {
	/// no light model
        None, 
        /// Phong
        Phong,
        /// Blinn-Phong
        BlinnPhong, 
      };
    };
    typedef LightingModel_::EnumType LightingModel;
    static LightingModel_::EnumType DefaultLightingModel;

    /// Interpolation model flags
    struct InterpolationModel_ {
    enum EnumType
      {
	/// flat model
	Flat,
	/// Gouraud model
	Gouraud,
	/// Phong model
	Phong
      };
    };
    typedef InterpolationModel_::EnumType InterpolationModel;
    static InterpolationModel_::EnumType DefaultInterpolationModel;

    /// Coloring model flags
    struct ColoringModel_ {
    enum EnumType
      {
	/// Material model
	Material,
	/// Normal model
	Normal,
	/// Direction model
	Direction	
      };
    };
    typedef ColoringModel_::EnumType ColoringModel;
    static ColoringModel_::EnumType DefaultColoringModel;

    /// Material model flags
    struct MaterialModel_ {
    enum EnumType
      {
	/// Classic model
	Classic,
	/// Oren-Nayar model
	OrenNayar
      };
    };
    typedef MaterialModel_::EnumType MaterialModel;
    static MaterialModel_::EnumType DefaultMaterialModel;

    Shader();
    Shader(const Shader &);

    virtual ~Shader();

    inline void setLightingModel(LightingModel m);
    inline void setInterpolationModel(InterpolationModel m);
    inline void setColoringModel(ColoringModel m);
    inline void setMaterialModel(MaterialModel m);
    inline void setModels(LightingModel lm, InterpolationModel im,
			ColoringModel cm, MaterialModel mm);

    inline LightingModel getLightingModel(void);
    inline InterpolationModel getInterpolationModel(void);
    inline ColoringModel getColoringModel(void);
    inline MaterialModel getMaterialModel(void);


    // is shader supported by the system ?
    static bool isSupported(void);
    // is shader activated by default in anatomist ?
    static bool getAnatomistDefaultBehaviour(void);
    // is shader currently activated ?
    static bool	isActivated(void);
    // is shader-based pipeline used by default in object rendering ?
    static bool	isUsedByDefault(void);

    // bind current shader on the givien GL component
    void bind(const GLComponent &glc, const ViewState & state);
    // release the current shader (get back on the default OpenGL pipeline)
    void release(void);
    // reload current shader
    void reload(void);
    // enable shader
    void enable(void);
    // disable shader
    void disable(void);
    // lazy shader loading
    void load_if_needed(void);

    //set shader parameters for several known anatomist types
    void setShaderParameters(const GLComponent &obj, const ViewState & state) const;
    void setShaderParameters(const GLMObject &obj, const ViewState & state) const;
    void setShaderParameters(const AVolumeBase &obj, const ViewState & state) const;
    void setShaderParameters(const ATexSurface &obj, const ViewState & state) const;

    Shader &operator = (const Shader &);
    friend std::ostream &operator << (std::ostream &, 
				      const anatomist::Shader &);
    bool operator != ( const Shader & ) const;
    bool operator == ( const Shader & mat ) const
      { return( !operator != ( mat ) ); }


    static void enable_all(void);
    static void disable_all(void);

  protected:
    // force shader loading
    void load(void);
    static bool _isSupported(void);

    LightingModel	_lighting_model;
    InterpolationModel	_interpolation_model;
    ColoringModel	_coloring_model;
    MaterialModel	_material_model;

  private:
    struct Private;
    Private	*d;
  };

  std::ostream &operator << (std::ostream &, const anatomist::Shader &);

  inline void Shader::setLightingModel(LightingModel m)
  {
    _lighting_model = m;
  }

  inline void Shader::setInterpolationModel(InterpolationModel m)
  {
    _interpolation_model = m;
  }

  inline void Shader::setColoringModel(ColoringModel m)
  {
    _coloring_model = m;
  }

  inline void Shader::setMaterialModel(MaterialModel m)
  {
    _material_model = m;
  }

  inline void Shader::setModels(LightingModel lm = Shader::DefaultLightingModel,
		InterpolationModel im = Shader::DefaultInterpolationModel,
		ColoringModel cm = Shader::DefaultColoringModel,
		MaterialModel mm = Shader::DefaultMaterialModel)
  {
    _lighting_model = lm;
    _interpolation_model = im;
    _coloring_model = cm;
    _material_model = mm;
  }

  inline Shader::LightingModel Shader::getLightingModel(void)
  {
    return _lighting_model;
  }

  inline Shader::InterpolationModel Shader::getInterpolationModel(void)
  {
    return _interpolation_model;
  }

  inline Shader::ColoringModel Shader::getColoringModel(void)
  {
    return _coloring_model;
  }

  inline Shader::MaterialModel Shader::getMaterialModel(void)
  {
    return _material_model;
  }
}

#endif
