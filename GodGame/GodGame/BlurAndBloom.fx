
#include "PostDefine.fx"

Texture2D gtxtInput : register(t0);
RWTexture2D<float4> gtxtResult : register(u0);
StructuredBuffer<float> lum : register(t1);

#define T_INVERSE 0.00001525878

cbuffer computeInfo : register(b2)
{
	float2   g_inputSize;
	float2   g_outputSize;

	float   g_Inverse;
	float   g_Threshold;
	float2  g_param;// g_inverse, fLum, Threshold;
};

cbuffer convolution : register(b1)
{
	float4 gWeights[11];// = { 0.05f, 0.05f, 0.1f, 0.1f, 0.1f, 0.2f, 0.1f, 0.1f, 0.1f, 0.05f, 0.05f };
};

//static float gWeights[11] = { 0.05f, 0.1f, 0.1f, 0.1f, 0.1f, 0.2f, 0.1f, 0.1f, 0.1f, 0.1f, 0.05f };

float4 ToneMapByLum(float4 fColor, float fLum)
{
	float4 vColor = max(0.0f, fColor - BRIGHT_THRESHOLD);
	vColor = CalculateToneColor(vColor, fLum, CalculateMiddleGray(fLum));
	//vColor *= MIDDLE_GRAY / (fLum + 0.001f);
	//vColor *= (1.0f + vColor / LUM_WHITE);
	//vColor /= (1.0f + vColor);

	return vColor;
}

groupshared float4 gTextureCache[(256 + (2 * 5))];
groupshared float4 gTextureCache2[(240 + (2 * 5))];

[numthreads(256, 1, 1)]
void HorizonBloom(uint3 vGroupThreadID : SV_GroupThreadID, uint3 vDispatchThreadID : SV_DispatchThreadID)
{
	//uint2 ftLength = gtxtInput.Length;
	float fLum = lum[0] * g_Inverse;
	float4 vColor;
	if (vGroupThreadID.x  < 5)
	{
		int x = max(vDispatchThreadID.x - 5, 0);
		vColor = gtxtInput[int2(x, vDispatchThreadID.y)];
		gTextureCache[vGroupThreadID.x] = ToneMapByLum(vColor, fLum);
	}
	else if (vGroupThreadID.x >= 256 - 5)
	{
		int x = min(vDispatchThreadID.x + 5, g_inputSize.x - 1);
		vColor = gtxtInput[int2(x, vDispatchThreadID.y)];
		gTextureCache[vGroupThreadID.x + 2 * 5] = ToneMapByLum(vColor, fLum);
	}
	//gTextureCache[vGroupThreadID.x + 5] = gtxtInput[min(vDispatchThreadID.xy, gtxtInput.Length.xy - 1)];
	vColor = gtxtInput[min(vDispatchThreadID.xy, g_inputSize.xy - 1)];
	gTextureCache[vGroupThreadID.x + 5] = ToneMapByLum(vColor, fLum);

	GroupMemoryBarrierWithGroupSync();

	float4 cBluerredColor = float4(0, 0, 0, 0);//float4(1, 1, 1, 1);
	float4 pickColor;
	//	if (gTextureCache[vGroupThreadID.x].a >= gThreshold) {

	[unroll]
	for (int i = -5; i <= 5; ++i)
	{
		int k = vGroupThreadID.x + 5 + i;
		pickColor = gTextureCache[k];
		cBluerredColor += gWeights[i + 5] * pickColor;// *pickColor.aaaa;
	}

	gtxtResult[vDispatchThreadID.xy] = cBluerredColor; //*gWeights[6];
													   //	}
													   //	else
													   //		gtxtResult[vDispatchThreadID.xy] = gTextureCache[vGroupThreadID.x];
}

//groupshared float4 gTextureCache2[(480 + (2 * 5))];
[numthreads(1, 240, 1)]
void VerticalBloom(uint3 vGroupThreadID : SV_GroupThreadID, uint3 vDispatchThreadID : SV_DispatchThreadID)
{
	//uint2 ftLength = gtxtInput.Length;
	float fLum = lum[0] * g_Inverse;
	float4 vColor;
	if (vGroupThreadID.y  < 5)
	{
		int y = max(vDispatchThreadID.y - 5, 0);
		vColor = gtxtInput[int2(vDispatchThreadID.x, y)];
		gTextureCache2[vGroupThreadID.y] = ToneMapByLum(vColor, fLum);
	}
	else if (vGroupThreadID.y >= 240 - 5)
	{
		int y = min(vDispatchThreadID.y + 5, g_inputSize.y - 1);
		vColor = gtxtInput[int2(vDispatchThreadID.x, y)];
		gTextureCache2[vGroupThreadID.y + 2 * 5] = ToneMapByLum(vColor, fLum);
	}
	//gTextureCache2[vGroupThreadID.y + 5] = gtxtInput[min(vDispatchThreadID.xy, g_inputSize.xy - 1)];
	vColor = gtxtInput[min(vDispatchThreadID.xy, g_inputSize.xy - 1)];
	vColor = ToneMapByLum(vColor, fLum);
	gTextureCache2[vGroupThreadID.y + 5] = vColor; //gtxtInput[min(vDispatchThreadID.xy, g_inputSize.xy - 1)];

	GroupMemoryBarrierWithGroupSync();

	float4 cBluerredColor = float4(0, 0, 0, 0);//float4(1, 1, 1, 1);
	float4 pickColor;
	//	if (gTextureCache2[vGroupThreadID.y].a >= gThreshold) {
	[unroll]
	for (int i = -5; i <= 5; ++i)
	{
		int k = vGroupThreadID.y + 5 + i;
		pickColor = gTextureCache2[k];
		cBluerredColor += gWeights[i + 5] * pickColor;// *pickColor.aaaa;
	}

	gtxtResult[vDispatchThreadID.xy] = cBluerredColor;// *gWeights[6];
													  //	}
													  //	else
													  //		gtxtResult[vDispatchThreadID.xy] = gTextureCache2[vGroupThreadID.y];
}


[numthreads(256, 1, 1)]
void HorizonBlur(uint3 vGroupThreadID : SV_GroupThreadID, uint3 vDispatchThreadID : SV_DispatchThreadID)
{
	//uint2 ftLength = gtxtInput.Length;
	float fLum = lum[0] * T_INVERSE;// *(g_Inverse + 0.01f);
	float4 vColor;
	if (vGroupThreadID.x  < 5)
	{
		int x = max(vDispatchThreadID.x - 5, 0);
		gTextureCache[vGroupThreadID.x] = gtxtInput[int2(x, vDispatchThreadID.y)];
	}
	else if (vGroupThreadID.x >= 256 - 5)
	{
		int x = min(vDispatchThreadID.x + 5, g_inputSize.x - 1);
		gTextureCache[vGroupThreadID.x + 2 * 5] = gtxtInput[int2(x, vDispatchThreadID.y)];
	}
	gTextureCache[vGroupThreadID.x + 5] = gtxtInput[min(vDispatchThreadID.xy, gtxtInput.Length.xy - 1)];

	GroupMemoryBarrierWithGroupSync();

	float4 cBluerredColor = float4(0, 0, 0, 0);//float4(1, 1, 1, 1);
	float4 pickColor;
	//	if (gTextureCache[vGroupThreadID.x].a >= gThreshold) {

	[unroll]
	for (int i = -5; i <= 5; ++i)
	{
		int k = vGroupThreadID.x + 5 + i;
		pickColor = gTextureCache[k];
		cBluerredColor += gWeights[i + 5] * pickColor;// *pickColor.aaaa;
	}

	gtxtResult[vDispatchThreadID.xy] = cBluerredColor; //*gWeights[6];
													   //	}
													   //	else
													   //		gtxtResult[vDispatchThreadID.xy] = gTextureCache[vGroupThreadID.x];
}


[numthreads(1, 480, 1)]
void VerticalBlur(uint3 vGroupThreadID : SV_GroupThreadID, uint3 vDispatchThreadID : SV_DispatchThreadID)
{
	//uint2 ftLength = gtxtInput.Length;
	//float fLum = lum[0] * T_INVERSE;// *(g_Inverse + 0.01f);

	if (vGroupThreadID.y  < 5)
	{
		int y = max(vDispatchThreadID.y - 5, 0);
		gTextureCache2[vGroupThreadID.y] = gtxtInput[int2(vDispatchThreadID.x, y)];
	}
	else if (vGroupThreadID.y >= 480 - 5)
	{
		int y = min(vDispatchThreadID.y + 5, g_inputSize.y - 1);
		gTextureCache2[vGroupThreadID.y + 2 * 5] = gtxtInput[int2(vDispatchThreadID.x, y)];
	}
	gTextureCache2[vGroupThreadID.y + 5] = gtxtInput[min(vDispatchThreadID.xy, g_inputSize.xy - 1)];

	GroupMemoryBarrierWithGroupSync();

	float4 cBluerredColor = float4(0, 0, 0, 0);//float4(1, 1, 1, 1);
	float4 pickColor;
	//	if (gTextureCache2[vGroupThreadID.y].a >= gThreshold) {
	[unroll]
	for (int i = -5; i <= 5; ++i)
	{
		int k = vGroupThreadID.y + 5 + i;
		pickColor = gTextureCache2[k];
		cBluerredColor += gWeights[i + 5] * pickColor;// *pickColor.aaaa;
	}

	gtxtResult[vDispatchThreadID.xy] = cBluerredColor;// *gWeights[6];
													  //	}
													  //	else
													  //		gtxtResult[vDispatchThreadID.xy] = gTextureCache2[vGroupThreadID.y];
}
