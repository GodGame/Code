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
	float  gMaxSize;
	float  gnColorInfo;
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
	float fagePercent : AGE;
	float4 color : COLOR;
	float3 velocity : VELOCITY;
	uint   type  : TYPE;
};

struct PARTICLE_PSIN
{
	float4 posH  : SV_POSITION;
	float3 posW  : POSITION;
//	float fOpacity : OPACITY;
	float fagePercent : AGE;
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
			float3 vRandom             = gtxtRandom.SampleLevel(gPTSampler, gfTime, 0).xyz;
			vRandom                    = normalize(vRandom);

			PARTICLE_INPUT particle;
			particle.pos               = gvParticleEmitPos.xyz;
			particle.size              = gMaxSize;//gMaxSize.x * abs(vRandom.z) + 2;//, abs(vRandom.y) * gMaxSize.y + 2;
			particle.velocity          = vRandom * gvParticleVelocity;
			particle.age               = 0.0f;
			particle.type              = PARTICLE_TYPE_FLARE;
			Pout.Append(particle);

			input[0].age = 0.0f;
		}
		Pout.Append(input[0]);
	}
	else if (input[0].age < gfLifeTime)
	{
		Pout.Append(input[0]);
	}
	
}

PARTICLE_GSIN VSParticleDraw(PARTICLE_INPUT input)
{
	PARTICLE_GSIN output;

	float t            = input.age;
	output.velocity    = (0.5f * gvAccel * t * t) + (input.velocity * t);
	output.pos         = output.velocity + input.pos;
	output.fagePercent = t / gfLifeTime;
	float fOpacity     = 1.0f - smoothstep(0.0f, 1.0f, output.fagePercent);

	output.color = gcColors[uint(gnColorInfo)] * fOpacity;

	output.size = input.size;
	output.type = input.type;

	return output;
}

[maxvertexcount(4)]
void GSParticleDraw(point PARTICLE_GSIN input[1], inout TriangleStream<PARTICLE_PSIN> triStream)
{
	if (input[0].type == PARTICLE_TYPE_EMITTER) return;
	//if (input[0].color.a < 0.02f) return;

	float3 vLook  = normalize(gf3CameraPos.xyz - input[0].pos);
	float3 vUp	  = normalize(input[0].velocity); //cross(vLook, vRight);
	float3 vRight = cross(vUp, vLook);			  //normalize(cross(float3(0.0f, 1.0f, 0.0f), vLook));

	float fHalfWidth = 0.5f * input[0].size.x, fHalfHeight = 0.5f * input[0].size.y;
	float4 vQuad[4];
	vQuad[0] = float4(input[0].pos + fHalfWidth * vRight - fHalfHeight * vUp, 1.0f);
	vQuad[1] = float4(input[0].pos + fHalfWidth * vRight + fHalfHeight * vUp, 1.0f);
	vQuad[2] = float4(input[0].pos - fHalfWidth * vRight - fHalfHeight * vUp, 1.0f);
	vQuad[3] = float4(input[0].pos - fHalfWidth * vRight + fHalfHeight * vUp, 1.0f);

	PARTICLE_PSIN output;
	[unroll]
	for (int i = 0; i < 4; ++i)
	{
		output.posH        = mul(vQuad[i], gmtxViewProjection);
		output.posW        = vQuad[i].xyz;
		output.tex         = gvQuadTexCoord[i];
		output.fagePercent = input[0].fagePercent;
		output.color       = input[0].color;
		triStream.Append(output);
	}
}

PS_MRT_OUT PSParticleDraw(PARTICLE_PSIN input)
{
	PS_MRT_OUT output;

	float3 uvw = float3(input.tex, uint(input.fagePercent + 0.85f));// 0);//(input.primID % 4));
	float4 cColor = gTextureArray.Sample(gSamplerState, uvw);
	//if (cColor.a < 0.02f) discard;

	output.vNormal  = float4(0, 0, 0, 0);
	output.vPos     = float4(1, 1, 1, 1);
	output.vDiffuse = float4(1, 1, 1, 0);
	output.vSpec    = float4(0, 0, 0, 1);
	output.vTxColor = (cColor * input.color);
	return(output);
}

///////////////////////////////////
[maxvertexcount(6)]
void GSRainSO(point PARTICLE_INPUT input[1], inout PointStream<PARTICLE_INPUT> Pout)
{
	input[0].age += gfTimeStep;

	if (input[0].type == PARTICLE_TYPE_EMITTER)
	{
		if (input[0].age > gfNewTime && gbEnable == 1)
		{
			[unroll]
			for (int i = 0; i < 5; ++i)
			{
				float3 vRandom    = gtxtRandom.SampleLevel(gPTSampler, gfTime + i, 0).xyz;
				vRandom           = abs(normalize(vRandom));

				PARTICLE_INPUT particle;
				particle.pos      = gvParticleEmitPos.xyz + (vRandom * 2048.0f);
				particle.pos.y    = gvParticleEmitPos.y;
				particle.size     = gMaxSize;
				particle.velocity = gvParticleVelocity;
				particle.age      = 0.0f;
				particle.type     = PARTICLE_TYPE_FLARE;
				Pout.Append(particle);
			}
			input[0].age = 0.0f;
		}
		Pout.Append(input[0]);
	}
	else if (input[0].age < gfLifeTime)
	{
		Pout.Append(input[0]);
	}
}

struct RAIN_GSIN
{
	float3 pos   : POSITION;
	float2 size  : SIZE;
	uint   type  : TYPE;
};

struct RAIN_PSIN
{
	float4 posH  : SV_POSITION;
	float3 posW  : POSITION;
//	float2 tex   : TEXCOORD;
};

RAIN_GSIN VSRainDraw(PARTICLE_INPUT input)
{
	RAIN_GSIN output;

	float t = input.age;
	output.pos = (0.5f * gvAccel * t * t) + (input.velocity * t) + input.pos;
	output.size = input.size;
	output.type = input.type;

	return output;
}

[maxvertexcount(2)]
void GSRainDraw(point RAIN_GSIN input[1], inout LineStream<RAIN_PSIN> lines)
{
	if (input[0].type == PARTICLE_TYPE_EMITTER) return;

	float4 vLine[2];
	vLine[0] = vLine[1] = float4(input[0].pos, 1.0f);
	vLine[0].y += input[0].size.y;

	RAIN_PSIN output;
	[unroll]
	for (int i = 0; i < 2; ++i)
	{
		output.posH = mul(vLine[i], gmtxViewProjection);
		output.posW = vLine[i].xyz;

		lines.Append(output);
	}
}

PS_MRT_OUT PSRainDraw(RAIN_PSIN input)
{
	PS_MRT_OUT output;

	output.vNormal  = float4(0, 0, 0, 0);
	output.vPos		= float4(input.posW, 1);// cColor;
	output.vDiffuse = float4(1, 1, 1, 0);
	output.vSpec    = float4(0, 0, 0, 1);
	output.vTxColor = float4(0.8, 0.8, 1, 1);

	return(output);
}
