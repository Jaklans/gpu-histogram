// Boilerplate is adapted from Victor Gordan's OpenGL Tutorials source code at https://github.com/VictorGordan/opengl-tutorials
// Started from a compute shader example, and removed everything but the initialization

#include<iostream>

#include"glad/glad.h"
#include"GLFW/glfw3.h"

#include "histogram.comp"

#include <vector>

const unsigned int SCREEN_WIDTH = 32;
const unsigned int SCREEN_HEIGHT = 32;

const unsigned short OPENGL_MAJOR_VERSION = 4;
const unsigned short OPENGL_MINOR_VERSION = 6;

bool vSync = true;


// TODO: finish cleanup on early fails


void LogComputeCapability() 
{
	int work_grp_cnt[3];
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &work_grp_cnt[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &work_grp_cnt[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &work_grp_cnt[2]);
	std::cout << "Max work groups per compute shader" <<
		" x:" << work_grp_cnt[0] <<
		" y:" << work_grp_cnt[1] <<
		" z:" << work_grp_cnt[2] << "\n";

	int work_grp_size[3];
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &work_grp_size[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &work_grp_size[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &work_grp_size[2]);
	std::cout << "Max work group sizes" <<
		" x:" << work_grp_size[0] <<
		" y:" << work_grp_size[1] <<
		" z:" << work_grp_size[2] << "\n";

	int work_grp_inv;
	glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &work_grp_inv);
	std::cout << "Max invocations count per work group: " << work_grp_inv << "\n";
}

// Borrowed from https://learnopengl.com/In-Practice/Debugging
void APIENTRY glDebugOutput(GLenum source,
	GLenum type,
	unsigned int id,
	GLenum severity,
	GLsizei length,
	const char* message,
	const void* userParam)
{
	// ignore non-significant error/warning codes
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

	std::cout << "---------------" << std::endl;
	std::cout << "Debug message (" << id << "): " << message << std::endl;

	switch (source)
	{
	case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
	case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
	case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
	} std::cout << std::endl;

	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
	case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
	case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
	case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
	case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
	case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
	} std::cout << std::endl;

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
	case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
	case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
	} std::cout << std::endl;
	std::cout << std::endl;
}



int main()
{
	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, OPENGL_MAJOR_VERSION);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, OPENGL_MINOR_VERSION);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true); 
	

	GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "OpenGL Compute Shaders", NULL, NULL);
	if (!window)
	{
		std::cout << "Failed to create the GLFW window\n";
		glfwTerminate();
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(vSync);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize OpenGL context" << std::endl;
	}

	int flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
	{
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(glDebugOutput, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	}
	else 
	{
		std::cout << "unable to set debug callback";
	}

	std::cout << "OpenCL version " << glGetString(GL_VERSION) << std::endl;
	LogComputeCapability();

	GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);
	glShaderSource(computeShader, 1, &HistogramShaderSource, NULL);
	glCompileShader(computeShader);

	GLint success = 0;
	glGetShaderiv(computeShader, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		std::cout << "Failed to compile shader";

		GLint maxLength = 0;
		glGetShaderiv(computeShader, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		std::vector<GLchar> errorLog(maxLength);
		glGetShaderInfoLog(computeShader, maxLength, &maxLength, &errorLog[0]);

		std::cout << errorLog.data() << std::endl;

		exit(1);
	}

	GLuint computeProgram = glCreateProgram();
	glAttachShader(computeProgram, computeShader);
	glLinkProgram(computeProgram);

	// TODO, assert this is less than max
	const unsigned int DISPATCH_COUNT = 32;
	const unsigned int INPUT_COUNT = 1 << 16;
	const unsigned int BIN_SIZE = 32;

	unsigned char* inputArray = new unsigned char[INPUT_COUNT];
	unsigned int inputSize = INPUT_COUNT * sizeof(unsigned char);
	const unsigned int inputBinding = 0;
	GLuint inputBuffer = 0;

	unsigned int* outputArray = new unsigned int[INPUT_COUNT];
	unsigned int outputCount = ceil(256.0 / BIN_SIZE);
	unsigned int outputSize = outputCount * sizeof(unsigned int);
	const unsigned int outputBinding = 1;
	GLuint outputBuffer = 0;
	
	for (int i = 0; i < INPUT_COUNT; i++) {
		inputArray[i] = (char) i;
		//inputArray[i] |= 6;
		//inputArray[i] |= (3 << 8);
		//if (i % 2) {
		//	inputArray[i] |= 17 << 8;
		//}
	}


	glGenBuffers(1, &inputBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, inputBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, inputSize, inputArray, GL_STREAM_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, inputBinding, inputBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	glGenBuffers(1, &outputBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, outputBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, outputSize, nullptr, GL_STREAM_READ);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, outputBinding, outputBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);


	glUseProgram(computeProgram);

	// Send over input count and bin size
	glUniform1ui(0, INPUT_COUNT);
	glUniform1ui(1, BIN_SIZE);


	glDispatchCompute(DISPATCH_COUNT, 1, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	glFinish();

	glfwPollEvents();

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 1);
 	glGetNamedBufferSubData(outputBuffer, 0, outputSize, outputArray);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	//for (int i = 0; i < outputCount; i++) {
	//	outputArray[i] = 0;
	//}
	//for (int i = 0; i < INPUT_COUNT; i++) {
	//	outputArray[inputArray[i]]++;
	//}


	for (int i = 0; i < outputCount; i++) {
		std::cout << i << ": " << outputArray[i] << std::endl;
	}

	delete[] inputArray;
	delete[] outputArray;

	glfwDestroyWindow(window);
	glfwTerminate();
}