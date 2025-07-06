#ifndef SQUENCES_HLSLI
#define SQUENCES_HLSLI


float RadicalInverse_VdC(uint bits) {
  bits = (bits << uint(16U)) | (bits >> uint(16U));
  bits = ((bits & uint(0x55555555U)) << uint(1U)) | ((bits & uint(0xAAAAAAAAU)) >> uint(1U));
  bits = ((bits & uint(0x33333333U)) << uint(2U)) | ((bits & uint(0xCCCCCCCCU)) >> uint(2U));
  bits = ((bits & uint(0x0F0F0F0FU)) << uint(4U)) | ((bits & uint(0xF0F0F0F0U)) >> uint(4U));
  bits = ((bits & uint(0x00FF00FFU)) << uint(8U)) | ((bits & uint(0xFF00FF00U)) >> uint(8U));
  return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}


float2 Hammersley(uint const i, uint const N) {
  return float2(float(i) / float(N), RadicalInverse_VdC(i));
}

#endif
