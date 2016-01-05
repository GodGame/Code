#include "Define.fx"

struct SSAO_PSIN
{
	float4 posH			: SV_POSITION;
	float3 ToFarPlane	: TEXCOORD0;
	float2 Tex			: TEXCOORD1;
};
cbuffer cbPerFrame
{
	float4x4 gViewToTexSpace; // 투영 행렬 * 텍스쳐 행렬
	float4 gOffsetVectors[14];
	float4 gFrustumCorners[4];

	// 차폐판정
	float gOcclusionRadius = 0.5f;
	float gOcclusionFadeStart = 0.2f;
	float gOcclusionFadeEnd = 2.0f;
	float gSurfaceEpsilon = 0.05f;
};

// 표본점 q가 p를 얼마나 가리는지를 distZ(깊이 차이)로 계산
float OcclusionFunction(float distZ)
{
	float occlusion = 0.0f;

	if (distZ > gSurfaceEpsilon)
	{
		float fadeLength = gOcclusionFadeEnd - gOcclusionFadeStart;
		// distZ가 gOcclusionFadeStart 에서 gOcclusionFadeEnd로 증가함에 따라
		// 차폐도를 1에서 0으로 선형 감소
		occlusion = saturate((gOcclusionFadeEnd - distZ) / fadeLength);
	}
	return occlusion;
}

SSAO_PSIN VSSCeneSpaceAmbientOcclusion(VS_SCENE_INPUT input)
{
	SSAO_PSIN output;
	output.posH = float4(input.pos, 1.0f);
	output.ToFarPlane = gFrustumCorners[input.tex.x + (input.tex.y * 2)].xyz;
	output.Tex = input.tex;

	return output;
}

float4 PSSCeneSpaceAmbientOcclusion(SSAO_PSIN input)
{
	// P-- 지금 주벼광 차폐를 계산하고자 하는 픽셀에 해당하는 점
	// n-- p에서의 법선 벡터
	// q-- p 주변의 한 무작위 점(표본점)
	// r-- p를 가릴 가능성이 있는 잠재적 차폐점

	// 이 픽셀의 시야 공간 법선과 z 성분을 가져온다. 지금 렌더링되는 화면 전체 사각형 텍스쳐 좌표는 이미 uv안에 있다.
	//float4 normalDepth = 

}

