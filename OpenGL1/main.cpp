#include <iostream>
#include <map>
#include <filesystem>
#include <string>
#include <fstream>
#include "include/json.hpp"
#include <string>    // Necesario para std::string

using json = nlohmann::json;

// Dependencias
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <ft2build.h>
#include FT_FREETYPE_H

// Aseg�rate de que stb_image.h est� disponible
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "include/shader.h"
#include "include/textura.h"
#include "include/renderizartexto.h"
#include "include/estado.h"
#include "include/aplicacion.h"

class Aplicacion;
class Shader;
class Textura;
class RenderizadorTexto;


int main() {
    if (!std::filesystem::exists("font")) {
        std::cerr << "Error: El directorio 'font' no existe. Aseg�rate de que tus fuentes est�n en 'font/'\n";
        return -1;
    }
    if (!std::filesystem::exists("PNG")) {
        std::cerr << "Error: El directorio 'PNG' no existe. Aseg�rate de que tus im�genes est�n en 'PNG/'\n";
        return -1;
    }

    Aplicacion miJuego(1440, 1024, "Viaje Culinario Interactivo - Versi�n en Clases");
    miJuego.ejecutar();
    return 0;
}
