Texture2D     gtxtTexture            : register(t0);
SamplerState  gSamplerState          : register(s0);
			  
Texture2D     gtxtDetailTexture      : register(t1);
SamplerState  gDetailSamplerState    : register(s1);
			  
Texture2D     gtxtSlpatDetail        : register(t2);
			  
TextureCube   gtxtSkyBox             : register(t2);
SamplerState  gssSkyBox              : register(s2);
			  
Texture2D     gtxtSFTexture          : register(t2);
			  
//Texture2D   gtxtResult             : register(t16);
Texture2D     gtxtTxColor            : register(t17);
Texture2D     gtxtPos                : register(t18);
Texture2D     gtxtDiffuse            : register(t19);	// 재질 디퓨즈 일반적 텍스쳐 색상
Texture2D     gtxtSpecular           : register(t20);	// 재질 스펙큘러
Texture2D     gtxtNormal             : register(t21);
Texture1D     gtxtRandom             : register(t22);

Texture2DArray gTextureArray : register(t10);

#define	FRAME_BUFFER_WIDTH		1280
#define	FRAME_BUFFER_HEIGHT		960


//카메라 변환 행렬과 투영 변환 행렬을 위한 쉐이더 변수를 선언한다(슬롯 0을 사용).
cbuffer cbViewProjectionMatrix : register(b0)
{
	matrix gmtxViewProjection;
	float4 gf3CameraPos;

	static float gfCameraFar = 2000.0f;
	static float gfDepthFar = 0.001f;
};

//월드 변환 행렬을 위한 쉐이더 변수를 선언한다(슬롯 1을 사용).
cbuffer cbWorldMatrix : register(b1)
{
	matrix gmtxWorld : packoffset(c0);
};

cbuffer cbTerrain
{
	static int gWorldCell     = 256;
	static int gHegiht        = 512;
	static float gCameraMax   = 500.0f;
	static float gCameraMin   = 20.0f;

	static float gScaleHeight = 32.0f;
};

cbuffer cbFixed
{
	static float  gFogStart        = 20.0f;
	static float  gFogRangeInverse = 1 / 400.0f;
	static float4 gFogColor        = float4(0.1, 0.1, 0.3, 0.0);
};

cbuffer cbQuad 
{
	static float2 gvQuadTexCoord[4] = { float2(0.0f, 1.0f), float2(0.0f, 0.0f), float2(1.0f, 1.0f), float2(1.0f, 0.0f) };
};
cbuffer cbDisplacement : register(b3)
{
	float3 gBumpScale;
	float  gnBumpMax;
};

cbuffer cbShadow : register(b5)
{
	matrix gmtxShadowTransform : packoffset(c0);
}

/*(주의) register(b0)에서 b는 레지스터가 상수 버퍼를 위해 사용되는 것을 의미한다. 0는 레지스터의 번호이며
응용 프로그램에서 상수 버퍼를 디바이스 컨텍스트에 연결할 때의 슬롯 번호와 일치하도록 해야 한다.
pd3dDeviceContext->VSSetConstantBuffers(CB_SLOT_WORLD_MATRIX, 1, &m_pd3dcbWorldMatrix);*/
//정점-쉐이더의 출력을 위한 구조체이다.

struct PS_MRT_COLOR_OUT
{
	// 최대 8개 가능
	float4 color  : SV_Target0;
	float4 zDepth : SV_Target1;
	float4 colorR : SV_Target2;
	float4 colorG : SV_Target3;
	float4 colorB : SV_Target4;
};

struct PS_MRT_OUT
{
	// 최대 8개 가능
	float4 vTxColor : SV_Target0;
	float4 vPos		: SV_Target1;
	float4 vDiffuse : SV_Target2;
	float4 vSpec	: SV_Target3;
	float4 vNormal  : SV_Target4;
	//float4 vDepth : SV_Target4;
};

float4 FogColor(float4 color, float flerp)
{
	return lerp(color, gFogColor, flerp);
}

float4 FogExp(float4 color, float distance, float fFogDestiny)
{
	float fRate = 1.1f - saturate((distance - gFogStart) * gFogRangeInverse);
	float f = exp(-(fRate * fFogDestiny));
	return lerp(color, gFogColor, f);
}

float4 FogLerp(float4 color, float distance)
{
	float f = saturate((distance - gFogStart) * gFogRangeInverse);
	//return lerp(color, gFogColor, smoothstep(0, 1, f));
	return lerp(color, gFogColor, f);
}

