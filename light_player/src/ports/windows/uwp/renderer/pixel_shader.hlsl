
Texture2D<float> ytex;
Texture2D<float> utex;
Texture2D<float> vtex;
SamplerState textureSampler
{
	AddressU = Clamp;
	AddressV = Clamp;
	Filter = MIN_MAG_LINEAR;
};


struct vertex_output {
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
};

static const float3 R_cf = float3(1.164383, 0.000000, 1.596027);
static const float3 G_cf = float3(1.164383, -0.391762, -0.812968);
static const float3 B_cf = float3(1.164383, 2.017232, 0.000000);
static const float3 offset = float3(-0.0625, -0.5, -0.5);


float4 main(vertex_output i) : SV_TARGET
{
	float y = ytex.Sample(textureSampler, i.uv);
	float u = utex.Sample(textureSampler, i.uv);
	float v = vtex.Sample(textureSampler, i.uv);
	float3 yuv = float3(y, u, v) + offset;
	return float4(dot(yuv, R_cf), dot(yuv, G_cf), dot(yuv, B_cf), 1.0);

}