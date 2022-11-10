__global__ void sineWaveKernel(float4 *pos, unsigned int width, unsigned int height, float animtime)
{
	unsigned int x = blockIdx.x*blockDim.x + threadIdx.x;
	unsigned int y = blockIdx.y*blockDim.y + threadIdx.y;
	float u = x / (float)width;
	float v = y / (float)height;
	u = u*2.0 - 1.0;
	v = v*2.0 - 1.0;

	float frequency = 4.0;
	float w = sinf(frequency*u + animtime)*cosf(frequency*v + animtime)*0.5;
	pos[y*width + x] = make_float4(u, w, v, 1.0);
}

void LaunchCudaKernal(float4 *pos, unsigned int meshWidth, unsigned int meshHeight, float time)
{
	dim3 block(8, 8, 1);
	dim3 grid(meshWidth / block.x, meshHeight / block.y, 1.0);
	sineWaveKernel <<<grid,block>>> (pos, meshWidth, meshHeight, time);
}
