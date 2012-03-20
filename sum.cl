kernel void sum(global const float* src_a, global const float* src_b, global float* dst)
{
	const size_t i = get_global_id(0);
	dst[i] = src_a[i] + src_b[i];
}
