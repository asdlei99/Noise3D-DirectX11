/***********************************************************************

                           h��NoiseLightManager

************************************************************************/

#pragma once

namespace Noise3D
{
	//(dynamic) light type (static lights are managed by GI system (which is not implemented yet 2018.12.16))
	enum NOISE_LIGHT_TYPE
	{
		NOISE_LIGHT_TYPE_DYNAMIC_DIR = 0,
		NOISE_LIGHT_TYPE_DYNAMIC_POINT = 1,
		NOISE_LIGHT_TYPE_DYNAMIC_SPOT = 2
	};

	class /*_declspec(dllexport)*/ LightManager:
		IFactory<DirLight>,
		IFactory<PointLight>,
		IFactory<SpotLight>
	{
	public:

		DirLight*			CreateDynamicDirLight(N_UID lightName);

		PointLight*		CreateDynamicPointLight(N_UID lightName);

		SpotLight*			CreateDynamicSpotLight(N_UID lightName);

		DirLight*			GetDirLight(N_UID lightName);

		DirLight*			GetDirLight(UINT index);

		PointLight*		GetPointLight(N_UID lightName);
		
		PointLight*		GetPointLight(UINT index);

		SpotLight*		GetSpotLight(N_UID lightName);
		
		SpotLight*		GetSpotLight(UINT index);


		bool		DeleteDirLight(N_UID lightName);

		bool		DeleteDirLight(DirLight* pLight);

		bool		DeletePointLight(N_UID lightName);

		bool		DeletePointLight(PointLight* pLight);

		bool		DeleteSpotLight(N_UID lightName);

		bool		DeleteSpotLight(SpotLight* pLight);

		void		SetDynamicLightingEnabled(bool isEnabled);

		bool		IsDynamicLightingEnabled();

		UINT	GetLightCount(NOISE_LIGHT_TYPE lightType);

		UINT	GetTotalLightCount();


	private:

		friend  Renderer;//access to 'CanUpdateStaticLights'

		friend IFactory<LightManager>;

		LightManager();

		~LightManager();

		bool		mIsDynamicLightingEnabled;
		bool		mCanUpdateStaticLights;

	};
}