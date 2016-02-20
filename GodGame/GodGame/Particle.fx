#include "Define.fx"

Texture2D gtxtColor     : register(t8);
SamplerState gPTSampler : register(s8);

#define PARTICLE_TYPE_EMITTER	0
#define PARTICLE_TYPE_FLARE		1

cbuffer cbParticleInfo : register(b4)
{
	float3 gvParticleEmitPos;
	float  gfTime;
	float3 gvParticleVelocity;
	float  gfLifeTime;
	float3 gvAccel;
	float  gfTimeStep;
	float  gfNewTime;
	float2 gMaxSize;
	float  gbEnable;
};

struct PARTICLE_INPUT
{
	float3 pos      : POSITION;
	float3 velocity : VELOCITY;
	float2 size     : SIZE;
	float  age      : AGE;
	float  type     : TYPE;
};

struct PARTICLE_GSIN
{
	float3 pos   : POSITION;
	float2 size  : SIZE;
	float4 color : COLOR;
	uint   type  : TYPE;
};

struct PARTICLE_PSIN
{
	float4 posH  : SV_POSITION;
	float3 posW  : POSITION;
	float4 color : COLOR;
	float2 tex   : TEXCOORD;
};

PARTICLE_INPUT VSParticleSO(PARTICLE_INPUT input)
{
	return input;
}

[maxvertexcount(2)]
void GSParticleSO(point PARTICLE_INPUT input[1], inout PointStream<PARTICLE_INPUT> Pout)
{
	input[0].age += gfTimeStep;

	if (input[0].type == PARTICLE_TYPE_EMITTER)
	{
		if (gbEnable == 1 && input[0].age > gfNewTime)
		{
			float3 vRandom = gtxtRandom.SampleLevel(gPTSampler, gfTime, 0).xyz;
			vRandom = normalize(vRandom);

			PARTICLE_INPUT particle;// = (PARTICLE_INPUT)0;
			particle.pos		    = gvParticleEmitPos.xyz;// -(vRandom * 2);// +(gvParticleVelocity * gfTime);
			particle.size			= uint2(gMaxSize * abs(vRandom.z)).xy + 3;//, abs(vRandom.y) * gMaxSize.y + 2;
			particle.velocity       = vRandom * gvParticleVelocity;
			particle.age            = 0.0f;
			particle.type           = PARTICLE_TYPE_FLARE;
			Pout.Append(particle);

			input[0].age = 0.0f;
		}
		Pout.Append(input[0]);
	}
	else
	{
		if (input[0].age < gfLifeTime)
			Pout.Append(input[0]);
	}
}

//PARTICLE_PSIN VSParticleDraw(PARTICLE_INPUT input)
//{
//	PARTICLE_PSIN output;
//
//	float t = input.age;
//	output.posW = /*(0.5f * gvAccel * t * t) + (input.velocity * t) +*/ input.pos;
//
//	float fOpacity = 1.0f - smoothstep(0.0f, 1.0f, t);
//	output.color = float4(1.0f, 1.0f, 1.0f, fOpacity);
//
//	//output.size = input.size;
//	output.tex = float2(1, 1);
//	output.posH = mul(float4(output.posW, 1), gmtxViewProjection);
//
//	return output;
//}

PARTICLE_GSIN VSParticleDraw(PARTICLE_INPUT input)
{
	PARTICLE_GSIN output;

	float t = input.age;
	output.pos = (0.5f * gvAccel * t * t) + (input.velocity * t) + input.pos;

	float fOpacity = 1.0f - smoothstep(0.0f, 1.0f, t / gfLifeTime);
	output.color = fOpacity.rrrr;//float4(fOpacity, fOpacity, fOpacity, fOpacity);

	output.size = input.size;
	output.type = input.type;

	return output;
}

[maxvertexcount(4)]
void GSParticleDraw(point PARTICLE_GSIN input[1], inout TriangleStream<PARTICLE_PSIN> triStream)
{
	if (input[0].type == PARTICLE_TYPE_EMITTER) return;
	//if (input[0].color.a <= 0.0001f) return;

	float3 vLook  = normalize(gf3CameraPos.xyz - input[0].pos);
	float3 vRight = normalize(cross(float3(0.0f, 1.0f, 0.0f), vLook));
	float3 vUp    = cross(vLook, vRight);

	float fHalfWidth = 0.5f * input[0].size.x, fHalfHeight = 0.5f * input[0].size.y;
	float4 vQuad[4];
	vQuad[0] = float4(input[0].pos + fHalfWidth * vRight - fHalfHeight * vUp, 1.0f);
	vQuad[1] = float4(input[0].pos + fHalfWidth * vRight + fHalfHeight * vUp, 1.0f);
	vQuad[2] = float4(input[0].pos - fHalfWidth * vRight - fHalfHeight * vUp, 1.0f);
	vQuad[3] = float4(input[0].pos - fHalfWidth * vRight + fHalfHeight * vUp, 1.0f);

	PARTICLE_PSIN output;// = (PARTICLE_PSIN)0;
	for (int i = 0; i < 4; ++i)
	{
		output.posH = mul(vQuad[i], gmtxViewProjection);
		output.posW = vQuad[i].xyz;
		output.tex = gvQuadTexCoord[i];
		output.color = input[0].color;
		triStream.Append(output);
	}
}

PS_MRT_OUT PSParticleDraw(PARTICLE_PSIN input)
{
	PS_MRT_OUT output;

	float4 cColor = gtxtTexture.Sample(gSamplerState, input.tex);
	//if (cColor.a < 0.05) discard;
	output.vNormal  = float4(0, 0, 0, 0);
	output.vPos     = float4(input.posW, 1);
	output.vDiffuse = float4(0, 0, 0, 1);// cColor;
	output.vSpec    = float4(0, 0, 0, 1);
	output.vTxColor = (cColor * input.color);
	//cColor;//(gtxtTxColor.Load(int3(100, 100, 0)));
	return(output);
}