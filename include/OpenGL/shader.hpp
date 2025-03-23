#pragma once

#include <config.hpp>

#include <paths.hpp>

#include <FormatConsole.hpp>

class Shader {
	public:
		mutable GLuint ID;

		Shader(const filesystem::path& vertexFilepath, const filesystem::path& fragmentFilepath) {
			ID = makeShader(vertexFilepath, fragmentFilepath);
		}

		void activate() {
			glUseProgram(ID);
		}

		void destroy() {
			glDeleteProgram(ID);
		}

	private:
		GLuint makeModule(const filesystem::path& filepath, GLuint module_type) {
			std::ifstream file;
			stringstream bufferedLines;
		
			if (showShaderMessages) { cout << formatProcess("Compiling") << " module '" << formatPath(getFileName(filepath.string())) << "' ... "; }
		
			file.open(filepath);
			if (file.is_open()) {
				bufferedLines << file.rdbuf();
			}
			else {
				if (showShaderMessages) { std::cout << formatError("FAILED") << "\n"; }
				cerr << "unable to open " << formatPath(filepath.string()) << "\n" << endl;
				return 0;
			}
		
			std::string shaderSource = bufferedLines.str();
			const char* shaderSrc = shaderSource.c_str();
			bufferedLines.str("");
			file.close();
		
			GLuint shaderModule = glCreateShader(module_type);
			glShaderSource(shaderModule, 1, &shaderSrc, NULL);
			glCompileShader(shaderModule);
		
			int success;
			glGetShaderiv(shaderModule, GL_COMPILE_STATUS, &success);
			if (!success) {
				char errorLog[1024];
				glGetShaderInfoLog(shaderModule, 1024, NULL, errorLog);
				if (showShaderMessages) { std::cout << formatError("FAILED") << "\n"; }
				cerr << "Shader Module compilation error:\n" << colorText(errorLog, ANSII_YELLOW) << endl;
			}
			else if (showShaderMessages && success) { std::cout << formatSuccess("Done") << endl; }
		
			return shaderModule;
		}
		
		GLuint makeShader(const filesystem::path& vertexFilepath, const filesystem::path& fragmentFilepath) {
			//To store all the shader modules
			std::vector<GLuint> modules;
		
			// add modules to be attached to the shader
			modules.push_back(makeModule(vertexFilepath,  GL_VERTEX_SHADER));
			modules.push_back(makeModule(fragmentFilepath, GL_FRAGMENT_SHADER));
		
			if (showShaderMessages) { cout << formatProcess("Making shader") << " from '" << formatPath(getFileName(vertexFilepath.string())) << "' and '" << formatPath(getFileName(fragmentFilepath.string())) << "' ... "; }
		
			//Attach all the modules then link the program
			GLuint shader = glCreateProgram();
			for (GLuint shaderModule : modules) {
				glAttachShader(shader, shaderModule);
			}
			glLinkProgram(shader);
		
			//Check the linking worked
			int success;
			glGetProgramiv(shader, GL_LINK_STATUS, &success);
			if (!success) {
				char errorLog[1024];
				glGetProgramInfoLog(shader, 1024, NULL, errorLog);
				if (showShaderMessages) { std::cout << formatError("FAILED") << "\n"; }
				cerr << "Shader linking error:\n" << colorText(errorLog, ANSII_YELLOW) << '\n';
			}
			else if (showShaderMessages && success) { std::cout << formatSuccess("Done") << endl; }
		
			//Modules are now unneeded and can be freed
			for (GLuint shaderModule : modules) {
				glDeleteShader(shaderModule);
			}
		
			return shader;
		}
};

