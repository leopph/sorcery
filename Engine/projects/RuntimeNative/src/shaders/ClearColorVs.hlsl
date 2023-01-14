float4 main(float2 pos : POSITION) : SV_Position {
    return float4(pos, 0, 1);
}