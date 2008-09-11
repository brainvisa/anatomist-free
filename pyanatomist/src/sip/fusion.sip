
%ModuleCode
#if SIP_VERSION >= 0x040700
#include <anatomist/fusion/defFusionMethods.h>
#include <anatomist/surface/fusiontexsurf.h>
#endif
%End

namespace anatomist
{

class FusionMethod
{
%TypeHeaderCode
#include <anatomist/fusion/fusionFactory.h>
#if SIP_VERSION < 0x040700
#include "sipanatomistsipvector_AObjectPtr.h"
#include "sipanatomistsipset_AObjectPtr.h"
#endif
%End

%ConvertToSubClassCode
	if (dynamic_cast<anatomist::Fusion2dMethod *>(sipCpp))
		sipClass = sipClass_anatomist_Fusion2dMethod;
	else if (dynamic_cast<anatomist::Fusion3dMethod *>(sipCpp))
		sipClass = sipClass_anatomist_Fusion3dMethod;
	else if (dynamic_cast<anatomist::FusionCutMeshMethod *>(sipCpp))
		sipClass = sipClass_anatomist_FusionCutMeshMethod;
	else if (dynamic_cast<anatomist::FusionMultiTextureMethod *>(sipCpp))
		sipClass = sipClass_anatomist_FusionMultiTextureMethod;
	else if (dynamic_cast<anatomist::FusionSliceMethod *>(sipCpp))
		sipClass = sipClass_anatomist_FusionSliceMethod;
	else if (dynamic_cast<anatomist::FusionTexSurfMethod *>(sipCpp))
		sipClass = sipClass_anatomist_FusionTexSurfMethod;
	else if (dynamic_cast<anatomist::FusionTextureMethod *>(sipCpp))
		sipClass = sipClass_anatomist_FusionTextureMethod;
	else if (dynamic_cast<anatomist::PlanarFusion3dMethod *>(sipCpp))
		sipClass = sipClass_anatomist_PlanarFusion3dMethod;
  	else	sipClass = 0;
%End

public:
	FusionMethod();
	virtual ~FusionMethod();

	virtual bool canFusion(const set_AObjectPtr &) = 0;
	virtual anatomist::AObject *fusion(const vector_AObjectPtr &) = 0 /Factory/;

	virtual std::string ID() const = 0;
	virtual bool orderingMatters() const;
};

class Fusion2dMethod : anatomist::FusionMethod
{
%TypeHeaderCode
#include <anatomist/fusion/fusionFactory.h>
#include <anatomist/fusion/defFusionMethods.h>
#if SIP_VERSION < 0x040700
#include "sipanatomistsipvector_AObjectPtr.h"
#include "sipanatomistsipset_AObjectPtr.h"
#endif
%End
	public:
	Fusion2dMethod();
	virtual ~Fusion2dMethod();
        virtual bool canFusion(const set_AObjectPtr &);
        virtual anatomist::AObject *fusion(const vector_AObjectPtr &) /Factory/;
        virtual std::string ID() const = 0;
};

class Fusion3dMethod : anatomist::FusionMethod
{
%TypeHeaderCode
#include <anatomist/fusion/fusionFactory.h>
#include <anatomist/fusion/defFusionMethods.h>
#if SIP_VERSION < 0x040700
#include "sipanatomistsipvector_AObjectPtr.h"
#include "sipanatomistsipset_AObjectPtr.h"
#endif
%End
	public:
	Fusion3dMethod();
	virtual ~Fusion3dMethod();
        virtual bool canFusion(const set_AObjectPtr &);
        virtual anatomist::AObject *fusion(const vector_AObjectPtr &) /Factory/;
        virtual std::string ID() const = 0;
};

class FusionCutMeshMethod : anatomist::FusionMethod
{
%TypeHeaderCode
#include <anatomist/fusion/fusionFactory.h>
#include <anatomist/fusion/defFusionMethods.h>
#if SIP_VERSION < 0x040700
#include "sipanatomistsipvector_AObjectPtr.h"
#include "sipanatomistsipset_AObjectPtr.h"
#endif
%End
	public:
	FusionCutMeshMethod();
	virtual ~FusionCutMeshMethod();
        virtual bool canFusion(const set_AObjectPtr &);
        virtual anatomist::AObject *fusion(const vector_AObjectPtr &) /Factory/;
        virtual std::string ID() const = 0;
};

class FusionMultiTextureMethod : anatomist::FusionMethod
{
%TypeHeaderCode
#include <anatomist/fusion/fusionFactory.h>
#include <anatomist/fusion/defFusionMethods.h>
#if SIP_VERSION < 0x040700
#include "sipanatomistsipvector_AObjectPtr.h"
#include "sipanatomistsipset_AObjectPtr.h"
#endif
%End
	public:
	FusionMultiTextureMethod();
	virtual ~FusionMultiTextureMethod();
        virtual bool canFusion(const set_AObjectPtr &);
        virtual anatomist::AObject *fusion(const vector_AObjectPtr &) /Factory/;
        virtual std::string ID() const = 0;
};

class FusionSliceMethod : anatomist::FusionMethod
{
%TypeHeaderCode
#include <anatomist/fusion/fusionFactory.h>
#include <anatomist/fusion/defFusionMethods.h>
#if SIP_VERSION < 0x040700
#include "sipanatomistsipvector_AObjectPtr.h"
#include "sipanatomistsipset_AObjectPtr.h"
#endif
%End
	public:
	FusionSliceMethod();
	virtual ~FusionSliceMethod();
        virtual bool canFusion(const set_AObjectPtr &);
        virtual anatomist::AObject *fusion(const vector_AObjectPtr &) /Factory/;
        virtual std::string ID() const = 0;
};

class FusionTexSurfMethod : anatomist::FusionMethod
{
%TypeHeaderCode
#include <anatomist/fusion/fusionFactory.h>
#include <anatomist/surface/fusiontexsurf.h>
#if SIP_VERSION < 0x040700
#include "sipanatomistsipvector_AObjectPtr.h"
#include "sipanatomistsipset_AObjectPtr.h"
#endif
%End
	public:
	FusionTexSurfMethod();
	virtual ~FusionTexSurfMethod();
        virtual bool canFusion(const set_AObjectPtr &);
        virtual anatomist::AObject *fusion(const vector_AObjectPtr &) /Factory/;
        virtual std::string ID() const = 0;
};

class FusionTextureMethod : anatomist::FusionMethod
{
%TypeHeaderCode
#include <anatomist/fusion/fusionFactory.h>
#include <anatomist/fusion/defFusionMethods.h>
#if SIP_VERSION < 0x040700
#include "sipanatomistsipvector_AObjectPtr.h"
#include "sipanatomistsipset_AObjectPtr.h"
#endif
%End
	public:
	FusionTextureMethod();
	virtual ~FusionTextureMethod();
        virtual bool canFusion(const set_AObjectPtr &);
        virtual anatomist::AObject *fusion(const vector_AObjectPtr &) /Factory/;
        virtual std::string ID() const = 0;
};

class PlanarFusion3dMethod : anatomist::FusionMethod
{
%TypeHeaderCode
#include <anatomist/fusion/fusionFactory.h>
#include <anatomist/fusion/defFusionMethods.h>
#if SIP_VERSION < 0x040700
#include "sipanatomistsipvector_AObjectPtr.h"
#include "sipanatomistsipset_AObjectPtr.h"
#endif
%End
	public:
	PlanarFusion3dMethod();
	virtual ~PlanarFusion3dMethod();
        virtual bool canFusion(const set_AObjectPtr &);
        virtual anatomist::AObject *fusion(const vector_AObjectPtr &) /Factory/;
        virtual std::string ID() const = 0;
};

/* //FIXME : à deplacer dans un/des plugin(s)
class AInterpolerMethod : anatomist::FusionMethod
{
%TypeHeaderCode
#include <anatomist/fusion/fusionFactory.h>
#include <anatomist/interpoler/interpolMethod.h>
#if SIP_VERSION < 0x040700
#include "sipanatomistsipvector_AObjectPtr.h"
#include "sipanatomistsipset_AObjectPtr.h"
#endif
%End
	public:
	AInterpolerMethod();
	virtual ~AInterpolerMethod();
        virtual bool canFusion(const set_AObjectPtr &);
        virtual anatomist::AObject *fusion(const vector_AObjectPtr &) /Factory/;
        virtual std::string ID() const = 0;
};


class PlaneFusionMethod : anatomist::FusionMethod
{
%TypeHeaderCode
#include <anatomist/fusion/fusionFactory.h>
#include <anatomist/fusion/defFusionMethods.h>
#if SIP_VERSION < 0x040700
#include "sipanatomistsipvector_AObjectPtr.h"
#include "sipanatomistsipset_AObjectPtr.h"
#endif
%End
	public:
	PlaneFusionMethod();
	virtual ~PlaneFusionMethod();
        virtual bool canFusion(const set_AObjectPtr &);
        virtual anatomist::AObject *fusion(const vector_AObjectPtr &) /Factory/;
        virtual std::string ID() const = 0;
};

class CylinderFusionMethod : anatomist::FusionMethod
{
%TypeHeaderCode
#include <anatomist/fusion/fusionFactory.h>
#include <anatomist/fusion/defFusionMethods.h>
#if SIP_VERSION < 0x040700
#include "sipanatomistsipvector_AObjectPtr.h"
#include "sipanatomistsipset_AObjectPtr.h"
#endif
%End
	public:
	CylinderFusionMethod();
	virtual ~CylinderFusionMethod();
        virtual bool canFusion(const set_AObjectPtr &);
        virtual anatomist::AObject *fusion(const vector_AObjectPtr &) /Factory/;
        virtual std::string ID() const = 0;
};

class ASurfMatchMethod : anatomist::FusionMethod
{
%TypeHeaderCode
#include <anatomist/fusion/fusionFactory.h>
#include <anatomist/fusion/defFusionMethods.h>
#if SIP_VERSION < 0x040700
#include "sipanatomistsipvector_AObjectPtr.h"
#include "sipanatomistsipset_AObjectPtr.h"
#endif
%End
	public:
	ASurfMatchMethod();
	virtual ~ASurfMatchMethod();
        virtual bool canFusion(const set_AObjectPtr &);
        virtual anatomist::AObject *fusion(const vector_AObjectPtr &) /Factory/;
        virtual std::string ID() const = 0;
};
*/

class FusionFactory
{
%TypeHeaderCode
#include <anatomist/fusion/fusionFactory.h>
#if SIP_VERSION < 0x040700
#include "sipanatomistsipvector_AObjectPtr.h"
#endif
%End

public:
	FusionFactory();
	virtual ~FusionFactory();

	virtual anatomist::FusionMethod *chooseMethod(vector_AObjectPtr &, bool=false);
	virtual anatomist::FusionMethod *method(const std::string&) const;

	static anatomist::FusionFactory *factory();
	static bool registerMethod(anatomist::FusionMethod * /Transfer/);
	static bool canFusion(const set_AObjectPtr &);
};



};

