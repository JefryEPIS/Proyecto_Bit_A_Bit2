// CLASE: RenderizadorTexto
// Responsabilidad: Encapsular toda la lógica de FreeType.
// ===================================================================
struct Character {
    unsigned int textureID;
    int width, height, bearingX, bearingY;
    unsigned int advance;
};

class RenderizadorTexto {
public:
    RenderizadorTexto(const char* rutaFuente, int tamano) :
        shader(
            "#version 330 core\nlayout (location = 0) in vec4 vertex;\nout vec2 TexCoords;\nuniform mat4 projection;\nvoid main() {\n    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);\n    TexCoords = vertex.zw;\n}",
            "#version 330 core\nin vec2 TexCoords;\nout vec4 FragColor;\nuniform sampler2D text;\nuniform vec3 textColor;\nvoid main() {\n    float alpha = texture(text, TexCoords).r;\n    FragColor = vec4(textColor, alpha);\n}"
        )
    {
        FT_Library ft;
        if (FT_Init_FreeType(&ft)) {
            std::cerr << "ERROR::FREETYPE: No se pudo inicializar FreeType" << std::endl;
            return;
        }
        FT_Face face;
        if (FT_New_Face(ft, rutaFuente, 0, &face)) {
            std::cerr << "ERROR::FREETYPE: No se pudo cargar la fuente: " << rutaFuente << std::endl;
            return;
        }
        FT_Set_Pixel_Sizes(face, 0, tamano);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        for (unsigned char c = 0; c < 128; c++) {
            if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
                std::cerr << "ERROR::FREETYTPE: Fallo al cargar el glifo " << c << std::endl;
                continue;
            }
            unsigned int texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            Characters[c] = {
                texture,
                (int)face->glyph->bitmap.width, (int)face->glyph->bitmap.rows,
                (int)face->glyph->bitmap_left, (int)face->glyph->bitmap_top,
                (unsigned int)face->glyph->advance.x
            };
        }
        glBindTexture(GL_TEXTURE_2D, 0);
        FT_Done_Face(face);
        FT_Done_FreeType(ft);

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void render(std::string texto, float x, float y, float escala, float color[3], float* projection) {
        shader.usar();
        shader.setVec3("textColor", color[0], color[1], color[2]);
        shader.setMat4("projection", projection);
        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(VAO);

        for (char c : texto) {
            Character ch = Characters[c];
            float xpos = x + ch.bearingX * escala;
            float ypos = y - (ch.height - ch.bearingY) * escala;
            float w = ch.width * escala;
            float h = ch.height * escala;
            float vertices[6][4] = {
                { xpos,     ypos + h,   0.0f, 0.0f },
                { xpos,     ypos,       0.0f, 1.0f },
                { xpos + w, ypos,       1.0f, 1.0f },
                { xpos,     ypos + h,   0.0f, 0.0f },
                { xpos + w, ypos,       1.0f, 1.0f },
                { xpos + w, ypos + h,   1.0f, 0.0f }
            };
            glBindTexture(GL_TEXTURE_2D, ch.textureID);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            x += (ch.advance >> 6) * escala;
        }
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    float getTextWidth(std::string texto, float escala) {
        float width = 0.0f;
        for (char c : texto) {
            width += (Characters[c].advance >> 6) * escala;
        }
        return width;
    }

private:
    std::map<char, Character> Characters;
    unsigned int VAO, VBO;
    Shader shader;
};
