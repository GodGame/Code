Texture2D gtxtStaticShadow : register(t30);
Texture2D gtxtShadowMap : register(t31);
//#define SHADOW
#define SHADOW_PCF
SamplerComparisonState gsComShadow : register(s4);
SamplerState gsShadow : register(s3);
//b4 : Particle


cbuffer stShadow
{
	static float gfBias = 0.0006f;
};



float CalcShadowFactorByPCF(/*SamplerComparisonState ssShadow, Texture2D shadowMap,*/ float4 shadowPos)
{
	float3 shadowPosH = shadowPos.xyz;// / shadowPos.w;

	float fDepth = shadowPosH.z;

	const float dx = 1.0 / 2048.0f;
	float percentLit = 0.0f;
	const float2 offsets[9] =
	{
		float2(-dx, -dx),  float2 (0.0f, -dx), float2(dx, -dx),
		float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
		float2(-dx, dx),   float2(0.0f, dx),   float2(dx, dx)
	};

	[unroll]
	for (int i = 0; i < 9; ++i)
	{
		percentLit +=
			min(
				gtxtShadowMap.SampleCmpLevelZero(gsComShadow, shadowPosH.xy + offsets[i], fDepth).r,
				gtxtStaticShadow.SampleCmpLevelZero(gsComShadow, shadowPosH.xy + offsets[i], fDepth).r
				);
	}

	return (percentLit *= 0.11112f);
}

float CalcOneShadowFactor( float4 shadowPos, float fMinFactor)
{
	float3 shadowPosH = shadowPos.xyz;// / shadowPos.w;
	//shadowPosH.x = shadowPosH.x * 0.5f + 0.5f;
	//shadowPosH.y = shadowPosH.y * -0.5f + 0.5f;

	float fsDepth = 
		min(gtxtShadowMap.Sample(gsShadow, shadowPosH.xy).r, 
			gtxtStaticShadow.Sample(gsShadow, shadowPosH.xy).r);

	float fShadowFactor = fMinFactor;
	if (shadowPosH.z <= (fsDepth + gfBias))
		fShadowFactor = 1.0f;// fsDepth;// +gfBias;
	//else
	//	smoothstep(fMinFactor, 1.0f, shadowPosH.z);

	return fShadowFactor;
}
