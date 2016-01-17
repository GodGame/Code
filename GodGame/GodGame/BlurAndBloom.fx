
#include "Define.fx"

//static const float  MIDDLE_GRAY = 0.72f;
//static const float  LUM_WHITE = 1.5f;
//static const float  BRIGHT_THRESHOLD = 0.5f;

Texture2D gtxtInput : register(t0);
RWTexture2D<float4> gtxtResult : register(u0);


cbuffer computeInfo : register(b0)
{
	uint2   g_inputSize;
	uint    g_outputwidth;
	float   g_inverse;
};

cbuffer convolution : register(b1)
{
	float gWeights2[12];// = { 0.05f, 0.05f, 0.1f, 0.1f, 0.1f, 0.2f, 0.1f, 0.1f, 0.1f, 0.05f, 0.05f };
						//	static float gGaussian[9] = { 0.0f, 1.0f, 0.0f, 1.0f, 4.0f, 1.0f, 0.0f, 1.0f, 0.0f};
};

static float gWeights[11] = { 0.05f, 0.1f, 0.1f, 0.1f, 0.1f, 0.2f, 0.1f, 0.1f, 0.1f, 0.1f, 0.05f };

cbuffer bloom
{
	static float gThreshold = 0.6f;
};

groupshared float4 gTextureCache[(256 + (2 * 5))];
groupshared float4 gTextureCache2[(480 + (2 * 5))];


[numthreads(256, 1, 1)]
void HorizonBlur(uint3 vGroupThreadID : SV_GroupThreadID, uint3 vDispatchThreadID : SV_DispatchThreadID)
{
	uint2 ftLength = gtxtInput.Length;

	if (vGroupThreadID.x  < 5)
	{
		int x = max(vDispatchThreadID.x - 5, 0);
		gTextureCache[vGroupThreadID.x] = gtxtInput[int2(x, vDispatchThreadID.y)];
	}
	else if (vGroupThreadID.x >= 256 - 5)
	{
		int x = min(vDispatchThreadID.x + 5, ftLength.x - 1);
		gTextureCache[vGroupThreadID.x + 2 * 5] = gtxtInput[int2(x, vDispatchThreadID.y)];
	}
	//gTextureCache[vGroupThreadID.x + 5] = gtxtInput[min(vDispatchThreadID.xy, gtxtInput.Length.xy - 1)];
	gTextureCache[vGroupThreadID.x + 5] = gtxtInput[min(vDispatchThreadID.xy, ftLength.xy - 1)];

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
	uint2 ftLength = gtxtInput.Length;

	if (vGroupThreadID.y  < 5)
	{
		int y = max(vDispatchThreadID.y - 5, 0);
		gTextureCache2[vGroupThreadID.y] = gtxtInput[int2(vDispatchThreadID.x, y)];
	}
	else if (vGroupThreadID.y >= 480 - 5)
	{
		int y = min(vDispatchThreadID.y + 5, ftLength.y - 1);
		gTextureCache2[vGroupThreadID.y + 2 * 5] = gtxtInput[int2(vDispatchThreadID.x, y)];
	}
	//gTextureCache[vGroupThreadID.x + 5] = gtxtInput[min(vDispatchThreadID.xy, gtxtInput.Length.xy - 1)];
	gTextureCache2[vGroupThreadID.y + 5] = gtxtInput[min(vDispatchThreadID.xy, ftLength.xy - 1)];

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


//#define kernelhalf 5
//#define groupthreads 128
//groupshared float4 temp[groupthreads];
//
//[numthreads(groupthreads, 1, 1)]
//void BrightPassAndHorizBlur(uint3 Gid : SV_GroupID, uint GI : SV_GroupIndex)
//{
//	int2 coord = int2(GI - kernelhalf + (groupthreads - kernelhalf * 2) * Gid.x, Gid.y);
//	coord = coord.xy * 8 + int2(4, 3);
//	coord = clamp(coord, int2(0, 0), int2(g_inputsize.x - 1, g_inputsize.y - 1));
//	float4 vColor = Input.Load(int3(coord, 0));
//
//	float fLum = lum[0] * g_inverse;
//
//	// Bright pass and tone mapping
//	vColor = max(0.0f, vColor - BRIGHT_THRESHOLD);
//	vColor *= MIDDLE_GRAY / (fLum + 0.001f);
//	vColor *= (1.0f + vColor / LUM_WHITE);
//	vColor /= (1.0f + vColor);
//
//	temp[GI] = vColor;
//
//	GroupMemoryBarrierWithGroupSync();
//
//	// Horizontal blur
//	if (GI >= kernelhalf &&
//		GI < (groupthreads - kernelhalf) &&
//		((Gid.x * (groupthreads - 2 * kernelhalf) + GI - kernelhalf) < g_outputwidth))
//	{
//		float4 vOut = 0;
//
//		[unroll]
//		for (int i = -kernelhalf; i <= kernelhalf; ++i)
//			vOut += temp[GI + i] * g_avSampleWeights[i + kernelhalf];
//
//		Result[GI - kernelhalf + (groupthreads - kernelhalf * 2) * Gid.x + Gid.y * g_outputwidth] = float4(vOut.rgb, 1.0f);
//	}
//}
