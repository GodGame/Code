#include "Define.fx"

struct SSAO_PSIN
{
	float4 posH			: SV_POSITION;
	float3 ToFarPlane	: TEXCOORD0;
	float2 Tex			: TEXCOORD1;
};
cbuffer cbPerFrame
{
	float4x4 gViewToTexSpace; // ���� ��� * �ؽ��� ���
	float4 gOffsetVectors[14];
	float4 gFrustumCorners[4];

	// ��������
	float gOcclusionRadius = 0.5f;
	float gOcclusionFadeStart = 0.2f;
	float gOcclusionFadeEnd = 2.0f;
	float gSurfaceEpsilon = 0.05f;
};

// ǥ���� q�� p�� �󸶳� ���������� distZ(���� ����)�� ���
float OcclusionFunction(float distZ)
{
	float occlusion = 0.0f;

	if (distZ > gSurfaceEpsilon)
	{
		float fadeLength = gOcclusionFadeEnd - gOcclusionFadeStart;
		// distZ�� gOcclusionFadeStart ���� gOcclusionFadeEnd�� �����Կ� ����
		// ���󵵸� 1���� 0���� ���� ����
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
	// P-- ���� �ֺ��� ���� ����ϰ��� �ϴ� �ȼ��� �ش��ϴ� ��
	// n-- p������ ���� ����
	// q-- p �ֺ��� �� ������ ��(ǥ����)
	// r-- p�� ���� ���ɼ��� �ִ� ������ ������

	// �� �ȼ��� �þ� ���� ������ z ������ �����´�. ���� �������Ǵ� ȭ�� ��ü �簢�� �ؽ��� ��ǥ�� �̹� uv�ȿ� �ִ�.
	//float4 normalDepth = 

}

