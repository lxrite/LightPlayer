struct vertex_input{
	float4 pos : POSITION;
	float2 uv : TEXCOORD0;
};
cbuffer PER_WINDOW_SIZE {
	float2 posScale;
}


struct vertex_output{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
};

vertex_output main(vertex_input i ) 
{
	vertex_output o;
	o.pos = float4(i.pos.xy*posScale, i.pos.zw);
	o.uv = i.uv;
	return o;
}