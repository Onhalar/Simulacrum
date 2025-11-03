#ifndef SHADER_CLASS_HEADER
#define SHADER_CLASS_HEADER

// only for central assignment of the 'debugMode' variable
#include <debug.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>

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

class Shader {
	public:

		GLuint ID;

		std::unordered_map<const char*, int> otherUniforms;

		int modelMatrixUniform, projectionMatrixUniform, viewMatrixUniform;

		bool hasModelMatrixUniform, hasProjectionMatrixUniform, hasViewMatrixUniform;

		glm::mat4 modelMatrix = glm::mat4(1.0f);
		glm::mat4 viewMatrix = glm::mat4(1.0f);
		glm::mat4 projectionMatrix = glm::mat4(1.0f);

		Shader(const std::filesystem::path& vertexFilepath, const std::filesystem::path& fragmentFilepath, bool throwErrors = false, const char* modelMatrixUniformName = "model", const char* viewMatrixUniformName = "view", const char* projectionMatrixUniformName = "projection") {
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
			this->activate();
			GLuint uniformID = getUniformID(name, structMode);
			if (uniformID == -1 || uniformID == GL_INVALID_INDEX) {
                return false;
            }
			glUniformMatrix4fv(otherUniforms[name], 1, GL_FALSE, glm::value_ptr(data));
            return true;
		}
		bool setUniform(const char* name, glm::vec3 data, bool structMode = false) {
			this->activate();
			GLuint uniformID = getUniformID(name, structMode);
			if (uniformID == -1 || uniformID == GL_INVALID_INDEX) {
                return false;
            }
			glUniform3fv(otherUniforms[name], 1, glm::value_ptr(data));
            return true;
		}
		bool setUniform(const char* name, glm::vec4 data, bool structMode = false) {
			this->activate();
			GLuint uniformID = getUniformID(name, structMode);
			if (uniformID == -1 || uniformID == GL_INVALID_INDEX) {
                return false;
            }
			glUniform4fv(otherUniforms[name], 1, glm::value_ptr(data));
            return true;
		}
		bool setUniform(const char* name, GLfloat data, bool structMode = false) {
			this->activate();
			GLuint uniformID = getUniformID(name, structMode);
			if (uniformID == -1 || uniformID == GL_INVALID_INDEX) {
                return false;
            }
			glUniform1f(otherUniforms[name], data);
            return true;
		}
		bool setUniform(const char* name, GLint data, bool structMode = false) {
			this->activate();
			GLuint uniformID = getUniformID(name, structMode);
			if (uniformID == -1 || uniformID == GL_INVALID_INDEX) {
                return false;
            }
			glUniform1i(otherUniforms[name], data);
            return true;
		}
		bool setUniform(const char* name, glm::vec2 data, bool structMode = false) {
			this->activate();
			GLuint uniformID = getUniformID(name, structMode);
			if (uniformID == -1 || uniformID == GL_INVALID_INDEX) {
                return false;
            }
			glUniform2fv(otherUniforms[name], 1, glm::value_ptr(data));
            return true;
		}

        void setUniformBlockBinding(const char* blockName, GLuint bindingPoint) {
            GLuint blockIndex = glGetUniformBlockIndex(ID, blockName);
            if (blockIndex != GL_INVALID_INDEX) {
                glUniformBlockBinding(ID, blockIndex, bindingPoint);
            }
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

		~Shader() {
			glDeleteProgram(ID);
		}

	private:
		GLuint makeModule(const std::filesystem::path& filepath, GLuint module_type) {
			std::ifstream file;
			std::stringstream bufferedLines;
		
			if (debugMode) { std::cout << formatProcess("Compiling") << " module '" << formatPath(getFileName(filepath.string())) << "' ... "; }
		
			file.open(filepath);
			if (file.is_open()) {
				bufferedLines << file.rdbuf();
			}
			else {
				if (debugMode) { std::cout << formatError("FAILED") << "\n"; }
				std::cerr << "unable to open " << formatPath(filepath.string()) << "\n" << std::endl;
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
				std::cerr << "Shader Module compilation error:\n" << colorText(errorLog, ANSII_YELLOW) << std::endl;
			}
			else if (debugMode && success) { std::cout << formatSuccess("Done") << std::endl; }
		
			return shaderModule;
		}
		
		GLuint makeShader(const std::filesystem::path& vertexFilepath, const std::filesystem::path& fragmentFilepath) {
			//To store all the shader modules
			std::vector<GLuint> modules;
		
			// add modules to be attached to the shader
			modules.push_back(makeModule(vertexFilepath,  GL_VERTEX_SHADER));
			modules.push_back(makeModule(fragmentFilepath, GL_FRAGMENT_SHADER));
		
			if (debugMode) { std::cout << formatProcess("Making shader") << " from '" << formatPath(getFileName(vertexFilepath.string())) << "' and '" << formatPath(getFileName(fragmentFilepath.string())) << "' ... "; }
		
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
				std::cerr << "Shader linking error:\n" << colorText(errorLog, ANSII_YELLOW) << '\n';
			}
			else if (debugMode && success) { std::cout << formatSuccess("Done") << std::endl; }
		
			//Modules are now unneeded and can be freed
			for (GLuint shaderModule : modules) {
				glDeleteShader(shaderModule);
			}
		
			return shader;
		}
};

#endif // SHADER_CLASS_HEADER
