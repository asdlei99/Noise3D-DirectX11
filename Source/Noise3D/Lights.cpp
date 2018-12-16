
/***********************************************************************

											  cpp�� Lights

			Desc��def of light interfaces and light description structure

************************************************************************/
#include "Noise3D.h"

using namespace Noise3D;
using namespace Noise3D::Ut;

//-------------------Base Light--------------------------
IBaseLight::IBaseLight():
	m_pShadowMapDSV(nullptr),
	mIsCastingShadowEnabled(true)
{

}

void IBaseLight::SetAmbientColor(const NVECTOR3 & color)
{
	mBaseLightDesc.ambientColor = Clamp(color, NVECTOR3(0.0f, 0.0f, 0.0f), NVECTOR3(1.0f, 1.0f, 1.0f));
}

void IBaseLight::SetDiffuseColor(const NVECTOR3 & color)
{
	mBaseLightDesc.diffuseColor = Clamp(color, NVECTOR3(0.0f, 0.0f, 0.0f), NVECTOR3(1.0f, 1.0f, 1.0f));
}

void IBaseLight::SetSpecularColor(const NVECTOR3 & color)
{
	mBaseLightDesc.specularColor = Clamp(color, NVECTOR3(0.0f, 0.0f, 0.0f), NVECTOR3(1.0f, 1.0f, 1.0f));
}

void IBaseLight::SetSpecularIntensity(float specInt)
{
	mBaseLightDesc.specularIntensity = Clamp(specInt, 0.0f, 100.0f);
}

void IBaseLight::SetDiffuseIntensity(float diffInt)
{
	mBaseLightDesc.diffuseIntensity = Clamp(diffInt, 0.0f, 100.0f);
}

void IBaseLight::SetDesc(const N_CommonLightDesc & desc)
{
	SetDiffuseColor(desc.diffuseColor);
	SetAmbientColor(desc.ambientColor);
	SetSpecularColor(desc.specularColor);
	SetSpecularIntensity(desc.specularIntensity);
	SetDiffuseIntensity(desc.diffuseIntensity);
}

void IBaseLight::GetDesc(N_CommonLightDesc & outDesc)
{
	outDesc.ambientColor = mBaseLightDesc.ambientColor;
	outDesc.diffuseColor = mBaseLightDesc.diffuseColor;
	outDesc.specularColor = mBaseLightDesc.specularColor;
	outDesc.diffuseIntensity = mBaseLightDesc.diffuseIntensity;
	outDesc.specularIntensity = mBaseLightDesc.specularIntensity;
}



//--------------------DYNAMIC DIR LIGHT------------------
DirLight::DirLight()
{
	ZeroMemory(this, sizeof(*this));
	mLightDesc.specularIntensity = 1.0f;
	mLightDesc.direction = NVECTOR3(1.0f, 0, 0);
	mLightDesc.diffuseIntensity = 0.5;
}

DirLight::~DirLight()
{
}

void DirLight::SetDirection(const NVECTOR3& dir)
{
	//the length of directional vector must be greater than 0
	if (!(dir.x == 0 && dir.y == 0 && dir.z == 0))
	{
		mLightDesc.direction = dir;
	}
}

void DirLight::SetDesc(const N_DirLightDesc & desc)
{
	IBaseLight::SetDesc(desc);//only modify the common part
	SetDirection(desc.direction);//modify extra part
}

N_DirLightDesc DirLight::GetDesc()
{
	//fill in the common attribute part
	IBaseLight::GetDesc(mLightDesc);
	return mLightDesc;
}




//--------------------DYNAMIC POINT LIGHT------------------
PointLight::PointLight()
{
	mLightDesc.specularIntensity = 1.0f;
	mLightDesc.mAttenuationFactor = 0.05f;
	mLightDesc.mLightingRange = 100.0f;
	mLightDesc.diffuseIntensity = 0.5;
}

PointLight::~PointLight()
{
}

void PointLight::SetPosition(const NVECTOR3 & pos)
{
	mLightDesc.mPosition = pos;
}

void PointLight::SetAttenuationFactor(float attFactor)
{
	mLightDesc.mAttenuationFactor = Clamp(attFactor,0.0f,1.0f);
}

void PointLight::SetLightingRange(float range)
{
	mLightDesc.mLightingRange = Clamp(range, 0.0f, 10000000.0f);
}

void PointLight::SetDesc(const N_PointLightDesc & desc)
{
	IBaseLight::SetDesc(desc);
	SetPosition(desc.mPosition);
	SetAttenuationFactor(desc.mAttenuationFactor);
	SetLightingRange(desc.mLightingRange);
}

N_PointLightDesc PointLight::GetDesc()
{
	//fill in the common attribute part
	IBaseLight::GetDesc(mLightDesc);
	return mLightDesc;
}



//--------------------DYNAMIC SPOT LIGHT------------------

SpotLight::SpotLight()
{
	mLightDesc.specularIntensity = 1.0f;
	mLightDesc.mAttenuationFactor = 1.0f;
	mLightDesc.mLightingRange = 100.0f;
	mLightDesc.mLightingAngle = Ut::PI / 4.0f;
	mLightDesc.diffuseIntensity = 0.5;
	mLightDesc.mLitAt = NVECTOR3(1.0f, 0, 0);
	mLightDesc.mPosition = NVECTOR3(0, 0, 0);
}

SpotLight::~SpotLight()
{
}

void SpotLight::SetPosition(const NVECTOR3 & pos)
{
	NVECTOR3 deltaVec = pos - mLightDesc.mLitAt;

	//pos and litAt can't superpose
	if (!(deltaVec.x == 0 && deltaVec.y == 0 && deltaVec.z == 0))
	{
		mLightDesc.mPosition = pos;
	}
}

void SpotLight::SetAttenuationFactor(float attFactor)
{
	mLightDesc.mAttenuationFactor = Clamp(attFactor,0.0f,1.0f);
}

void SpotLight::SetLitAt(const NVECTOR3 & vLitAt)
{
	NVECTOR3 deltaVec = vLitAt - mLightDesc.mPosition;

	//pos and litAt can't superpose
	if (!(deltaVec.x == 0 && deltaVec.y == 0 && deltaVec.z == 0))
	{
		mLightDesc.mLitAt = vLitAt;
	}
}

void SpotLight::SetLightingAngle(float coneAngle_Rad)
{
	// i'm not sure...but spot light should have a cone angle smaller than ��...??
	mLightDesc.mLightingAngle = Clamp(coneAngle_Rad, 0.0f, Ut::PI-0.001f);
}

void SpotLight::SetLightingRange(float range)
{
	mLightDesc.mLightingRange = Clamp(range, 0.0f, 10000000.0f);
}

void SpotLight::SetDesc(const N_SpotLightDesc & desc)
{
	IBaseLight::SetDesc(desc);
	SetPosition(desc.mPosition);
	SetLitAt(desc.mLitAt);
	SetAttenuationFactor(desc.mAttenuationFactor);
	SetLightingRange(desc.mLightingRange);
	SetLightingAngle(desc.mLightingAngle);
}

N_SpotLightDesc SpotLight::GetDesc()
{
	//fill in the common attribute part
	IBaseLight::GetDesc(mLightDesc);
	return mLightDesc;
}

