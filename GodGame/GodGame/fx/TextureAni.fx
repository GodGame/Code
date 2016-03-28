#include "Define.fx"

Texture2D gTexture     : register(t0);

cbuffer CB_TX_ANIMATION_INFO : register(b4)
{
	float	 fGameTime;
	float	 fFramePerTime;
	float2   xv2Size;

	float3	 xv3Pos;
	float	 nColorNum;

	float3	 xv3Look;
	float	 bMove;
};

struct VS_IN_TX_ANI
{
	// x : float	 fFrameWidthPercent;
	// y : float	 fFrameHeightPercent;
	float4 pos : POSITION;
};

struct PS_IN_TX_ANI
{
	float4 posH  : SV_POSITION;
	//	float3 posW  : POSITION;
	//	float fOpacity : OPACITY;
	float4 color : COLOR;
	float2 tex   : TEXCOORD;
};

VS_IN_TX_ANI VSTextureAnimate(VS_IN_TX_ANI input)
{
	return input;
}

[maxvertexcount(4)]
void GSTextureAnimate(point VS_IN_TX_ANI input[1], inout TriangleStream<PS_IN_TX_ANI> triStream)
{
	float3 pos   = xv3Pos;
	float3 vLook = normalize(gf3CameraPos.xyz - pos);;
	float3 vUp    = float3(0, 1, 0);

	if (bMove > 0)
	{
		vUp = normalize(xv3Look);// normalize(gf3CameraPos.xyz - pos);
	}
	float3 vRight = cross(vUp, normalize(vLook));

	float fHalfWidth = 0.5f * xv2Size.x, fHalfHeight = 0.5f * xv2Size.y;
	float4 vQuad[4];
	vQuad[0] = float4(pos + fHalfWidth * vRight - fHalfHeight * vUp, 1.0f);
	vQuad[1] = float4(pos + fHalfWidth * vRight + fHalfHeight * vUp, 1.0f);
	vQuad[2] = float4(pos - fHalfWidth * vRight - fHalfHeight * vUp, 1.0f);
	vQuad[3] = float4(pos - fHalfWidth * vRight + fHalfHeight * vUp, 1.0f);

	//float2 fScaled = float2(fFrameWidthPercent, fFrameHeightPercent);

	int2 nFrames;
	nFrames.x = 1.0 / input[0].pos.x;
//	nFrames.y = 1.0 / fScaled.y;
	nFrames.y = fGameTime / fFramePerTime;

	float2 fOffset;
	fOffset.x = (nFrames.y % nFrames.x) * input[0].pos.x;
	fOffset.y = (nFrames.y / nFrames.x) * input[0].pos.y;

	PS_IN_TX_ANI output;
	[unroll]
	for (int i = 0; i < 4; ++i)
	{
		output.posH = mul(vQuad[i], gmtxViewProjection);
		//		output.posW		   = vToCamera;
		output.tex = (gvQuadTexCoord[i] * input[0].pos.xy) + fOffset;

		output.color = gcColors[uint(nColorNum)];
		triStream.Append(output);
	}
}

PS_MRT_OUT PSTextureAnimate(PS_IN_TX_ANI input)
{
	PS_MRT_OUT output;

	float4 cColor =  gTexture.Sample(gSamplerState, input.tex);
	//if (cColor.a < 0.15f) discard;

	output.vNormal  = vZero;
	output.vPos     = vZero;
	output.vDiffuse = vZero;
	output.vSpec    = vZero;
	output.vTxColor = (cColor * input.color);
	return(output);
}