#include "Light.fx"
#include "Common.fx"
#include "Shadow.fx"
#include "PostDefine.fx"

///Texture2D txScreen : register(t0);
//SamplerState gScreenSampler : register(s0);

struct VS_SCENE_INPUT
{
	float3 pos			: POSITION;
	float2 tex			: TEXCOORD;
};

struct PS_SCENE_INPUT
{
	float4 pos			: SV_POSITION;
	float2 tex			: TEXCOORD;
};


cbuffer cbPS : register(b0)
{
	float4    g_param; // : packoffset(c0);
};

PS_SCENE_INPUT VSScreen(VS_SCENE_INPUT input)
{
	PS_SCENE_INPUT output;
	output.pos = float4(input.pos, 1);
	output.tex = input.tex;

	return output;
}

float4 InfoScreen(PS_SCENE_INPUT input) : SV_Target
{
	int3 uvm = int3(input.pos.xy, 0);
	//float2 Tex = float2((float)input.pos.x / FRAME_BUFFER_WIDTH, (float)input.pos.y / FRAME_BUFFER_HEIGHT);
	//float4 color = gtxtTexture.Sample(gSamplerState, Tex);
	//float4 color = gtxtTexture.Sample(gSamplerState, input.tex);

	float4 color = gtxtTexture.Load(uvm);
	return color;
}

float4 LightScreen(PS_SCENE_INPUT input) : SV_Target
{
	int3 uvm = int3(input.pos.xy, 0);
	float color = gtxtTexture.Load(uvm);
	return float4(color.xxx, 0.0);
}

float4 PSScreen(PS_SCENE_INPUT input) : SV_Target
{
	int3 uvm        = int3(input.pos.xy, 0);	// (u, v, level)
	float4 normal   = gtxtNormal.Load(uvm);
	float3 pos      = gtxtPos.Load(uvm).xyz;
	float4 diffuse  = pow(gtxtDiffuse.Load(uvm), 2.2);	// 2.2
	float4 specular = gtxtSpecular.Load(uvm);
	float4 txColor  = pow(gtxtTxColor.Load(uvm), 2.2);// 2.2

	float fLightingAmount = 0;
	float4 color;
	if (diffuse.a == 0.0)
	{
		color = FogColor(txColor, 0.9f);
	}
	else
	{
		float4 shadowPos = mul(float4(pos, 1.0f), gmtxShadowTransform);

		float fShadowFactor = 0.3f;
#ifdef SHADOW_PCF
		fShadowFactor = CalcShadowFactorByPCF(shadowPos);
#else
		fShadowFactor = CalcOneShadowFactor(shadowPos, fShadowFactor);
#endif
		color = Lighting(pos, normal, float4(diffuse.rgb, fShadowFactor), specular) * txColor;
		float distance = length(pos - gvCameraPosition.xyz);
		color = FogLerp(color, distance);
		//color = FogExp(color, distance, 0.88f);
	}
#ifdef LUMCOLOR
	color   = float4(LumToColor(color), 1);//float4(LumToColor(ColorToLum(rgb)), 1);
	color.r = min(LUM_WHITE, color.r);
#else
	color   = min(LUM_WHITE, color);
#endif
	return color;
}

float4 DumpMap(PS_SCENE_INPUT input) : SV_Target
{
	float2 fInputSize = g_param.zw;
	float2 Tex = float2((float)input.pos.x / fInputSize.x, (float)input.pos.y / fInputSize.y);
	
	return gtxtTexture.Sample(gSamplerState, Tex);
}

float4 ScreenDraw(PS_SCENE_INPUT input) : SV_Target
{
	float4 color = gtxtTexture.Sample(gSamplerState, input.tex);

	if (color.a == 0.0) discard;

	return color;
}
