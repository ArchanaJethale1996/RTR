__kernel void sineWaveKernel(__global float4 *pos, unsigned int width, unsigned int height, float animtime)
{
	unsigned int x = get_global_id(0);
	unsigned int y = get_global_id(1);
	float u = x / (float)width;
	float v = y / (float)height;
	u = u*2.0 - 1.0;
	v = v*2.0 - 1.0;

	float frequency = 4.0;
	float w = sinf(frequency*u + animtime)*cosf(frequency*v + animtime)*0.5;
	pos[y*width + x] = (float4)(u, w, v, 1.0);
}
