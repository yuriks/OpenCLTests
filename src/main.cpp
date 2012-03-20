#include <CL/cl.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <memory>

template <typename T>
struct Array {
	Array(size_t sz) : data(new T[sz]), size(sz) { }
	~Array() { delete[] data; }
	operator T* () { return data; }
	operator const T* () const { return data; }
	void free()	{ delete[] data; data = nullptr; size = 0; }

	T* data;
	size_t size;
};


void checkOpenCLError(cl_int return_code, const char* file, int line)
{
	if (return_code == CL_SUCCESS)
		return;

	const char* enum_name;
	switch (return_code) {
#define ENUM_ENTRY(x) case x: enum_name = #x; break;
	ENUM_ENTRY(CL_SUCCESS                                  )
	ENUM_ENTRY(CL_DEVICE_NOT_FOUND                         )
	ENUM_ENTRY(CL_DEVICE_NOT_AVAILABLE                     )
	ENUM_ENTRY(CL_COMPILER_NOT_AVAILABLE                   )
	ENUM_ENTRY(CL_MEM_OBJECT_ALLOCATION_FAILURE            )
	ENUM_ENTRY(CL_OUT_OF_RESOURCES                         )
	ENUM_ENTRY(CL_OUT_OF_HOST_MEMORY                       )
	ENUM_ENTRY(CL_PROFILING_INFO_NOT_AVAILABLE             )
	ENUM_ENTRY(CL_MEM_COPY_OVERLAP                         )
	ENUM_ENTRY(CL_IMAGE_FORMAT_MISMATCH                    )
	ENUM_ENTRY(CL_IMAGE_FORMAT_NOT_SUPPORTED               )
	ENUM_ENTRY(CL_BUILD_PROGRAM_FAILURE                    )
	ENUM_ENTRY(CL_MAP_FAILURE                              )
	ENUM_ENTRY(CL_MISALIGNED_SUB_BUFFER_OFFSET             )
	ENUM_ENTRY(CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST)

	ENUM_ENTRY(CL_INVALID_VALUE                            )
	ENUM_ENTRY(CL_INVALID_DEVICE_TYPE                      )
	ENUM_ENTRY(CL_INVALID_PLATFORM                         )
	ENUM_ENTRY(CL_INVALID_DEVICE                           )
	ENUM_ENTRY(CL_INVALID_CONTEXT                          )
	ENUM_ENTRY(CL_INVALID_QUEUE_PROPERTIES                 )
	ENUM_ENTRY(CL_INVALID_COMMAND_QUEUE                    )
	ENUM_ENTRY(CL_INVALID_HOST_PTR                         )
	ENUM_ENTRY(CL_INVALID_MEM_OBJECT                       )
	ENUM_ENTRY(CL_INVALID_IMAGE_FORMAT_DESCRIPTOR          )
	ENUM_ENTRY(CL_INVALID_IMAGE_SIZE                       )
	ENUM_ENTRY(CL_INVALID_SAMPLER                          )
	ENUM_ENTRY(CL_INVALID_BINARY                           )
	ENUM_ENTRY(CL_INVALID_BUILD_OPTIONS                    )
	ENUM_ENTRY(CL_INVALID_PROGRAM                          )
	ENUM_ENTRY(CL_INVALID_PROGRAM_EXECUTABLE               )
	ENUM_ENTRY(CL_INVALID_KERNEL_NAME                      )
	ENUM_ENTRY(CL_INVALID_KERNEL_DEFINITION                )
	ENUM_ENTRY(CL_INVALID_KERNEL                           )
	ENUM_ENTRY(CL_INVALID_ARG_INDEX                        )
	ENUM_ENTRY(CL_INVALID_ARG_VALUE                        )
	ENUM_ENTRY(CL_INVALID_ARG_SIZE                         )
	ENUM_ENTRY(CL_INVALID_KERNEL_ARGS                      )
	ENUM_ENTRY(CL_INVALID_WORK_DIMENSION                   )
	ENUM_ENTRY(CL_INVALID_WORK_GROUP_SIZE                  )
	ENUM_ENTRY(CL_INVALID_WORK_ITEM_SIZE                   )
	ENUM_ENTRY(CL_INVALID_GLOBAL_OFFSET                    )
	ENUM_ENTRY(CL_INVALID_EVENT_WAIT_LIST                  )
	ENUM_ENTRY(CL_INVALID_EVENT                            )
	ENUM_ENTRY(CL_INVALID_OPERATION                        )
	ENUM_ENTRY(CL_INVALID_GL_OBJECT                        )
	ENUM_ENTRY(CL_INVALID_BUFFER_SIZE                      )
	ENUM_ENTRY(CL_INVALID_MIP_LEVEL                        )
	ENUM_ENTRY(CL_INVALID_GLOBAL_WORK_SIZE                 )
	ENUM_ENTRY(CL_INVALID_PROPERTY                         )
#undef ENUM_ENTRY
	default: enum_name = "Unknown error"; break;
	}

	std::cerr << file << ':' << line << ": OpenCL returned error " << enum_name << std::endl;
}
#define CHECK(x) checkOpenCLError(x, __FILE__, __LINE__)

bool enumerateDevices(cl_platform_id& platform_id, cl_device_id& device_id)
{
	bool found_device = false;

	cl_platform_id platform_ids[8];
	cl_uint num_platforms;

	CHECK(clGetPlatformIDs(8, platform_ids, &num_platforms));
	std::cout << "Found " << num_platforms << " platforms:\n";

	for (unsigned int i = 0; i < num_platforms; ++i) {
		char vendor_buffer[64];
		char name_buffer[64];
		char version_buffer[64];

		CHECK(clGetPlatformInfo(platform_ids[i], CL_PLATFORM_VENDOR,  sizeof(vendor_buffer),  static_cast<void*>(vendor_buffer),  nullptr));
		CHECK(clGetPlatformInfo(platform_ids[i], CL_PLATFORM_NAME,    sizeof(name_buffer),    static_cast<void*>(name_buffer),    nullptr));
		CHECK(clGetPlatformInfo(platform_ids[i], CL_PLATFORM_VERSION, sizeof(version_buffer), static_cast<void*>(version_buffer), nullptr));
		std::cout << "    " << vendor_buffer << " - " << name_buffer << " - " << version_buffer << '\n';

		cl_device_id device_ids[8];
		cl_uint num_devices;

		CHECK(clGetDeviceIDs(platform_ids[i], CL_DEVICE_TYPE_ALL, 8, device_ids, &num_devices));

		for (unsigned int j = 0; j < num_devices; ++j) {
			char vendor_buffer[64];
			char name_buffer[64];

			CHECK(clGetDeviceInfo(device_ids[j], CL_DEVICE_VENDOR, sizeof(vendor_buffer), static_cast<void*>(vendor_buffer), nullptr));
			CHECK(clGetDeviceInfo(device_ids[j], CL_DEVICE_NAME,   sizeof(name_buffer),   static_cast<void*>(name_buffer),   nullptr));
			std::cout << "        " << vendor_buffer << " - " << name_buffer << '\n';

			cl_device_type device_type;
			CHECK(clGetDeviceInfo(device_ids[j], CL_DEVICE_TYPE,   sizeof(device_type),   static_cast<void*>(&device_type),  nullptr));

			if (device_type == CL_DEVICE_TYPE_GPU) {
				platform_id = platform_ids[i];
				device_id = device_ids[j];
				found_device = true;
			}
		}
	}

	if (found_device) {
		char name_buffer[64];
		char version_buffer[64];

		CHECK(clGetDeviceInfo(device_id, CL_DEVICE_NAME,    sizeof(name_buffer),    static_cast<void*>(name_buffer),    nullptr));
		CHECK(clGetDeviceInfo(device_id, CL_DEVICE_VERSION, sizeof(version_buffer), static_cast<void*>(version_buffer), nullptr));
		std::cout << "Using device " << name_buffer << " - " << version_buffer << '\n';
	} else {
		std::cout << "No appropriate device found. :(\n";
	}

	return found_device;
}

cl_context createContext(cl_platform_id platform_id, cl_device_id device_id)
{
	cl_context_properties context_properties[] = {
		CL_CONTEXT_PLATFORM,
		(cl_context_properties)platform_id,
		0
	};

	std::cout << "Creating context...\n";

	cl_int error_code;
	cl_context context = clCreateContext(context_properties, 1, &device_id, nullptr, nullptr, &error_code);
	CHECK(error_code);

	return context;
}

cl_command_queue createCommandQueue(cl_context context, cl_device_id device_id)
{
	std::cout << "Creating command queue...\n";

	cl_int error_code;
	cl_command_queue cmd_queue = clCreateCommandQueue(context, device_id, 0, &error_code);
	CHECK(error_code);

	return cmd_queue;
}

cl_program loadProgramFromFile(cl_context context, const char* fname)
{
	std::ifstream f(fname);

	if (!f)
		return nullptr;

	std::vector<char> str;

	f.seekg(0, std::ios::end);
	str.reserve((unsigned int)f.tellg() + 1);
	f.seekg(0);

	str.assign(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
	str.push_back('\0');

	const char* str_ptr = str.data();

	cl_int error_code;
	cl_program program = clCreateProgramWithSource(context, 1, &str_ptr, nullptr, &error_code);
	CHECK(error_code);

	return program;
}

bool checkProgramLog(cl_program program, cl_device_id device_id)
{
	cl_build_status status;
	CHECK(clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_STATUS, sizeof(status), &status, nullptr));

	if (status == CL_BUILD_SUCCESS) {
		std::cout << "Program built successfully!\n";
		return true;
	} else if (status == CL_BUILD_ERROR) {
		std::cout << "Program failed to build:\n";

		size_t log_size;
		CHECK(clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, 0, nullptr, &log_size));

		std::vector<char> log_buffer(log_size);
		CHECK(clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, log_buffer.size(), log_buffer.data(), nullptr));

		std::cout << log_buffer.data();
	} else if (status == CL_BUILD_NONE) {
		std::cout << "No build done.\n";
	} else if (status == CL_BUILD_IN_PROGRESS) {
		std::cout << "Build is still in progress.\n";
	}

	return false;
}

void fillWithRandomData(Array<float>& v)
{
	for (size_t i = 0; i < v.size; ++i) {
		v[i] = rand() / (float)RAND_MAX * 10.0f;
	}
}

void sumKernelCpu(size_t n, const float* src_a, const float* src_b, float* dst)
{
	for (size_t i = 0; i < n; ++i) {
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
}

bool compareResults(const Array<float>& v_a, const Array<float>& v_b)
{
	static const float EPS = 0.00005f;

	if (v_a.size != v_b.size)
		return false;

	for (size_t i = 0; i < v_a.size; ++i) {
		if (std::abs(v_a[i] - v_b[i]) > EPS) {
			std::cout << i << '\n';
			return false;
		}
	}

	return true;
}

int program_main()
{
	cl_int error_code;

	cl_platform_id platform_id;
	cl_device_id device_id;

	if (!enumerateDevices(platform_id, device_id))
		return 1;

	cl_context context = createContext(platform_id, device_id);
	cl_command_queue cmd_queue = createCommandQueue(context, device_id);

	std::cout << "Loading program from sum.cl...\n";
	cl_program program = loadProgramFromFile(context, "sum.cl");

	std::cout << "Building program...\n";
	CHECK(clBuildProgram(program, 0, nullptr, "-Werror -cl-mad-enable -cl-fast-relaxed-math", nullptr, nullptr));
	if (!checkProgramLog(program, device_id)) {
		return 1;
	}

	std::cout << "Creating \"sum\" kernel...\n";
	cl_kernel sum_kernel = clCreateKernel(program, "sum", &error_code); CHECK(error_code);

	std::cout << "Allocating host buffers...\n";

	static const size_t WORK_DATA_SIZE = 16 * 1024 * 1024;
	Array<float> src_a(WORK_DATA_SIZE);
	Array<float> src_b(WORK_DATA_SIZE);
	Array<float> dst_opencl(WORK_DATA_SIZE);
	Array<float> dst_reference(WORK_DATA_SIZE);

	srand(0);
	fillWithRandomData(src_a);
	fillWithRandomData(src_b);

	std::cout << "Creating OpenCL buffers...\n";

	cl_mem src_a_buf = clCreateBuffer(context, CL_MEM_READ_ONLY,  src_a.size      * sizeof(float), nullptr, &error_code); CHECK(error_code);
	cl_mem src_b_buf = clCreateBuffer(context, CL_MEM_READ_ONLY,  src_b.size      * sizeof(float), nullptr, &error_code); CHECK(error_code);
	cl_mem dst_buf   = clCreateBuffer(context, CL_MEM_WRITE_ONLY, dst_opencl.size * sizeof(float), nullptr, &error_code); CHECK(error_code);

	std::cout << "Setting kernel arguments...\n";
	CHECK(clSetKernelArg(sum_kernel, 0, sizeof(src_a_buf), &src_a_buf));
	CHECK(clSetKernelArg(sum_kernel, 1, sizeof(src_b_buf), &src_b_buf));
	CHECK(clSetKernelArg(sum_kernel, 2, sizeof(dst_buf),   &dst_buf));

	std::cout << "\nRunning CPU algorithm...\n";
	sumKernelCpu(dst_reference.size, src_a, src_b, dst_reference);
	std::cout << "Done!\n\n";

	std::cout << "Copying from host to OpenCL buffers...\n";
	CHECK(clFinish(cmd_queue));
	CHECK(clEnqueueWriteBuffer(cmd_queue, src_a_buf, CL_FALSE, 0, src_a.size * sizeof(float), src_a, 0, nullptr, nullptr));
	CHECK(clEnqueueWriteBuffer(cmd_queue, src_b_buf, CL_FALSE, 0, src_b.size * sizeof(float), src_b, 0, nullptr, nullptr));
	CHECK(clFinish(cmd_queue));

	std::cout << "Executing OpenCL kernel...\n";
	const cl_uint work_dimensions[1] = { WORK_DATA_SIZE };
	CHECK(clEnqueueNDRangeKernel(cmd_queue, sum_kernel, 1, nullptr, work_dimensions, nullptr, 0, nullptr, nullptr));
	CHECK(clFinish(cmd_queue));

	std::cout << "Reading computation results back to host...\n";
	CHECK(clEnqueueReadBuffer(cmd_queue, dst_buf, CL_FALSE, 0, dst_opencl.size * sizeof(float), dst_opencl, 0, nullptr, nullptr));
	CHECK(clFinish(cmd_queue));
	std::cout << "Done!\n\n";

	std::cout << "Comparing results...\n";
	if (compareResults(dst_opencl, dst_reference)) {
		std::cout << "Results match!\n\n";
	} else {
		std::cout << "Results don't match! :(\n\n";
	}

	std::cout << "Releasing OpenCL buffers...\n";
	CHECK(clReleaseMemObject(src_a_buf));
	CHECK(clReleaseMemObject(src_b_buf));
	CHECK(clReleaseMemObject(dst_buf));

	std::cout << "Freeing host buffers...\n";
	src_a.free();
	src_b.free();
	dst_opencl.free();
	dst_reference.free();

	std::cout << "Releasing kernel...\n";
	CHECK(clReleaseKernel(sum_kernel));

	std::cout << "Releasing program...\n";
	CHECK(clReleaseProgram(program));

	std::cout << "Releasing command queue...\n";
	CHECK(clReleaseCommandQueue(cmd_queue));

	std::cout << "Releasing context...\n";
	CHECK(clReleaseContext(context));

	return 0;
}

int main() {
	int ret = program_main();

	std::cout << "Done.\n";
	std::cin.get();

	return ret;
}