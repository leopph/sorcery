#ifndef COMMON_HLSLI
#define COMMON_HLSLI

struct SorceryDrawCallParams {
  int base_vertex;
  uint base_instance;
};

#define DECLARE_PARAMS1(type, name) ConstantBuffer<type> name : register(b0, space0)
#define DECLARE_PARAMS(type) DECLARE_PARAMS1(type, g_params)
#define DECLARE_DRAW_CALL_PARAMS(name) ConstantBuffer<SorceryDrawCallParams> name : register(b1, space0);

#define GetResource(idx) ResourceDescriptorHeap[idx]

#pragma pack_matrix(row_major)

#endif
