#ifndef COMMON_HLSLI
#define COMMON_HLSLI

#define DECLARE_PARAMS1(type, name) ConstantBuffer<type> name : register(b0, space0)
#define DECLARE_PARAMS(type) DECLARE_PARAMS1(type, g_params)

#endif
