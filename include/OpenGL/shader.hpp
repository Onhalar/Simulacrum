#ifndef SHADER_CLASS_HEADER
#define SHADER_CLASS_HEADER

// only for central assignment of the 'debugMode' variable
#include <config.hpp>
// uncomment the below and remove the include above
// bool debugMode = true;

#include <glm/glm.hpp>

#include <filesystem>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <unordered_map>

// custom libraries
#include <FormatConsole.hpp>
#include <paths.hpp>

#include <json.hpp> // external library - likely to cause an error if used separately

float allowShaderCaching = true;

class ShaderCache {
	private:
		static filesystem::path shaderManagerFile;
		static filesystem::path shaderCachePath;
		static bool cachingEnabled;

	public:
		// Exceptions
		class setupFailedException : std::runtime_error { public: explicit setupFailedException (const char* message) : std::runtime_error(message) {} };

		class RetrievalException : std::runtime_error { public: explicit RetrievalException (const char* message) : std::runtime_error(message) {} };
		class WriteException : std::runtime_error { public: explicit WriteException (const char* message) : std::runtime_error(message) {} };

		class EncodingException : std::runtime_error { public: explicit EncodingException (const char* message) : std::runtime_error(message) {} };
		class DecodingException : std::runtime_error { public: explicit DecodingException (const char* message) : std::runtime_error(message) {} };

		static void enable(filesystem::path path, filesystem::path managerFile = "shader_cache", bool allowCaching = true /* just here in case it needs an external variable connection */) {
			if (allowCaching) { cachingEnabled = true; }
			else {
				cachingEnabled = false;
				return;
			}
			
			shaderManagerFile = managerFile;
			shaderCachePath = path;
		}

	private:
		static void ensureCacheExists() {
			if (!filesystem::exists(shaderCachePath)) {
				try {
					filesystem::create_directory(shaderCachePath);
				}
				catch (const exception& e) {
					string error = "Could not create a cache directory: " + (string)e.what();
					throw setupFailedException(error.c_str());
				}
			}

			if (!filesystem::exists(shaderCachePath / shaderManagerFile)) {
				try {
					ofstream(shaderCachePath / shaderManagerFile); // automatically closed and destroyed once out of scope.
				}
				catch (const exception& e) {
					string error = "Could not create a cache manager file: " + (string)e.what();
					throw setupFailedException(error.c_str());
				}
			}
		}

};

class Shader {
	public:

		mutable GLuint ID;

		mutable std::unordered_map<const char*, GLuint> otherUniforms;

		mutable int modelMatrixUniform, projectionMatrixUniform, viewMatrixUniform;

		mutable bool hasModelMatrixUniform, hasProjectionMatrixUniform, hasViewMatrixUniform;

		mutable glm::mat4 modelMatrix = glm::mat4(1.0f);
		mutable glm::mat4 viewMatrix = glm::mat4(1.0f);
		mutable glm::mat4 projectionMatrix = glm::mat4(1.0f);

		Shader(const filesystem::path& vertexFilepath, const filesystem::path& fragmentFilepath, bool throwErrors = false, const char* modelMatrixUniformName = "model", const char* viewMatrixUniformName = "view", const char* projectionMatrixUniformName = "projection") {
			ID = makeShader(vertexFilepath, fragmentFilepath);

			hasModelMatrixUniform = hasProjectionMatrixUniform = hasViewMatrixUniform = false;

			activate();

			modelMatrixUniform = glGetUniformLocation(ID, modelMatrixUniformName);
			if (modelMatrixUniform != -1) { hasModelMatrixUniform = true; }

			viewMatrixUniform = glGetUniformLocation(ID, viewMatrixUniformName);
			if (viewMatrixUniform != -1) { hasViewMatrixUniform = true; }

			projectionMatrixUniform = glGetUniformLocation(ID, projectionMatrixUniformName);
			if (projectionMatrixUniform != -1) { hasProjectionMatrixUniform = true; }
		}

		GLuint getUniformID(const char* name, bool structMode) {
			if (otherUniforms.find(name) == otherUniforms.end() || structMode) {
				otherUniforms[name] = glGetUniformLocation(ID, name);
			}
			return otherUniforms[name];
		}

		// tries to find it if it's already set, elsewere looks for it in the shader if not found returns; shader needs to be activated first
		bool setUniform(const char* name, glm::mat4 data, bool structMode = false) {
			GLuint uniformID = getUniformID(name, structMode);
			if (uniformID == -1) {
                return false;
            }
			glUniformMatrix4fv(otherUniforms[name], 1, GL_FALSE, glm::value_ptr(data));
            return true;
		}
		bool setUniform(const char* name, glm::vec3 data, bool structMode = false) {
			GLuint uniformID = getUniformID(name, structMode);
			if (uniformID == -1) {
                return false;
            }
			glUniform3fv(otherUniforms[name], 1, glm::value_ptr(data));
            return true;
		}
		bool setUniform(const char* name, glm::vec4 data, bool structMode = false) {
			GLuint uniformID = getUniformID(name, structMode);
			if (uniformID == -1) {
                return false;
            }
			glUniform4fv(otherUniforms[name], 1, glm::value_ptr(data));
            return true;
		}
		bool setUniform(const char* name, GLfloat data, bool structMode = false) {
			GLuint uniformID = getUniformID(name, structMode);
			if (uniformID == -1) {
                return false;
            }
			glUniform1f(otherUniforms[name], data);
            return true;
		}
		bool setUniform(const char* name, GLint data, bool structMode = false) {
			GLuint uniformID = getUniformID(name, structMode);
			if (uniformID == -1) {
                return false;
            }
			glUniform1i(otherUniforms[name], data);
            return true;
		}


		// applies the main model matrix
		void applyModelMatrix() {
			if (hasModelMatrixUniform) {
				glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, glm::value_ptr(modelMatrix));
			}
		}
		// applies custom model matrix
		void applyModelMatrix(const glm::mat4& modelMatrixARG) {
			if (hasModelMatrixUniform) {
				glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, glm::value_ptr(modelMatrixARG));
			}
		}
		
		// applies the main view matrix
		void applyViewMatrix() {
			if (hasViewMatrixUniform) {
            	glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, glm::value_ptr(viewMatrix));
			}
        }
		// applies custom view matrix
        void applyViewMatrix(const glm::mat4& viewMatrixARG) {
			if (hasViewMatrixUniform) {
            	glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, glm::value_ptr(viewMatrixARG));
			}
        }
		
		// applies the main projection matrix
        void applyProjectionMatrix() {
			if (hasProjectionMatrixUniform) {
            	glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
			}
        }
		// applies custom projection matrix
		void applyProjectionMatrix(const glm::mat4& projectionMatrixARG) {
			if (hasProjectionMatrixUniform) {
            	glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, glm::value_ptr(projectionMatrixARG));
			}
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
		
			if (debugMode) { cout << formatProcess("Compiling") << " module '" << formatPath(getFileName(filepath.string())) << "' ... "; }
		
			file.open(filepath);
			if (file.is_open()) {
				bufferedLines << file.rdbuf();
			}
			else {
				if (debugMode) { std::cout << formatError("FAILED") << "\n"; }
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
				if (debugMode) { std::cout << formatError("FAILED") << "\n"; }
				cerr << "Shader Module compilation error:\n" << colorText(errorLog, ANSII_YELLOW) << endl;
			}
			else if (debugMode && success) { std::cout << formatSuccess("Done") << endl; }
		
			return shaderModule;
		}
		
		GLuint makeShader(const filesystem::path& vertexFilepath, const filesystem::path& fragmentFilepath) {
			//To store all the shader modules
			std::vector<GLuint> modules;
		
			// add modules to be attached to the shader
			modules.push_back(makeModule(vertexFilepath,  GL_VERTEX_SHADER));
			modules.push_back(makeModule(fragmentFilepath, GL_FRAGMENT_SHADER));
		
			if (debugMode) { cout << formatProcess("Making shader") << " from '" << formatPath(getFileName(vertexFilepath.string())) << "' and '" << formatPath(getFileName(fragmentFilepath.string())) << "' ... "; }
		
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
				if (debugMode) { std::cout << formatError("FAILED") << "\n"; }
				cerr << "Shader linking error:\n" << colorText(errorLog, ANSII_YELLOW) << '\n';
			}
			else if (debugMode && success) { std::cout << formatSuccess("Done") << endl; }
		
			//Modules are now unneeded and can be freed
			for (GLuint shaderModule : modules) {
				glDeleteShader(shaderModule);
			}
		
			return shader;
		}
};

#endif // SHADER_CLASS_HEADER