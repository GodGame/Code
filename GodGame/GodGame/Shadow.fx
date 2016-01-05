

Texture2D gtxtShadowMap : register(t30);
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
		float2(-dx, -dx), float2 (0.0f, -dx), float2(dx, -dx),
		float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
		float2(-dx, dx), float2(0.0f, dx), float2(dx, +dx)
	};

	[unroll]
	for (int i = 0; i < 9; ++i)
	{
		percentLit += gtxtShadowMap.SampleCmpLevelZero(gsComShadow, shadowPosH.xy + offsets[i], fDepth).r;
	}

	return percentLit *= 0.11111f;
}

float CalcOneShadowFactor( float4 shadowPos, float fMinFactor)
{
	float3 shadowPosH = shadowPos.xyz;// / shadowPos.w;
	//shadowPosH.x = shadowPosH.x * 0.5f + 0.5f;
	//shadowPosH.y = shadowPosH.y * -0.5f + 0.5f;

	float fsDepth = gtxtShadowMap.Sample(gsShadow, shadowPosH.xy).r;
	float fShadowFactor = fMinFactor;
	if (shadowPosH.z <= (fsDepth + gfBias))
		fShadowFactor = 1.0f;// fsDepth;// +gfBias;
	//else
	//	smoothstep(fMinFactor, 1.0f, shadowPosH.z);

	return fShadowFactor;
}


//struct VS_TEXTURED_LIGHTING_SHADOW_OUTPUT
//{
//	float4 position : SV_POSITION;
//	float3 positionW : POSITION;
//	float3 normalW : NORMAL;
//	float2 texCoord : TEXCOORD0;
//};
//
//VS_TEXTURED_LIGHTING_SHADOW_OUTPUT VSTexturedLightingColor(VS_TEXTURED_LIGHTING_COLOR_INPUT input)
//{
//	VS_TEXTURED_LIGHTING_SHADOW_OUTPUT output = (VS_TEXTURED_LIGHTING_COLOR_OUTPUT)0;
//	output.normalW = mul(input.normal, (float3x3)gmtxWorld);
//	output.positionW = mul(float4(input.position, 1.0f), gmtxWorld).xyz;
//	output.position = mul(float4(output.positionW, 1.0f), gmtxViewProjection);
//	output.texCoord = input.texCoord;
//
//	return(output);
//}
//
//PS_MRT_OUT PSTexturedLightingShadow(VS_TEXTURED_LIGHTING_SHADOW_OUTPUT input)
//{
//	input.normalW = normalize(input.normalW);
//	//float4 cIllumination = Lighting(input.positionW, input.normalW);
//	float4 cColor = gtxtTexture.Sample(gSamplerState, input.texCoord);// *cIllumination;
//
//	PS_MRT_OUT output;
//	output.vNormal = float4(input.normalW, 1.0f);
//	output.vPos = float4(input.positionW, 1.0f);
//	output.vDiffuse = gMaterial.m_cDiffuse;
//	output.vSpec = gMaterial.m_cSpecular;
//	output.vTxColor = cColor;
//
//	return(output);
//}

