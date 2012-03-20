#include <CL/cl.h>
#include <iostream>
#include <fstream>
#include <vector>

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

int program_main()
{
	cl_platform_id platform_id;
	cl_device_id device_id;

	if (!enumerateDevices(platform_id, device_id))
		return 1;

	cl_context context = createContext(platform_id, device_id);
	cl_command_queue cmd_queue = createCommandQueue(context, device_id);

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