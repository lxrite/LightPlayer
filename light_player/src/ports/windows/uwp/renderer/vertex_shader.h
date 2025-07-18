#if 0
//
// Generated by Microsoft (R) HLSL Shader Compiler 6.3.9600.16384
//
//
// Buffer Definitions: 
//
// cbuffer PER_WINDOW_SIZE
// {
//
//   float2 posScale;                   // Offset:    0 Size:     8
//
// }
//
//
// Resource Bindings:
//
// Name                                 Type  Format         Dim Slot Elements
// ------------------------------ ---------- ------- ----------- ---- --------
// PER_WINDOW_SIZE                   cbuffer      NA          NA    0        1
//
//
//
// Input signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// POSITION                 0   xyzw        0     NONE   float   xyzw
// TEXCOORD                 0   xy          1     NONE   float   xy  
//
//
// Output signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// SV_POSITION              0   xyzw        0      POS   float   xyzw
// TEXCOORD                 0   xy          1     NONE   float   xy  
//
//
// Constant buffer to DX9 shader constant mappings:
//
// Target Reg Buffer  Start Reg # of Regs        Data Conversion
// ---------- ------- --------- --------- ----------------------
// c1         cb0             0         1  ( FLT, FLT, FLT, FLT)
//
//
// Runtime generated constant mappings:
//
// Target Reg                               Constant Description
// ---------- --------------------------------------------------
// c0                              Vertex Shader position offset
//
//
// Level9 shader bytecode:
//
vs_2_0
dcl_texcoord v0
dcl_texcoord1 v1
mul r0.xy, v0.w, c0
mad oPos.xy, v0, c1, r0
mov oPos.zw, v0
mov oT0.xy, v1

// approximately 4 instruction slots used
vs_4_0
dcl_constantbuffer cb0[1], immediateIndexed
dcl_input v0.xyzw
dcl_input v1.xy
dcl_output_siv o0.xyzw, position
dcl_output o1.xy
mul o0.xy, v0.xyxx, cb0[0].xyxx
mov o0.zw, v0.zzzw
mov o1.xy, v1.xyxx
ret
// Approximately 4 instruction slots used
#endif

const BYTE YUV_RENDER_VERTEX_SHADER[] =
{
     68,  88,  66,  67, 255, 191,
     47, 197,  58, 147, 181,  29,
     82, 155, 203, 126, 229,  97,
    210,  49,   1,   0,   0,   0,
    108,   3,   0,   0,   6,   0,
      0,   0,  56,   0,   0,   0,
    208,   0,   0,   0, 112,   1,
      0,   0, 236,   1,   0,   0,
    192,   2,   0,   0,  20,   3,
      0,   0,  65, 111, 110,  57,
    144,   0,   0,   0, 144,   0,
      0,   0,   0,   2, 254, 255,
     92,   0,   0,   0,  52,   0,
      0,   0,   1,   0,  36,   0,
      0,   0,  48,   0,   0,   0,
     48,   0,   0,   0,  36,   0,
      1,   0,  48,   0,   0,   0,
      0,   0,   1,   0,   1,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   0,   2, 254, 255,
     31,   0,   0,   2,   5,   0,
      0, 128,   0,   0,  15, 144,
     31,   0,   0,   2,   5,   0,
      1, 128,   1,   0,  15, 144,
      5,   0,   0,   3,   0,   0,
      3, 128,   0,   0, 255, 144,
      0,   0, 228, 160,   4,   0,
      0,   4,   0,   0,   3, 192,
      0,   0, 228, 144,   1,   0,
    228, 160,   0,   0, 228, 128,
      1,   0,   0,   2,   0,   0,
     12, 192,   0,   0, 228, 144,
      1,   0,   0,   2,   0,   0,
      3, 224,   1,   0, 228, 144,
    255, 255,   0,   0,  83,  72,
     68,  82, 152,   0,   0,   0,
     64,   0,   1,   0,  38,   0,
      0,   0,  89,   0,   0,   4,
     70, 142,  32,   0,   0,   0,
      0,   0,   1,   0,   0,   0,
     95,   0,   0,   3, 242,  16,
     16,   0,   0,   0,   0,   0,
     95,   0,   0,   3,  50,  16,
     16,   0,   1,   0,   0,   0,
    103,   0,   0,   4, 242,  32,
     16,   0,   0,   0,   0,   0,
      1,   0,   0,   0, 101,   0,
      0,   3,  50,  32,  16,   0,
      1,   0,   0,   0,  56,   0,
      0,   8,  50,  32,  16,   0,
      0,   0,   0,   0,  70,  16,
     16,   0,   0,   0,   0,   0,
     70, 128,  32,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
     54,   0,   0,   5, 194,  32,
     16,   0,   0,   0,   0,   0,
    166,  30,  16,   0,   0,   0,
      0,   0,  54,   0,   0,   5,
     50,  32,  16,   0,   1,   0,
      0,   0,  70,  16,  16,   0,
      1,   0,   0,   0,  62,   0,
      0,   1,  83,  84,  65,  84,
    116,   0,   0,   0,   4,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   4,   0,
      0,   0,   1,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   1,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   2,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
     82,  68,  69,  70, 204,   0,
      0,   0,   1,   0,   0,   0,
     76,   0,   0,   0,   1,   0,
      0,   0,  28,   0,   0,   0,
      0,   4, 254, 255,   0,   1,
      0,   0, 152,   0,   0,   0,
     60,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      1,   0,   0,   0,   0,   0,
      0,   0,  80,  69,  82,  95,
     87,  73,  78,  68,  79,  87,
     95,  83,  73,  90,  69,   0,
     60,   0,   0,   0,   1,   0,
      0,   0, 100,   0,   0,   0,
     16,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
    124,   0,   0,   0,   0,   0,
      0,   0,   8,   0,   0,   0,
      2,   0,   0,   0, 136,   0,
      0,   0,   0,   0,   0,   0,
    112, 111, 115,  83,  99,  97,
    108, 101,   0, 171, 171, 171,
      1,   0,   3,   0,   1,   0,
      2,   0,   0,   0,   0,   0,
      0,   0,   0,   0,  77, 105,
     99, 114, 111, 115, 111, 102,
    116,  32,  40,  82,  41,  32,
     72,  76,  83,  76,  32,  83,
    104,  97, 100, 101, 114,  32,
     67, 111, 109, 112, 105, 108,
    101, 114,  32,  54,  46,  51,
     46,  57,  54,  48,  48,  46,
     49,  54,  51,  56,  52,   0,
    171, 171,  73,  83,  71,  78,
     76,   0,   0,   0,   2,   0,
      0,   0,   8,   0,   0,   0,
     56,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      3,   0,   0,   0,   0,   0,
      0,   0,  15,  15,   0,   0,
     65,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      3,   0,   0,   0,   1,   0,
      0,   0,   3,   3,   0,   0,
     80,  79,  83,  73,  84,  73,
     79,  78,   0,  84,  69,  88,
     67,  79,  79,  82,  68,   0,
    171, 171,  79,  83,  71,  78,
     80,   0,   0,   0,   2,   0,
      0,   0,   8,   0,   0,   0,
     56,   0,   0,   0,   0,   0,
      0,   0,   1,   0,   0,   0,
      3,   0,   0,   0,   0,   0,
      0,   0,  15,   0,   0,   0,
     68,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      3,   0,   0,   0,   1,   0,
      0,   0,   3,  12,   0,   0,
     83,  86,  95,  80,  79,  83,
     73,  84,  73,  79,  78,   0,
     84,  69,  88,  67,  79,  79,
     82,  68,   0, 171, 171, 171
};