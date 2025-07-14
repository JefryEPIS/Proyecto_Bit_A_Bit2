// CLASE: Shader
// Responsabilidad: Cargar, compilar y gestionar programas de shader.
// ===================================================================
class Shader {
public:
    unsigned int ID;

    Shader(const char* vertexSource, const char* fragmentSource) {
        GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
        GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);

        ID = glCreateProgram();
        glAttachShader(ID, vertexShader);
        glAttachShader(ID, fragmentShader);
        glLinkProgram(ID);

        // Comprobar errores de enlazado
        int success;
        glGetProgramiv(ID, GL_LINK_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetProgramInfoLog(ID, 512, NULL, infoLog);
            std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        }

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }

    void usar() const {
        glUseProgram(ID);
    }

    void setMat4(const std::string& name, const float* mat) const {
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, mat);
    }
    void setVec3(const std::string& name, float v1, float v2, float v3) const {
        glUniform3f(glGetUniformLocation(ID, name.c_str()), v1, v2, v3);
    }
    void setFloat(const std::string& name, float value) const {
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
    }
    void setVec4(const std::string& name, float v1, float v2, float v3, float v4) const {
        glUniform4f(glGetUniformLocation(ID, name.c_str()), v1, v2, v3, v4);
    }

private:
    GLuint compileShader(GLenum type, const char* source) {
        GLuint s = glCreateShader(type);
        glShaderSource(s, 1, &source, NULL);
        glCompileShader(s);
        int success;
        glGetShaderiv(s, GL_COMPILE_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetShaderInfoLog(s, 512, NULL, infoLog);
            std::cerr << "ERROR::SHADER::COMPILATION_FAILED of type " << type << "\n" << infoLog << std::endl;
        }
        return s;
    }
};
