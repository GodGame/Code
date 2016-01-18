
#include "Define.fx"
#include "Shadow.fx"

cbuffer cbPS : register(b0)
{
	uint4    g_param;
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
	float4 color = gtxtTexture.Load(uvm);
	//if (color.x > 100.0f)
	//{
	//	color.xyz /= 2000.0f;
	//}
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
	int3 uvm = int3(input.pos.xy, 0);	// (u, v, level)
	float4 normal = gtxtNormal.Load(uvm);
	float3 pos = gtxtPos.Load(uvm).xyz;
	float4 diffuse = gtxtDiffuse.Load(uvm);
	float4 specular = gtxtSpecular.Load(uvm);
	float4 txColor = /*pow(*/gtxtTxColor.Load(uvm)/*, 2.2)*/;//GammaToneMapping(gtxtTxColor.Load(uvm));

	float fLightingAmount = 0;
	float4 color;
	if (diffuse.a == 0.0)
	{
		color = lerp(txColor, gFogColor, 0.9f);
	}
	else
	{
		//matrix shadowProj = mul(gmtxWorld, gmtxShadowTransform);
		float4 shadowPos = mul(float4(pos, 1.0f), gmtxShadowTransform);

		float fShadowFactor = 0.3f;
#ifdef SHADOW_PCF
		fShadowFactor = CalcShadowFactorByPCF(shadowPos);
#else
		fShadowFactor = CalcOneShadowFactor(shadowPos, fShadowFactor);
#endif
		color = Lighting(pos, normal, float4(diffuse.rgb, fShadowFactor), specular);
		//return float4(ColorToLum(color), 1);
		color += DirectLighting(pos, normal, float4(diffuse.rgb, fShadowFactor), specular);
	//	float4 lights = color;
	//	lights.x = min(1.0f, lights.x);
	//	lights.y = min(1.0f, lights.y);
	//	lights.z = min(1.0f, lights.z);
	//	fLightingAmount = dot( float4(0.333, 0.333, 0.333, 0), lights);
	//	fLightingAmount =  max(max(lights.x, lights.y), lights.z);
		color *= txColor;
		//color = GammaToneMapping(color);
		//color.a = fLightingAmount;
		//color = pow(color, 2.2);
		float distance = length(pos - gvCameraPosition.xyz);

		float fogLerp = saturate((distance - gFogStart) / gFogRange);
		// Blend the fog color and the lit color.
		color = lerp(color, gFogColor, fogLerp);
	}

	//rgb.a = fLightingAmount;
	return ColorToLum(color).rgbr;//float4(LumToColor(ColorToLum(rgb)), 1);
}

float4 DumpMap(PS_SCENE_INPUT input) : SV_Target
{
	float2 Tex = float2((float)input.pos.x / (float)g_param.x, (float)input.pos.y / (float)g_param.y);
	return gtxtTexture.Sample(gSamplerState, Tex);
}




