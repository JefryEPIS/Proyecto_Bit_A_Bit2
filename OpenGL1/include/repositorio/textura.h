// CLASE: Textura
// Responsabilidad: Cargar una imagen y crear una textura de OpenGL.
// ===================================================================
class Textura {
public:
    unsigned int ID;
    Textura(const char* ruta) {
        glGenTextures(1, &ID);
        glBindTexture(GL_TEXTURE_2D, ID);

        int w, h, c;
        stbi_set_flip_vertically_on_load(true);
        unsigned char* data = stbi_load(ruta, &w, &h, &c, 0);
        if (data) {
            GLenum format = GL_RGB;
            if (c == 1) format = GL_RED;
            else if (c == 4) format = GL_RGBA;

            glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        } else {
            std::cerr << "No se pudo cargar la imagen: " << ruta << std::endl;
        }
        stbi_image_free(data);
    }

    void bind() const {
        glBindTexture(GL_TEXTURE_2D, ID);
    }
};
