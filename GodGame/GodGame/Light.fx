#define MAX_LIGHTS		4 
#define POINT_LIGHT		1.0f
#define SPOT_LIGHT		2.0f
#define DIRECTIONAL_LIGHT	3.0f

#define _WITH_LOCAL_VIEWER_HIGHLIGHTING
#define _WITH_THETA_PHI_CONES
//#define _WITH_REFLECT


//재질을 위한 구조체를 선언한다.
struct MATERIAL
{
	float4 m_cAmbient;
	float4 m_cDiffuse;
	float4 m_cSpecular; //a = power
	float4 m_cEmissive;
};

//조명을 위한 구조체를 선언한다.
struct LIGHT
{
	float4 m_cAmbient;
	float4 m_cDiffuse;
	float4 m_cSpecular;
	float3 m_vPosition;
	float m_fRange;
	float3 m_vDirection;
	float m_nType;
	float3 m_vAttenuation;
	float m_fFalloff;
	float m_fTheta; //cos(m_fTheta)
	float m_fPhi; //cos(m_fPhi)
	float m_bEnable;
	float padding;
};

//조명을 위한 상수버퍼를 선언한다. 
cbuffer cbLight : register(b0)
{
	LIGHT gLights[MAX_LIGHTS];
	float4 gcLightGlobalAmbient;
	float4 gvCameraPosition;
};

//재질을 위한 상수버퍼를 선언한다. 
cbuffer cbMaterial : register(b1)
{
	MATERIAL gMaterial;
};
cbuffer cbAmbient
{
	static float4 gAmbient = float4(0.1f, 0.1f, 0.1f, 0.1f);
};

struct LIGHTEDCOLOR
{
	float4 m_cAmbient;
	float4 m_cDiffuse;
	float4 m_cSpecular;
};

/*방향성 조명의 효과를 계산하는 함수이다. 방향성 조명은 조명까지의 거리에 따라 조명의 양이 변하지 않는다.*/
LIGHTEDCOLOR DirectionalLight(int i, float3 vNormal, float3 vToCamera)
{
	LIGHTEDCOLOR output = (LIGHTEDCOLOR)0;

	float3 vToLight = -gLights[i].m_vDirection;
		float fDiffuseFactor = dot(vToLight, vNormal);
	//조명의 방향이 법선 벡터와 이루는 각도가 예각일 때 직접 조명의 영향을 계산한다.
	if (fDiffuseFactor > 0.0f)
	{
		//재질의 스펙큘러 파워가 0이 아닐 때만 스펙큘러 조명의 영향을 계산한다.
		if (gMaterial.m_cSpecular.a != 0.0f)
		{
#ifdef _WITH_REFLECT
			float3 vReflect = reflect(-vToLight, vNormal);
				float fSpecularFactor = pow(max(dot(vReflect, vToCamera), 0.0f), gMaterial.m_cSpecular.a);
#else
#ifdef _WITH_LOCAL_VIEWER_HIGHLIGHTING
			float3 vHalf = normalize(vToCamera + vToLight);
#else
			float3 vHalf = float3(0.0f, 1.0f, 0.0f);
#endif
				float fSpecularFactor = pow(max(dot(vHalf, vNormal), 0.0f), gMaterial.m_cSpecular.a);
#endif
			output.m_cSpecular = gMaterial.m_cSpecular * (gLights[i].m_cSpecular * fSpecularFactor);
		}
		output.m_cDiffuse = gMaterial.m_cDiffuse * (gLights[i].m_cDiffuse * fDiffuseFactor);
	}
	output.m_cAmbient = gMaterial.m_cAmbient * gLights[i].m_cAmbient;
	return(output);
}

//점 조명의 효과를 계산하는 함수이다.
LIGHTEDCOLOR PointLight(int i, float3 vPosition, float3 vNormal, float3 vToCamera)
{
	LIGHTEDCOLOR output = (LIGHTEDCOLOR)0;

	float3 vToLight = gLights[i].m_vPosition - vPosition;
		float fDistance = length(vToLight);
	//조명까지의 거리가 조명의 유효거리보다 작을 때만 조명의 영향을 계산한다.
	if (fDistance <= gLights[i].m_fRange)
	{
		vToLight /= fDistance;
		float fDiffuseFactor = dot(vToLight, vNormal);
		//조명의 방향이 법선 벡터와 이루는 각도가 예각일 때 직접 조명의 영향을 계산한다.
		if (fDiffuseFactor > 0.0f)
		{
			//재질의 스펙큘러 파워가 0이 아닐 때만 스펙큘러 조명의 영향을 계산한다.
			if (gMaterial.m_cSpecular.a != 0.0f)
			{
#ifdef _WITH_REFLECT
				float3 vReflect = reflect(-vToLight, vNormal);
					float fSpecularFactor = pow(max(dot(vReflect, vToCamera), 0.0f), gMaterial.m_cSpecular.a);
#else
#ifdef _WITH_LOCAL_VIEWER_HIGHLIGHTING
				float3 vHalf = normalize(vToCamera + vToLight);
#else
				float3 vHalf = float3(0.0f, 1.0f, 0.0f);
#endif
					float fSpecularFactor = pow(max(dot(vHalf, vNormal), 0.0f), gMaterial.m_cSpecular.a);
#endif
				output.m_cSpecular = gMaterial.m_cSpecular * (gLights[i].m_cSpecular * fSpecularFactor);
			}
			output.m_cDiffuse = gMaterial.m_cDiffuse * (gLights[i].m_cDiffuse * fDiffuseFactor);
		}
		//조명까지의 거리에 따라 조명의 영향을 계산한다.
		float fAttenuationFactor = 1.0f / dot(gLights[i].m_vAttenuation, float3(1.0f, fDistance, fDistance*fDistance));
		output.m_cAmbient = gMaterial.m_cAmbient * (gLights[i].m_cAmbient * fAttenuationFactor);
		output.m_cDiffuse *= fAttenuationFactor;
		output.m_cSpecular *= fAttenuationFactor;
	}
	return(output);
}

//스팟 조명의 효과를 계산하는 함수이다.
LIGHTEDCOLOR SpotLight(int i, float3 vPosition, float3 vNormal, float3 vToCamera)
{
	LIGHTEDCOLOR output = (LIGHTEDCOLOR)0;
	float3 vToLight = gLights[i].m_vPosition - vPosition;
	float fDistance = length(vToLight);
	//조명까지의 거리가 조명의 유효거리보다 작을 때만 조명의 영향을 계산한다.
	if (fDistance <= gLights[i].m_fRange)
	{
		vToLight /= fDistance;
		float fDiffuseFactor = dot(vToLight, vNormal);
		//조명의 방향이 법선 벡터와 이루는 각도가 예각일 때 직접 조명의 영향을 계산한다.
		if (fDiffuseFactor > 0.0f)
		{
			//재질의 스펙큘러 파워가 0이 아닐 때만 스펙큘러 조명의 영향을 계산한다.
			if (gMaterial.m_cSpecular.a != 0.0f)
			{
#ifdef _WITH_REFLECT
				float3 vReflect = reflect(-vToLight, vNormal);
				float fSpecularFactor = pow(max(dot(vReflect, vToCamera), 0.0f), gMaterial.m_cSpecular.a);
#else
#ifdef _WITH_LOCAL_VIEWER_HIGHLIGHTING
				float3 vHalf = normalize(vToCamera + vToLight);
#else
				float3 vHalf = float3(0.0f, 1.0f, 0.0f);
#endif
				float fSpecularFactor = pow(max(dot(vHalf, vNormal), 0.0f), gMaterial.m_cSpecular.a);
#endif
				output.m_cSpecular = gMaterial.m_cSpecular * (gLights[i].m_cSpecular * fSpecularFactor);
			}
			output.m_cDiffuse = gMaterial.m_cDiffuse * (gLights[i].m_cDiffuse * fDiffuseFactor);
		}
#ifdef _WITH_THETA_PHI_CONES
		float fAlpha = max(dot(-vToLight, gLights[i].m_vDirection), 0.0f);
		float fSpotFactor = pow(max(((fAlpha - gLights[i].m_fPhi) / (gLights[i].m_fTheta - gLights[i].m_fPhi)), 0.0f), gLights[i].m_fFalloff);
#else
		float fSpotFactor = pow(max(dot(-vToLight, gLights[i].m_vDirection), 0.0f), gLights[i].m_fFalloff);
#endif
		float fAttenuationFactor = 1.0f / dot(gLights[i].m_vAttenuation, float3(1.0f, fDistance, fDistance*fDistance));
		output.m_cAmbient = gMaterial.m_cAmbient * (gLights[i].m_cAmbient * fAttenuationFactor * fSpotFactor);
		output.m_cDiffuse *= fAttenuationFactor * fSpotFactor;
		output.m_cSpecular *= fAttenuationFactor * fSpotFactor;
	}
	return(output);
}

float4 Lighting(float3 vPosition, float3 vNormal)
{
	int i;
	float3 vCameraPosition = float3(gvCameraPosition.x, gvCameraPosition.y, gvCameraPosition.z);
		float3 vToCamera = normalize(vCameraPosition - vPosition);
		LIGHTEDCOLOR LightedColor = (LIGHTEDCOLOR)0;
	float4 cColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
	for (i = 0; i < MAX_LIGHTS; i++)
	{
		//활성화된 조명에 대하여 조명의 영향을 계산한다.
		if (gLights[i].m_bEnable == 1.0f)
		{
			//조명의 유형에 따라 조명의 영향을 계산한다.
			if (gLights[i].m_nType == DIRECTIONAL_LIGHT)
			{
				LightedColor = DirectionalLight(i, vNormal, vToCamera);
				cColor += (LightedColor.m_cAmbient + LightedColor.m_cDiffuse + LightedColor.m_cSpecular);
			}
			if (gLights[i].m_nType == POINT_LIGHT)
			{
				LightedColor = PointLight(i, vPosition, vNormal, vToCamera);
				cColor += (LightedColor.m_cAmbient + LightedColor.m_cDiffuse + LightedColor.m_cSpecular);
			}
			if (gLights[i].m_nType == SPOT_LIGHT)
			{
				LightedColor = SpotLight(i, vPosition, vNormal, vToCamera);
				cColor += (LightedColor.m_cAmbient + LightedColor.m_cDiffuse + LightedColor.m_cSpecular);
			}
		}
	}
	//글로벌 주변 조명의 영향을 최종 색상에 더한다.
	cColor += (gcLightGlobalAmbient * gMaterial.m_cAmbient);
	//최종 색상의 알파값은 재질의 디퓨즈 색상의 알파값으로 설정한다.
	cColor.a = gMaterial.m_cDiffuse.a;
	return(cColor);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////




/*방향성 조명의 효과를 계산하는 함수이다. 방향성 조명은 조명까지의 거리에 따라 조명의 양이 변하지 않는다.*/
LIGHTEDCOLOR DirectionalLight(int i, float3 vNormal, float3 vToCamera, float4 vDiffuse, float4 vSpec)
{
	LIGHTEDCOLOR output = (LIGHTEDCOLOR)0;

	float3 vToLight = -gLights[i].m_vDirection;
	float fDiffuseFactor = dot(vToLight, vNormal);
	//조명의 방향이 법선 벡터와 이루는 각도가 예각일 때 직접 조명의 영향을 계산한다.
	if (fDiffuseFactor > 0.0f)
	{
		//재질의 스펙큘러 파워가 0이 아닐 때만 스펙큘러 조명의 영향을 계산한다.
		if (vSpec.a != 0.0f)
		{
#ifdef _WITH_REFLECT
			float3 vReflect = reflect(-vToLight, vNormal);
			float fSpecularFactor = pow(max(dot(vReflect, vToCamera), 0.0f), vSpec.a);
#else
#ifdef _WITH_LOCAL_VIEWER_HIGHLIGHTING
			float3 vHalf = normalize(vToCamera + vToLight);
#else
			float3 vHalf = float3(0.0f, 1.0f, 0.0f);
#endif
			float fSpecularFactor = pow(max(dot(vHalf, vNormal), 0.0f), vSpec.a);
#endif
			output.m_cSpecular = vSpec * (gLights[i].m_cSpecular * fSpecularFactor);
		}
		output.m_cDiffuse = float4(vDiffuse.rgb, 1) * (gLights[i].m_cDiffuse * fDiffuseFactor);
	}
	output.m_cAmbient = gLights[i].m_cAmbient; // gAmbient *
	return(output);
}

//점 조명의 효과를 계산하는 함수이다.
LIGHTEDCOLOR PointLight(int i, float3 vToLight, float3 vNormal, float3 vToCamera, float4 vDiffuse, float4 vSpec)
{
	LIGHTEDCOLOR output = (LIGHTEDCOLOR)0;

	float fDistance = length(vToLight);
	//조명까지의 거리가 조명의 유효거리보다 작을 때만 조명의 영향을 계산한다.
	if (fDistance <= gLights[i].m_fRange)
	{
		vToLight /= fDistance;
		float fDiffuseFactor = dot(vToLight, vNormal);
		//조명의 방향이 법선 벡터와 이루는 각도가 예각일 때 직접 조명의 영향을 계산한다.
		if (fDiffuseFactor > 0.0f)
		{
			//재질의 스펙큘러 파워가 0이 아닐 때만 스펙큘러 조명의 영향을 계산한다.
			if (vSpec.a != 0.0f)
			{
#ifdef _WITH_REFLECT
				float3 vReflect = reflect(-vToLight, vNormal);
				float fSpecularFactor = pow(max(dot(vReflect, vToCamera), 0.0f), vSpec.a);
#else
#ifdef _WITH_LOCAL_VIEWER_HIGHLIGHTING
				float3 vHalf = normalize(vToCamera + vToLight);
#else
				float3 vHalf = float3(0.0f, 1.0f, 0.0f);
#endif
				float fSpecularFactor = pow(max(dot(vHalf, vNormal), 0.0f), vSpec.a);
#endif
				output.m_cSpecular = vSpec * (gLights[i].m_cSpecular * fSpecularFactor);
			}
			output.m_cDiffuse = float4(vDiffuse.rgb, 1) * (gLights[i].m_cDiffuse * fDiffuseFactor);
		}
		//조명까지의 거리에 따라 조명의 영향을 계산한다.
		float fAttenuationFactor = 1.0f / dot(gLights[i].m_vAttenuation, float3(1.0f, fDistance, fDistance*fDistance));
		output.m_cAmbient = gAmbient * (gLights[i].m_cAmbient * fAttenuationFactor);
		output.m_cDiffuse *= fAttenuationFactor;
		output.m_cSpecular *= fAttenuationFactor;
	}
	return(output);
}

//스팟 조명의 효과를 계산하는 함수이다.
LIGHTEDCOLOR SpotLight(int i, float3 vToLight, float3 vNormal, float3 vToCamera, float4 vDiffuse, float4 vSpec)
{
	LIGHTEDCOLOR output = (LIGHTEDCOLOR)0;

	float fDistance = length(vToLight);
	//조명까지의 거리가 조명의 유효거리보다 작을 때만 조명의 영향을 계산한다.
	if (fDistance <= gLights[i].m_fRange)
	{
		vToLight /= fDistance;
		float fDiffuseFactor = dot(vToLight, vNormal);
		//조명의 방향이 법선 벡터와 이루는 각도가 예각일 때 직접 조명의 영향을 계산한다.
		if (fDiffuseFactor > 0.0f)
		{
			//재질의 스펙큘러 파워가 0이 아닐 때만 스펙큘러 조명의 영향을 계산한다.
			if (vSpec.a != 0.0f)
			{
#ifdef _WITH_REFLECT
				float3 vReflect = reflect(-vToLight, vNormal);
				float fSpecularFactor = pow(max(dot(vReflect, vToCamera), 0.0f), vSpec.a);
#else
#ifdef _WITH_LOCAL_VIEWER_HIGHLIGHTING
				float3 vHalf = normalize(vToCamera + vToLight);
#else
				float3 vHalf = float3(0.0f, 1.0f, 0.0f);
#endif
					float fSpecularFactor = pow(max(dot(vHalf, vNormal), 0.0f), vSpec.a);
#endif
				output.m_cSpecular = vSpec * (gLights[i].m_cSpecular * fSpecularFactor);
			}
			output.m_cDiffuse = float4(vDiffuse.rgb, 1) * (gLights[i].m_cDiffuse * fDiffuseFactor);
		}
#ifdef _WITH_THETA_PHI_CONES
		float fAlpha = max(dot(-vToLight, gLights[i].m_vDirection), 0.0f);
		float fSpotFactor = pow(max(((fAlpha - gLights[i].m_fPhi) / (gLights[i].m_fTheta - gLights[i].m_fPhi)), 0.0f), gLights[i].m_fFalloff);
#else
		float fSpotFactor = pow(max(dot(-vToLight, gLights[i].m_vDirection), 0.0f), gLights[i].m_fFalloff);
#endif
		float fAttenuationFactor = 1.0f / dot(gLights[i].m_vAttenuation, float3(1.0f, fDistance, fDistance*fDistance));
		output.m_cAmbient = gAmbient * (gLights[i].m_cAmbient * fAttenuationFactor * fSpotFactor);
		output.m_cDiffuse *= fAttenuationFactor * fSpotFactor;
		output.m_cSpecular *= fAttenuationFactor * fSpotFactor;
	}
	return(output);
}

float4 HemisphericLight(float3 vNormal, float3 vPos)
{
	const static float4 cModel = { 1.0f, 1.0f, 1.0f, 1.0f };
	const static float4 cGround = { 0.2f, 0.2f, 0.2f, 1.0f };
	const static float3 vFromSky = { 0.0f, 1.0f, 0.0f };

	//float3 normal = normalize(vNormal);
	float3 fromCamera = normalize(vPos - gvCameraPosition.xyz);
	//float3 vReflected = reflect(vNormal, fromCamera);

	//return (0.9f * lerp(cGround, cModel, dot(vNormal, vFromSky) * 0.5f + 0.5f));
	return (0.9f * lerp(cGround, cModel, dot(-vNormal, fromCamera) * 0.5f + 0.5f));
}

float CookTorrenceSF(float3 vNormal,float3 vToCamera, float3 vToLight, float fm, float fFRI)
{
	float3 N = normalize(vNormal);
	float3 L = normalize(vToLight);
	float3 E = normalize(vToCamera);
	float3 H = normalize(vToLight + vToCamera);

	float NH = saturate(dot(N, H));
	float EH = saturate(dot(E, H));
	float NE = saturate(dot(N, E));
	float NL = saturate(dot(N, L));
	
	float NH2 = NH * NH;
	float m2 = fm * fm;

	float D = (0.25f * m2 * NH2 * NH2) * (exp(-((1 - NH2) / (m2 * NH2))));
	float G = min(1.0f, min((2 * NH * NL) / EH, (2 * NH * NE) / EH));
	float F = fFRI + (1 - fFRI) * pow((1 - NE), 5.0f);
	float fSF = (F * D * G) / (3.1415926535 * NL * NE);

	return fSF;
}

float4 Lighting(float3 vPos, float3 vNormal, float4 vDiff, float4 vSpecular)
{
	int i;
	float4 vDiffuse = vDiff;
	float fShadowFactor = vDiff.a;
	float4 vSpec = float4(vSpecular.xyz, vSpecular.w * 255.0f);
	float3 vCameraPosition = float3(gvCameraPosition.x, gvCameraPosition.y, gvCameraPosition.z);
	float3 vToCamera = normalize(vCameraPosition - vPos);
	LIGHTEDCOLOR LightedColor = (LIGHTEDCOLOR)0;

	float4 cColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
	for (i = 1; i < MAX_LIGHTS; i++)
	{
		//활성화된 조명에 대하여 조명의 영향을 계산한다.
		if (gLights[i].m_bEnable == 1.0f)
		{
			float3 vToLight = gLights[i].m_vPosition - vPos;
			//vSpec.w = CookTorrenceSF(vNormal, vToCamera, vToLight, 0.85f, 0.01f);
			//vSpec.w = max(0.2, vSpec.w);
			vSpec.w = 255.0f;

			//조명의 유형에 따라 조명의 영향을 계산한다.
			//if (gLights[i].m_nType == DIRECTIONAL_LIGHT)
			//{
			//	LightedColor = DirectionalLight(i, vNormal, vToCamera, vDiffuse, vSpec);
			//	cColor += (LightedColor.m_cAmbient + LightedColor.m_cDiffuse  * fShadowFactor + LightedColor.m_cSpecular * fShadowFactor);
			//}
			if (gLights[i].m_nType == POINT_LIGHT)
			{
				LightedColor = PointLight(i, vToLight, vNormal, vToCamera, vDiffuse, vSpec);
				cColor += (LightedColor.m_cAmbient + LightedColor.m_cDiffuse  * fShadowFactor + LightedColor.m_cSpecular * fShadowFactor);
			}
			if (gLights[i].m_nType == SPOT_LIGHT)
			{
				LightedColor = SpotLight(i, vToLight, vNormal, vToCamera, vDiffuse, vSpec);
				cColor += (LightedColor.m_cAmbient + LightedColor.m_cDiffuse  + LightedColor.m_cSpecular );
			}
		}
	}
	//글로벌 주변 조명의 영향을 최종 색상에 더한다.
	cColor += (gcLightGlobalAmbient  /** HemisphericLight( vNormal, vPos)*//*gAmbient*/ );//gMaterial.m_cAmbient);
	//cColor *= HemisphericLight(vNormal, vPos);
	//최종 색상의 알파값은 재질의 디퓨즈 색상의 알파값으로 설정한다.
	//cColor.a = gMaterial.m_cDiffuse.a;
	return(cColor);
}

float4 DirectLighting(float3 vPos, float3 vNormal, float4 vDiff, float4 vSpecular)
{
	float4 vDiffuse = vDiff;
	float fShadowFactor = vDiff.a;
	float4 vSpec = float4(vSpecular.xyz, vSpecular.w * 255.0f);
	float3 vCameraPosition = float3(gvCameraPosition.x, gvCameraPosition.y, gvCameraPosition.z);
	float3 vToCamera = normalize(vCameraPosition - vPos);
	LIGHTEDCOLOR LightedColor = (LIGHTEDCOLOR)0;

	float4 cColor = float4(0.0f, 0.0f, 0.0f, 0.0f);

	LightedColor = DirectionalLight(0, vNormal, vToCamera, vDiffuse, vSpec);
	cColor += (LightedColor.m_cAmbient + LightedColor.m_cDiffuse  * fShadowFactor + LightedColor.m_cSpecular * fShadowFactor);

	return cColor;
}


float3 NoStripAverageVertexNormal()
{
	float3 result = float3(0, 0, 0);
	return result;
}

float4 ToneMapping(float4 LinearColor)
{
	return (LinearColor * (6.2 * LinearColor + 0.5)) / (LinearColor * (6.2 * LinearColor + 1.7) + 0.06);
}

float4 GammaToneMapping(float4 GammaColor)
{
	float4 color = max(0, pow(GammaColor, 2.2) - 0.004);
	color = (color * (6.2 * color + 0.5)) / (color * (6.2 * color + 1.7) + 0.06);
	return color;
}

static const float  MIDDLE_GRAY = 0.72f;
static const float  LUM_WHITE = 1.5f;
static const float  BRIGHT_THRESHOLD = 0.5f;


float3 ColorToLum(float3 fColor)
{
	const float3x3 RGBtoXYZ = {
		0.5141364, 0.3238786, 0.16036376,
		0.265068, 0.67023428, 0.06409157,
		0.0241188, 0.1228178, 0.84442666
	};

	//const float3x3 RGBtoXYZ = {
	//	0.4124, 0.3576, 0.1805,
	//	0.2126, 0.7152, 0.7222,
	//	0.0193, 0.1192, 0.9505
	//};

	float3 XYZ = mul(RGBtoXYZ, fColor);

	float3 Yxy;
	Yxy.r = XYZ.g;
	//x = X / (X + Y + Z)
	//y = X / (X + Y + Z)

	float temp = dot(float3(1.0f, 1.0f, 1.0f), XYZ.rgb);
	Yxy.gb = XYZ.rg / temp;

	return Yxy;
}

float3 ToneMappingByLum(float3 fLum)
{
	float3 Yxy = fLum;
	float LumScaled = fLum.r * MIDDLE_GRAY / (fLum.x + 0.001f);
	Yxy.r = (LumScaled * (1.0f + LumScaled / LUM_WHITE)) / (1.0f + LumScaled);
	return Yxy;
}

float3 LumToColor(float3 fLum)
{
	float3 XYZ;
	// Tone
	XYZ.r = fLum.r * fLum.g / fLum.b;
	XYZ.g = fLum.r;
	XYZ.b = fLum.r * (1 - fLum.g - fLum.b) / fLum.b;

	const float3x3 XYZtoRGB = {
		2.5651, -1.1665, -0.3986,
		-1.0217, 1.9777, 0.0439,
		0.0753, -0.2543, 1.1892
	};

	//const float3x3 XYZtoRGB = {
	//	3.2405, -1.5371, -0.4985,
	//	-0.9693, 1.8760, 0.0416,
	//	0.0556, -0.2040, 1.0572
	//};

	return mul(XYZtoRGB, XYZ);
}