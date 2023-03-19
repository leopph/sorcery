float4 main(const uint vertexId : SV_VertexID ) : SV_POSITION
{
	switch(vertexId) {
	case 5:
	case 0:
            return float4(-1, -1, 0, 1);
	case 1:
            return float4(-1, 1, 0, 1);
	case 2:
	case 3:
            return float4(1, 1, 0, 1);
	case 4:
            return float4(1, -1, 0, 1);
	default:
            return float4(0, 0, 0, 0);
    }
}