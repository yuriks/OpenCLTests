kernel void sum(global const float* src_a, global const float* src_b, global float* dst)
{
	const size_t i = get_global_id(0);
	const size_t n = get_global_size(0);

	float sum = 0.0f;

	for (int j = -20; j <= 20; ++j) {
		int index = i+j;
		if (index < 0)
			index = 0;
		else if ((size_t)index >= n)
			index = n-1;

		float a = src_a[index];
		float b = src_b[index];
		sum += a*a + b*b;
	}

	dst[i] = sum / 41.0f;
}
