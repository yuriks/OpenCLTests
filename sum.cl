kernel void sum(global const float* src_a, global const float* src_b, global float* dst)
{
	const size_t i = get_global_id(0);

	float a = src_a[i];
	float b = src_b[i];

	dst[i] = a*a + b*b;
}

// Reduce-sums values in src, outputting partial results to partial_sums.
kernel void reduce_pass1(
	global const float src         [/* n */],
	global       float partial_sums[/* global_size / local_size */],
	local        float scratch     [/* local_size */],
	const int n)
{
	float accum = 0.f;
	for (int i = get_global_id(0); i < n; i += get_global_size(0)) {
		accum += src[i];
	}

	int lid = get_local_id(0);
	scratch[lid] = accum;
	barrier(CLK_LOCAL_MEM_FENCE);

	for (int offset = get_local_size(0) / 2; offset > 0; offset /= 2) {
		if (lid < offset) {
			scratch[lid] += scratch[lid+offset];
		}
		barrier(CLK_LOCAL_MEM_FENCE);
	}

	if (lid == 0) {
		partial_sums[get_group_id(0)] = scratch[0];
	}
}

// Reduce-sums values in partial_sums, outputting final sum to final_sum
kernel void reduce_pass2(
	global const float  partial_sums[/* n */],
	global       float* final_sum,
	const int n)
{
	float accum = 0.f;
	for (int i = 0; i < n; ++i) {
		accum += partial_sums[i];
	}
	*final_sum = accum;
}
