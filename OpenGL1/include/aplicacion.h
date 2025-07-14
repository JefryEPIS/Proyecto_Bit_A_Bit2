#include <iostream>
#include <string>
#include <vector>
#include <glm/glm.hpp>
//#include <glm/gtc/matrix_transform.hpp> // para glm::translate, glm::rotate, glm::scale
#include <glm/gtc/type_ptr.hpp>         // para glm::value_ptr
#include <tuple>

using namespace std;

// Se añade un estado de 'hover' a la estructura para simplificar el código.
struct BotonImagen {
    float x, y, w, h;
    bool hovered = false;
};

// Estructura para almacenar la información de cada provincia
struct Provincia {
    std::string nombre;
    std::vector<glm::vec2> poligono;
    glm::vec3 color;
    glm::vec3 colorHover;
    bool hovered = false;
    glm::vec4 boundingBox; // x_min, y_min, x_max, y_max
    glm::vec2 centro;

    std::string platoBandera;
    std::string rutaImagenPlato;
    Textura* texturaPlato = nullptr; // Puntero a la textura del plato

    std::vector<std::string> historia; // Para guardar la preparación línea por línea

};

class Aplicacion {
public:
    int VentanaAncho, VentanaAlto;

    Aplicacion(int ancho, int alto, const char* titulo) {
        VentanaAncho = ancho;
        VentanaAlto = alto;
        estadoActual = EstadoApp::TITULO_INICIO; // Estado inicial

        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        ventana = glfwCreateWindow(ancho, alto, titulo, NULL, NULL);
        if (ventana == NULL) {
            std::cerr << "Fallo al crear la ventana de GLFW" << std::endl;
            glfwTerminate();
            return;
        }
        glfwMakeContextCurrent(ventana);
        glfwSetWindowUserPointer(ventana, this);

        // Configurar callbacks
        glfwSetFramebufferSizeCallback(ventana, [](GLFWwindow* w, int width, int height){
            static_cast<Aplicacion*>(glfwGetWindowUserPointer(w))->framebuffer_size_callback(width, height);
        });
        glfwSetCursorPosCallback(ventana, [](GLFWwindow* w, double x, double y){
            static_cast<Aplicacion*>(glfwGetWindowUserPointer(w))->cursor_pos_callback(x, y);
        });
        glfwSetMouseButtonCallback(ventana, [](GLFWwindow* w, int button, int action, int mods){
            static_cast<Aplicacion*>(glfwGetWindowUserPointer(w))->mouse_button_callback(button, action, mods);
        });

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            std::cerr << "Fallo al inicializar GLAD" << std::endl;
            return;
        }

        // Paleta de 13 colores para las provincias
        coloresProvincias = {
            {0.86f, 0.37f, 0.37f}, {0.86f, 0.62f, 0.37f}, {0.86f, 0.84f, 0.37f},
            {0.66f, 0.86f, 0.37f}, {0.41f, 0.86f, 0.37f}, {0.37f, 0.86f, 0.64f},
            {0.37f, 0.86f, 0.86f}, {0.37f, 0.60f, 0.86f}, {0.43f, 0.37f, 0.86f},
            {0.68f, 0.37f, 0.86f}, {0.86f, 0.37f, 0.76f}, {0.86f, 0.37f, 0.51f},
            {0.65f, 0.65f, 0.65f}
        };

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        inicializarRecursos();
        framebuffer_size_callback(VentanaAncho, VentanaAlto);
    }

    void ejecutar() {
        while (!glfwWindowShouldClose(ventana)) {
            renderizar();
            glfwSwapBuffers(ventana);
            glfwPollEvents();
        }
    }

    ~Aplicacion() {
        delete shaderImagen;
        delete shaderBoton;
        delete texChicharron;
        delete texBandera;
        delete texSistemas;
        delete texPlato;
        delete texCuchillo;
        delete texTenedor;
        delete texTrucha;
        delete texChicharron2;
        delete texIspi;
        delete texSistemas2;
        delete texUnap;
        delete texEscudo;
        delete texEsquina;
        delete texInca;
        delete texLineas;
        delete texto;
        delete textoMap;


        for (auto& prov : provincias) {
            delete prov.texturaPlato;
        }
        glDeleteVertexArrays(1, &VAO_generico);
        glDeleteBuffers(1, &VBO_generico);
        glfwTerminate();
    }

private:
    // Un vector de polígonos. Cada polígono es un vector de puntos (vértices).
    std::vector<Provincia> provincias;
    std::vector<glm::vec3> coloresProvincias;
    Provincia* provinciaSeleccionada = nullptr;

    GLFWwindow* ventana;
    Shader* shaderImagen;
    Shader* shaderBoton;
    Shader* shaderMapa;
    Textura* texChicharron;
    Textura* texBandera;
    Textura* texSistemas;
    Textura* texPlato;
    Textura* texCuchillo;
    Textura* texTenedor;
    Textura* texTrucha;
    Textura* texChicharron2;
    Textura* texIspi;
    Textura* texSistemas2;
    Textura* texUnap;
    Textura* texEscudo;
    Textura* texEsquina;
    Textura* texInca;
    Textura* texLineas;

    RenderizadorTexto* texto;
    RenderizadorTexto* textoMap;
    unsigned int VAO_generico, VBO_generico;
    float projection[16];

    EstadoApp estadoActual;

    // Estructuras para manejar todos los botones
    BotonImagen botonEmpezar;
    BotonImagen botonMapa;
    BotonImagen botonInformacion;
    BotonImagen botonTrucha;
    BotonImagen botonIspi;
    BotonImagen botonChicharron;
    BotonImagen botonVolver;

    // En aplicacion.h, dentro de la clase Aplicacion

    void cargarDatosMapa(const string& rutaArchivo) {
        ifstream archivo(rutaArchivo);
        if (!archivo.is_open()) {
            cerr << "Error: No se pudo abrir el archivo de mapa: " << rutaArchivo << endl;
            return;
        }

        json datosGeo;
        archivo >> datosGeo;

        // Mapa para asociar provincia -> {nombre del plato, ruta de imagen}
        std::map<std::string, std::tuple<std::string, std::string, std::vector<std::string>>> datosPlatos = {
            {"SANDIA", {"Juanes", "PNG/juanes.png",
                {
                 "• El juane es un plato tradicional.",
                 "• Se prepara con arroz y gallina.",
                 "• Envuelto en hojas de bijao."
                }
            }},
            {"CARABAYA", {"Mazamorra de Ayrampo", "PNG/mazamorra de ayrampo.png", {}}}, // <--- Añadir {}
            {"MELGAR", {"Asado de Cordero", "PNG/asado de cordero.png", {}}},      // <--- Añadir {}
            {"AZANGARO", {"Queso Frito", "PNG/queso frito.png",
                {
                 "• Rodajas de queso fresco.",
                 "• Se fríen hasta quedar doradas.",
                 "• Acompañado con papas."
                }
            }},
            {"LAMPA", {"Caldo de Pancita", "PNG/caldo de pancita.png", {}}},       // <--- Añadir {}
            {"SAN ROMAN", {"Caldo de Cabeza", "PNG/calde de cabeza.png", {}}},   // <--- Añadir {}
            {"PUNO", {"Trucha Frita", "PNG/trucha-frita.png",
                {
                "• Truchas limpias y sazonadas.",
                "• Se fríen en abundante aceite.",
                "• Servir con papas y choclo."
                }
            }},
            {"EL COLLAO", {"Pesq'e de Quinua", "PNG/pesque de quinua.png", {}}}, // <--- Añadir {}
            {"CHUCUITO", {"Chairo", "PNG/chairo.png", {}}},                     // <--- Añadir {}
            {"SAN ANTONIO DE PUTINA", {"Chicharron de Alpaca", "PNG/chicharron de alpaca.png", {}}},
            {"HUANCANE", {"Huarjata", "PNG/huarjata.png", {}}},                 // <--- Añadir {}
            {"MOHO", {"Chicharron Moheno", "PNG/chicharron moheno.png", {}}},   // <--- Añadir {}
            {"YUNGUYO", {"Picante Yunguyeno", "PNG/picante yunguyeno.png", {}}}  // <--- Añadir {}
        };

        // --- BUCLE ÚNICO Y CORRECTO ---
        if (datosGeo.find("features") != datosGeo.end()) {
            int colorIndex = 0;
            for (const auto& feature : datosGeo["features"]) {
                // 1. Declaramos la provincia PRIMERO.
                Provincia prov;

                // 2. Obtenemos el nombre de la provincia desde el archivo.
                if (feature.contains("properties") && feature["properties"].contains("NOMBPROV")) {
                    prov.nombre = feature["properties"]["NOMBPROV"];
                } else {
                    prov.nombre = "Desconocido";
                }

                // 3. AHORA que 'prov.nombre' tiene un valor, lo usamos para buscar los datos del plato.
                std::string nombre_upper = prov.nombre;
                std::transform(nombre_upper.begin(), nombre_upper.end(), nombre_upper.begin(), ::toupper);

                // Verificamos si la provincia existe en nuestro mapa de platos.
                if (datosPlatos.count(nombre_upper)) {
                    // Usamos std::get para acceder a los elementos del tuple.
                    prov.platoBandera    = std::get<0>(datosPlatos[nombre_upper]);
                    prov.rutaImagenPlato = std::get<1>(datosPlatos[nombre_upper]);
                    prov.historia        = std::get<2>(datosPlatos[nombre_upper]);
                }

                // 4. Asignar color único
                prov.color = coloresProvincias[colorIndex % coloresProvincias.size()];
                prov.colorHover = glm::vec3(0.980f, 0.824f, 0.004f); // Amarillo para resaltar
                colorIndex++;

                // 5. Obtener geometría y coordenadas (esto ya estaba bien)
                const auto& geometria = feature["geometry"];
                string tipo = geometria["type"];
                json coordenadas_base = geometria["coordinates"];

                if (tipo == "Polygon") {
                    coordenadas_base = coordenadas_base[0];
                } else if (tipo == "MultiPolygon") {
                    coordenadas_base = coordenadas_base[0][0];
                }

                float minX = 1e9, minY = 1e9, maxX = -1e9, maxY = -1e9;
                for (const auto& coord : coordenadas_base) {
                    float lon = coord[0].get<float>();
                    float lat = coord[1].get<float>();
                    prov.poligono.push_back(glm::vec2(lon, lat));

                    if (lon < minX) minX = lon;
                    if (lon > maxX) maxX = lon;
                    if (lat < minY) minY = lat;
                    if (lat > maxY) maxY = lat;
                }
                prov.boundingBox = glm::vec4(minX, minY, maxX, maxY);
                prov.centro = glm::vec2((minX + maxX) / 2.0f, (minY + maxY) / 2.0f);

                // 6. Añadir la provincia COMPLETA al vector.
                provincias.push_back(prov);
            }
        }
        cout << "Mapa cargado. " << provincias.size() << " provincias encontradas." << endl;
    }

    void inicializarRecursos() {
        // --- Shaders ---
        shaderImagen = new Shader(
            "#version 330 core\nlayout (location = 0) in vec2 aPos; layout (location = 1) in vec2 aTex; out vec2 TexCoords; uniform mat4 projection; void main() { gl_Position = projection * vec4(aPos, 0.0, 1.0); TexCoords = aTex; }",
            "#version 330 core\nin vec2 TexCoords; out vec4 FragColor; uniform sampler2D image; void main() { FragColor = texture(image, TexCoords); }"
        );
        shaderBoton = new Shader(
            "#version 330 core\nlayout (location = 0) in vec2 aPos; uniform mat4 projection; out vec2 v_coords; void main() { gl_Position = projection * vec4(aPos, 0.0, 1.0); v_coords = aPos; }",
            "#version 330 core\nin vec2 v_coords; out vec4 FragColor; uniform vec3 buttonColor; uniform vec4 buttonRect; uniform float borderRadius; float sdRoundedBox(vec2 p, vec2 b, float r) { vec2 q = abs(p) - b + r; return min(max(q.x, q.y), 0.0) + length(max(q, 0.0)) - r; } void main() { vec2 size = vec2(buttonRect.z, buttonRect.w); vec2 center = vec2(buttonRect.x, buttonRect.y) + size / 2.0; float dist = sdRoundedBox(v_coords - center, size / 2.0, borderRadius); float smoothed_alpha = 1.0 - smoothstep(0.0, 2.0, dist); if (smoothed_alpha < 0.1) discard; FragColor = vec4(buttonColor, smoothed_alpha); }"
        );
        shaderMapa = new Shader(
            "#version 330 core\nlayout (location = 0) in vec2 aPos; uniform mat4 projection; void main() { gl_Position = projection * vec4(aPos, 0.0, 1.0); }",
            "#version 330 core\nout vec4 FragColor; uniform vec3 mapColor; void main() { FragColor = vec4(mapColor, 1.0); }"
        );

        // --- Texturas ---
        texChicharron = new Textura("PNG/Chicharron.png");
        texBandera = new Textura("PNG/bandera_Puno.png");
        texSistemas = new Textura("PNG/sistemas1.png");
        texPlato = new Textura("PNG/plato.png");
        texCuchillo = new Textura("PNG/cuchillo.png");
        texTenedor = new Textura("PNG/tenedor.png");
        texTrucha = new Textura("PNG/Trucha Frita.png");
        texChicharron2 = new Textura("PNG/chicharron 2.png");
        texIspi = new Textura("PNG/ispi frito.png");
        texSistemas2 = new Textura("PNG/sistemas2.png");
        texUnap = new Textura("PNG/unap.png");
        texEscudo = new Textura("PNG/escudo.png");
        texEsquina = new Textura("PNG/esquina pequena.png");
        texInca = new Textura("PNG/inca.png");
        texLineas = new Textura("PNG/lineas.png");

        //cargarDatosMapa("mapa/peru_provincial_simple.geojson");
        cargarDatosMapa("mapa/1234.geojson");
        // --- Cargar texturas de los platos para cada provincia ---
        for (auto& prov : provincias) {
            if (!prov.rutaImagenPlato.empty() && std::filesystem::exists(prov.rutaImagenPlato)) {
                prov.texturaPlato = new Textura(prov.rutaImagenPlato.c_str());
            }
        }

        // --- Texto ---
        if (!std::filesystem::exists("font/LondrinaSolid-Regular.ttf")) {
            std::cerr << "Error: No se encontró la fuente en 'font/LondrinaSolid-Regular.ttf'\n";
            exit(EXIT_FAILURE);
        }
        texto = new RenderizadorTexto("font/LondrinaSolid-Regular.ttf", 85);

        if (!std::filesystem::exists("font/SansitaOne.ttf")) {
            std::cerr << "Error: No se encontró la fuente en 'font/LondrinaSolid-Regular.ttf'\n";
            exit(EXIT_FAILURE);
        }
        textoMap = new RenderizadorTexto("font/SansitaOne.ttf", 85);
        // --- Geometría ---
        glGenVertexArrays(1, &VAO_generico);
        glGenBuffers(1, &VBO_generico);
        glBindVertexArray(VAO_generico);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_generico);
        glBufferData(GL_ARRAY_BUFFER,10192 * sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glBindVertexArray(0);
    }

    // En aplicacion.h, dentro de la clase Aplicacion
    void renderizarMapa() {
        shaderMapa->usar();
        shaderMapa->setMat4("projection", projection);

        // --- Lógica para hacer el mapa responsivo ---
        float margen = 0.1f; // 10% de margen en la ventana
        float targetAncho = VentanaAncho * (1.0f - 2.0f * margen);
        float targetAlto = VentanaAlto * (1.0f - 2.0f * margen);

        // Calcular la caja delimitadora de todo el mapa de Puno
        float minX = 1e9, minY = 1e9, maxX = -1e9, maxY = -1e9;
        for (const auto& prov : provincias) {
            if (prov.boundingBox.x < minX) minX = prov.boundingBox.x;
            if (prov.boundingBox.y < minY) minY = prov.boundingBox.y;
            if (prov.boundingBox.z > maxX) maxX = prov.boundingBox.z;
            if (prov.boundingBox.w > maxY) maxY = prov.boundingBox.w;
        }

        float mapaAnchoGeo = maxX - minX;
        float mapaAltoGeo = maxY - minY;

        // Calcular escala para que quepa en el area objetivo
        float escalaX = targetAncho / mapaAnchoGeo;
        float escalaY = targetAlto / mapaAltoGeo;
        float escalaMapa = std::min(escalaX, escalaY);

        // Calcular el centro para posicionar el mapa en la ventana
        float centroMapaGeoX = minX + mapaAnchoGeo / 2.0f;
        float centroMapaGeoY = minY + mapaAltoGeo / 2.0f;
        float centroVentanaX = VentanaAncho / 4.0f;
        float centroVentanaY = VentanaAlto / 2.0f;

        // --- Renderizar cada provincia ---
        glBindVertexArray(VAO_generico);
        glDisableVertexAttribArray(1);

        for (const auto& prov : provincias) {
            // Establecer el color (normal o hover)
            if (prov.hovered) {
                shaderMapa->setVec3("mapColor", prov.colorHover.r, prov.colorHover.g, prov.colorHover.b);
            } else {
                shaderMapa->setVec3("mapColor", prov.color.r, prov.color.g, prov.color.b);
            }

            vector<float> vertices;
            for (const auto& punto : prov.poligono) {
                // Transformar coordenadas geográficas a coordenadas de pantalla
                float pixelX = centroVentanaX + (punto.x - centroMapaGeoX) * escalaMapa;
                float pixelY = centroVentanaY + (punto.y - centroMapaGeoY) * escalaMapa;
                vertices.push_back(pixelX);
                vertices.push_back(pixelY);
            }

            glBindBuffer(GL_ARRAY_BUFFER, VBO_generico);
            glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), vertices.data());
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);

            // Dibujar el relleno del polígono
            glDrawArrays(GL_TRIANGLE_FAN, 0, prov.poligono.size());

            // Dibujar el borde
            shaderMapa->setVec3("mapColor", 0.1f, 0.1f, 0.1f); // Borde negro
            glLineWidth(2.0f);
            glDrawArrays(GL_LINE_LOOP, 0, prov.poligono.size());
        }

        // --- Renderizar el texto de cada provincia ---
        float negro[] = {0.0f, 0.0f, 0.0f};
        float escalaTexto = VentanaAlto / 3000.0f; // Escala de texto responsiva

        for (const auto& prov : provincias) {
            // Transformar el centro de la provincia a coordenadas de pantalla
            float textoX = centroVentanaX + (prov.centro.x - centroMapaGeoX) * escalaMapa;
            float textoY = centroVentanaY + (prov.centro.y - centroMapaGeoY) * escalaMapa;

            // Centrar el texto en su posición
            float anchoTexto = texto->getTextWidth(prov.nombre, escalaTexto);
            texto->render(prov.nombre, textoX - anchoTexto / 2.0f, textoY, escalaTexto, negro, projection);
        }

        // Restaurar atributos de VBO para el resto de la aplicación
        glBindBuffer(GL_ARRAY_BUFFER, VBO_generico);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);
    }



    void renderizar() {
        glClearColor(1.0f, 0.95f, 0.88f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        float negro[] = { 0.0f, 0.0f, 0.0f };
        float blanco[] = { 1.0f, 1.0f, 1.0f };
        float amarillo[] = { 0.980f, 0.824f, 0.004f };
        float verde[] = {0.184f, 0.49f, 0.439f};
        float marron[] = {0.851f, 0.424f, 0.231f};

        switch (estadoActual) {
            case EstadoApp::TITULO_INICIO: {
                float escalaTextoTitulo = VentanaAlto / 600.0f;
                texto->render("Un Viaje", 0.1f * VentanaAncho, VentanaAlto - 215 * escalaTextoTitulo, escalaTextoTitulo, negro, projection);
                texto->render("Culinario", 0.1f * VentanaAncho, VentanaAlto - 300 * escalaTextoTitulo, escalaTextoTitulo, negro, projection);
                texto->render("Interactivo", 0.1f * VentanaAncho, VentanaAlto - 385 * escalaTextoTitulo, escalaTextoTitulo, negro, projection);

                shaderImagen->usar();
                shaderImagen->setMat4("projection", projection);
                glBindVertexArray(VAO_generico);
                renderizarQuad(texChicharron, 0.45f * VentanaAncho, 0.3f * VentanaAlto, 0.5f * VentanaAncho, 0.5f * VentanaAlto);
                renderizarQuad(texBandera, 0.02f * VentanaAncho, 0.83f * VentanaAlto, 0.15f * VentanaAncho, 0.15f * VentanaAlto);
                renderizarQuad(texSistemas, 0.86f * VentanaAncho, 0.82f * VentanaAlto, 0.12f * VentanaAncho, 0.15f * VentanaAlto);

                renderizarBoton(botonEmpezar, "Empezar", blanco);
                break;
            }
            case EstadoApp::PANTALLA_MENU: {
                float escalaTextoTitulo = VentanaAlto / 600.0f;

                shaderImagen->usar();
                shaderImagen->setMat4("projection", projection);
                glBindVertexArray(VAO_generico);

                renderizarQuad(texPlato, 0.35f * VentanaAncho, 0.62f * VentanaAlto, 0.30f * VentanaAncho, 0.35f * VentanaAlto);
                renderizarQuad(texTenedor, 0.30f * VentanaAncho, 0.68f * VentanaAlto, 0.25f * VentanaAncho, 0.30f * VentanaAlto);
                renderizarQuad(texCuchillo, 0.55f * VentanaAncho, 0.68f * VentanaAlto, 0.25f * VentanaAncho, 0.30f * VentanaAlto);

                renderizarBotonImagen(texTrucha, botonTrucha);
                renderizarBotonImagen(texIspi, botonIspi);
                renderizarBotonImagen(texChicharron2, botonChicharron);

                renderizarBoton(botonMapa, "Mapa", blanco);
                //necesario para dos botones iguales (esto limpia el anterior)
                glBindVertexArray(VAO_generico);
                renderizarBoton(botonInformacion, "Creditos", blanco);

                float anchoTextoMenu = texto->getTextWidth("MENU", escalaTextoTitulo);
                texto->render("MENU", (VentanaAncho - anchoTextoMenu) / 2.0f, VentanaAlto - 150 * escalaTextoTitulo, escalaTextoTitulo, amarillo, projection);

                break;
            }
            case EstadoApp::PANTALLA_TRUCHA: {
                renderizarPantallaPlato(texTrucha, "Trucha Frita");
                float escalaTextoTitulo = VentanaAlto / 1900.0f;
                texto->render("• Truchas limpias, preparamos la mezcla de condimentos, esta vez estilo pollada, luego", 0.055f * VentanaAncho, VentanaAlto -1230 * escalaTextoTitulo, escalaTextoTitulo, negro, projection);
                texto->render("• lavamos las truchas una a una y dejamos reposar por media hora aproximado", 0.055f * VentanaAncho, VentanaAlto -1315 * escalaTextoTitulo, escalaTextoTitulo, negro, projection);
                texto->render("• Freímos. Servimos de inmediato. Buen provecho", 0.055f * VentanaAncho, VentanaAlto -1400 * escalaTextoTitulo, escalaTextoTitulo, negro, projection);

                texto->render("• INGREDIENTES:", 0.6f * VentanaAncho, VentanaAlto -350 * escalaTextoTitulo, escalaTextoTitulo, negro, projection);
                texto->render("• * 5 truchas deshuesadas", 0.57f * VentanaAncho, VentanaAlto -450 * escalaTextoTitulo, escalaTextoTitulo, negro, projection);
                texto->render("• * Al gusto salpimienta", 0.57f * VentanaAncho, VentanaAlto -550 * escalaTextoTitulo, escalaTextoTitulo, negro, projection);
                texto->render("• * 3 Cucharadas de vinagre", 0.57 * VentanaAncho, VentanaAlto -650 * escalaTextoTitulo, escalaTextoTitulo, negro, projection);
                texto->render("• * comino, orégano, sazonador", 0.57f * VentanaAncho, VentanaAlto -750 * escalaTextoTitulo, escalaTextoTitulo, negro, projection);

                break;
            }
            case EstadoApp::PANTALLA_ISPI: {
                renderizarPantallaPlato(texIspi, "Ispi Frito");

                float escalaTextoTitulo = VentanaAlto / 1900.0f;
                // preparacion
                texto->render("• Lava bien los ispis y escurrelos,Mezcla harina con sal y cubre los pescados, Calienta", 0.055f * VentanaAncho, VentanaAlto -1230 * escalaTextoTitulo, escalaTextoTitulo, negro, projection);
                texto->render("• abundante aceite en una sarten, Frie los ispis hasta que queden bien dorados y", 0.055f * VentanaAncho, VentanaAlto -1315 * escalaTextoTitulo, escalaTextoTitulo, negro, projection);
                texto->render("crujientes Retiralos del aceite y dejalos escurrir en papel, Sirve caliente con mote, papa o limon.", 0.055f * VentanaAncho, VentanaAlto -1400 * escalaTextoTitulo, escalaTextoTitulo, negro, projection);
                // ingredientes
                texto->render("• INGREDIENTES:", 0.6f * VentanaAncho, VentanaAlto -350 * escalaTextoTitulo, escalaTextoTitulo, negro, projection);
                texto->render("• * 1 libra de ispi fresco", 0.57f * VentanaAncho, VentanaAlto -450 * escalaTextoTitulo, escalaTextoTitulo, negro, projection);
                texto->render("• * 1 taza de harina de trigo", 0.57f * VentanaAncho, VentanaAlto -550 * escalaTextoTitulo, escalaTextoTitulo, negro, projection);
                texto->render("• * Sal al gusto", 0.57 * VentanaAncho, VentanaAlto -650 * escalaTextoTitulo, escalaTextoTitulo, negro, projection);
                texto->render("• * Pimienta al gusto (opcional)", 0.57f * VentanaAncho, VentanaAlto -750 * escalaTextoTitulo, escalaTextoTitulo, negro, projection);
                texto->render("• * Aceite para freir", 0.57f * VentanaAncho, VentanaAlto -835 * escalaTextoTitulo, escalaTextoTitulo, negro, projection);


                break;
            }
            case EstadoApp::PANTALLA_CHICHARRON: {
                renderizarPantallaPlato(texChicharron2, "Chicharron de Alpaca");

                float escalaTextoTitulo = VentanaAlto / 1900.0f;
                // preparacion
                texto->render("cortamos el chuno por la mitad sin llegar a partirlo y colocar queso dentro listo", 0.055f * VentanaAncho, VentanaAlto -1230 * escalaTextoTitulo, escalaTextoTitulo, negro, projection);
                texto->render("para sancochar, cortar la carne en trozos pequeños, lo sazonamos por harina luego", 0.055f * VentanaAncho, VentanaAlto -1315 * escalaTextoTitulo, escalaTextoTitulo, negro, projection);
                texto->render("sumergir en huevo batido luego freir con abundante aceite .Al terminar ponemos los", 0.055f * VentanaAncho, VentanaAlto -1400 * escalaTextoTitulo, escalaTextoTitulo, negro, projection);
                texto->render("trozos fritos sobre papel absorbente por unos minutos antes de servirlo", 0.055f * VentanaAncho, VentanaAlto -1485 * escalaTextoTitulo, escalaTextoTitulo, negro, projection);
                // ingredientes
                texto->render("• INGREDIENTES:", 0.6f * VentanaAncho, VentanaAlto -350 * escalaTextoTitulo, escalaTextoTitulo, negro, projection);
                texto->render("• * 1 kilo de carne de alpaca", 0.57f * VentanaAncho, VentanaAlto -450 * escalaTextoTitulo, escalaTextoTitulo, negro, projection);
                texto->render("• * 1/4 kilo de chuño", 0.57f * VentanaAncho, VentanaAlto -550 * escalaTextoTitulo, escalaTextoTitulo, negro, projection);
                texto->render("• * 1/4 kilo de queso ", 0.57 * VentanaAncho, VentanaAlto -650 * escalaTextoTitulo, escalaTextoTitulo, negro, projection);
                texto->render("• * 2 huevos", 0.57f * VentanaAncho, VentanaAlto -750 * escalaTextoTitulo, escalaTextoTitulo, negro, projection);
                texto->render("• * Harina", 0.57f * VentanaAncho, VentanaAlto -835 * escalaTextoTitulo, escalaTextoTitulo, negro, projection);
                texto->render("• * Sal, Comino, Pimienta", 0.57f * VentanaAncho, VentanaAlto -920 * escalaTextoTitulo, escalaTextoTitulo, negro, projection);
                break;
            }
            case EstadoApp::PANTALLA_INFORMACION: {
               // --- DIBUJAR DECORACIÓN (con código) ---
                // Esquinas verdes
                float esquinaW = 0.06f * VentanaAncho;
                float esquinaH = 0.20f * VentanaAncho;
                float esquinaOffset = 0.08f * VentanaAncho;

                renderizarRectanguloSolido(0, VentanaAlto - esquinaH, esquinaW, esquinaH, verde);
                renderizarRectanguloSolido(esquinaW, VentanaAlto - esquinaW, esquinaOffset, esquinaW, verde);
                renderizarRectanguloSolido(VentanaAncho - esquinaW, VentanaAlto - esquinaH, esquinaW, esquinaH, verde);
                renderizarRectanguloSolido(VentanaAncho - esquinaW - esquinaOffset, VentanaAlto - esquinaW, esquinaOffset, esquinaW, verde);
                // --- TÍTULO "CREDITOS" ---
                BotonImagen cajaCreditos; // Usamos la struct para conveniencia
                cajaCreditos.w = 0.4f * VentanaAncho;
                cajaCreditos.h = 0.1f * VentanaAlto;
                cajaCreditos.x = (VentanaAncho - cajaCreditos.w) / 2.0f;
                cajaCreditos.y = VentanaAlto - cajaCreditos.h - (0.1f * VentanaAlto);

                shaderBoton->usar();
                shaderBoton->setMat4("projection", projection);
                shaderBoton->setVec3("buttonColor", marron[0], marron[1], marron[2]);
                shaderBoton->setVec4("buttonRect", cajaCreditos.x, cajaCreditos.y, cajaCreditos.w, cajaCreditos.h);
                shaderBoton->setFloat("borderRadius", 30.0f * (VentanaAlto / 1024.0f));
                glEnableVertexAttribArray(0);
                glDisableVertexAttribArray(1);
                renderizarQuad(nullptr, cajaCreditos.x, cajaCreditos.y, cajaCreditos.w, cajaCreditos.h);
                glEnableVertexAttribArray(1);

                float escalaCreditos = VentanaAlto / 900.0f;
                float anchoCreditos = texto->getTextWidth("CREDITOS", escalaCreditos);
                texto->render("CREDITOS", cajaCreditos.x + (cajaCreditos.w - anchoCreditos) / 2.0f, cajaCreditos.y + (cajaCreditos.h / 2.0f) - (35.0f * escalaCreditos), escalaCreditos, blanco, projection);

                // --- LOGOS ---
                shaderImagen->usar();
                shaderImagen->setMat4("projection", projection);
                glBindVertexArray(VAO_generico);

                renderizarQuad(texSistemas2, 0.1f * VentanaAncho, 0.4f * VentanaAlto, 0.20f * VentanaAncho, 0.25f * VentanaAlto);
                renderizarQuad(texUnap, 0.30f * VentanaAncho, 0.32f * VentanaAlto, 0.40f * VentanaAncho, 0.55f * VentanaAlto);
                renderizarQuad(texEscudo, 0.7f * VentanaAncho, 0.4f * VentanaAlto, 0.15f * VentanaAncho, 0.26f * VentanaAlto);

                // --- LÍNEA SEPARADORA ---
                float lineaY = 0.35f * VentanaAlto;
                float lineaH = 3.0f;
                renderizarRectanguloSolido(0.1f * VentanaAncho, lineaY - (lineaH/2.0f), 0.8f * VentanaAncho, lineaH, negro);

                float escalaTextoTitulo = VentanaAlto / 1900.0f;

                texto->render("• Universidad Nacional del Altiplano Puno", 0.03f * VentanaAncho, VentanaAlto -1400 * escalaTextoTitulo, escalaTextoTitulo, negro, projection);
                texto->render("• Puno \"Ciudad del Lago Sagrado\"", 0.03f * VentanaAncho, VentanaAlto -1485 * escalaTextoTitulo, escalaTextoTitulo, negro, projection);
                texto->render("• Ing. Hugo Yosef Gomez Quispe", 0.03f * VentanaAncho, VentanaAlto -1570 * escalaTextoTitulo, escalaTextoTitulo, negro, projection);

                texto->render("• Quispe Galinda Jahan Kevin", 0.52f * VentanaAncho, VentanaAlto -1400 * escalaTextoTitulo, escalaTextoTitulo, negro, projection);
                texto->render("• Condori Huanca Dennys Anthoni", 0.52f * VentanaAncho, VentanaAlto -1485 * escalaTextoTitulo, escalaTextoTitulo, negro, projection);
                texto->render("• Ramos Castillo Jefry Edinson", 0.52f * VentanaAncho, VentanaAlto -1570 * escalaTextoTitulo, escalaTextoTitulo, negro, projection);

                // --- BOTÓN VOLVER ---
                renderizarBoton(botonVolver, "Volver", verde);
                break;
            }
            /*case EstadoApp::PANTALLA_MAPA: {
                float escalaTexto = VentanaAlto / 600.0f;
                float anchoTexto = texto->getTextWidth("Mapa de Puno", escalaTexto);

                texto->render("Mapa de Puno", (VentanaAncho - anchoTexto) / 2.0f, VentanaAlto -100 * escalaTexto, escalaTexto, negro, projection);
                renderizarMapa(); // ¡Llamamos a la función que dibuja el mapa!

                // Reutilizamos el botón "Volver" para poder salir del mapa
                renderizarBoton(botonVolver, "Volver", negro);
                break;
            }
            */
            case EstadoApp::PANTALLA_MAPA: {
                float escalaTextoMap = VentanaAlto / 1800.0f;
                textoMap->render("PLATOS DE LA REGION", 0.10f * VentanaAncho, VentanaAlto - 115 * escalaTextoMap, escalaTextoMap, negro, projection);
                textoMap->render("PUNO", 0.20f * VentanaAncho, VentanaAlto - 215 * escalaTextoMap, escalaTextoMap, negro, projection);

                shaderImagen->usar();
                shaderImagen->setMat4("projection", projection);
                glBindVertexArray(VAO_generico);

                float incaW = VentanaAncho * 0.08f;
                float incaH = VentanaAlto * 0.15f;
                float incaY = VentanaAlto * 0.83f;

                renderizarQuad(texInca, VentanaAncho * 0.01f, incaY, incaW, incaH);
                // Dibujar el inca de la derecha (volteado horizontalmente)
                {
                    texInca->bind(); // Asegurarse de que la textura del inca esté activa
                    float x = VentanaAncho * 0.42f;
                    float y = incaY;
                    float w = incaW;
                    float h = incaH;

                    // Vértices con coordenadas U (el primer valor de textura) invertidas
                    float verticesIncaDerecha[] = {
                        // Posición      // Coordenada Textura (U invertida)
                        x,     y,         1.0f, 0.0f,
                        x,     y + h,     1.0f, 1.0f,
                        x + w, y + h,     0.0f, 1.0f,

                        x,     y,         1.0f, 0.0f,
                        x + w, y + h,     0.0f, 1.0f,
                        x + w, y,         0.0f, 0.0f
                    };

                    // Reconfiguramos el formato para este quad antes de dibujarlo
                    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
                    glEnableVertexAttribArray(0);
                    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
                    glEnableVertexAttribArray(1);

                    glBindBuffer(GL_ARRAY_BUFFER, VBO_generico);
                    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verticesIncaDerecha), verticesIncaDerecha);
                    glDrawArrays(GL_TRIANGLES, 0, 6);
                }

                renderizarMapa();
                {
                    // Se definen los vértices de la línea vertical.
                    float lineaVertices[] = {
                        // Punto Superior (centro horizontal, arriba)
                        (float)VentanaAncho / 2.0f, (float)VentanaAlto,
                        // Punto Inferior (centro horizontal, abajo)
                        (float)VentanaAncho / 2.0f, 0.0f
                    };

                    // Reutilizamos el shader del mapa para dibujar la línea
                    shaderMapa->usar();
                    shaderMapa->setVec3("mapColor", 0.0f, 0.0f, 0.0f);

                    // Enviamos los datos de la línea a la GPU
                    glBindVertexArray(VAO_generico);
                    glBindBuffer(GL_ARRAY_BUFFER, VBO_generico);
                    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(lineaVertices), lineaVertices);

                    // Establecemos el formato de los vértices (X, Y)
                    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
                    glEnableVertexAttribArray(0);
                    glDisableVertexAttribArray(1);

                    // Dibujamos la línea
                    glLineWidth(5.0f);
                    glDrawArrays(GL_LINES, 0, 2);
                }
                // --- FIN DEL CÓDIGO DE LA LÍNEA ---

                glEnableVertexAttribArray(1); // ¡Rehabilita el atributo para las coordenadas de textura!
                glLineWidth(1.0f);
                // Dibuja el panel de detalles en la mitad derecha si hay una provincia seleccionada
                if (provinciaSeleccionada != nullptr) {
                    float panelX = VentanaAncho / 2.0f + 50;
                    float panelW = VentanaAncho / 2.0f - 100;

                    // Renderizar Título de la Provincia
                    float escalaTitulo = VentanaAlto / 1000.0f;
                    float anchoTitulo = texto->getTextWidth(provinciaSeleccionada->nombre, escalaTitulo);
                    texto->render(provinciaSeleccionada->nombre, panelX + (panelW - anchoTitulo) / 2.0f, VentanaAlto * 0.85f, escalaTitulo, negro, projection);

                    // Renderizar subtítulo "Plato Bandera"
                    float escalaSub = VentanaAlto / 1500.0f;
                    texto->render("Plato Bandera", panelX, VentanaAlto * 0.75f, escalaSub, marron, projection);

                    // Renderizar nombre del plato
                    float escalaPlato = VentanaAlto / 1500.0f;
                    texto->render("\"" + provinciaSeleccionada->platoBandera + "\"", panelX, VentanaAlto * 0.68f, escalaPlato, negro, projection);

                    // Renderizar imagen del plato
                    if (provinciaSeleccionada->texturaPlato != nullptr) {
                        std::cout << "Renderizando detalles para: " << provinciaSeleccionada->nombre << std::endl;
                        shaderImagen->usar();
                        shaderImagen->setMat4("projection", projection);
                        glBindVertexArray(VAO_generico);
                        glBindBuffer(GL_ARRAY_BUFFER, VBO_generico);

                        // Atributo 0: Posición (2 floats, el stride total es de 4 floats, empieza al inicio)
                        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
                        glEnableVertexAttribArray(0);

                        // Atributo 1: Coordenada de Textura (2 floats, stride de 4, empieza después de los 2 de posición)
                        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
                        glEnableVertexAttribArray(1);

                        float imgW = panelW * 0.4f;
                        float imgH = imgW; // Hacerla cuadrada
                        float imgX = panelX + (panelW - imgW) / 50.5f;
                        float imgY = VentanaAlto * 0.35f;
                        renderizarQuad(provinciaSeleccionada->texturaPlato, imgX, imgY, imgW, imgH);
                    }
                    // AHORA el código para renderizar la historia puede ver 'panelX'
                    if (!provinciaSeleccionada->historia.empty()) {
                        float escalaPrep = VentanaAlto / 2500.0f;
                        float posYInicial = VentanaAlto * 0.30f;
                        float espaciado = 70.0f * escalaPrep;

                        // CORRECCIÓN: Usamos size_t para evitar el warning de signed/unsigned
                        for (size_t i = 0; i < provinciaSeleccionada->historia.size(); ++i) {
                            float posY = posYInicial - (i * espaciado);
                            // Ahora 'panelX' es visible y no dará error
                            texto->render(provinciaSeleccionada->historia[i], panelX, posY, escalaPrep, negro, projection);
                        }
                    }

                } else {
                     // Opcional: Mostrar un texto indicando que se seleccione una provincia
                     float escalaTexto = VentanaAlto / 1200.0f;
                     float anchoTexto = texto->getTextWidth("Selecciona una provincia", escalaTexto);
                     texto->render("Selecciona una provincia", VentanaAncho * 0.75f - anchoTexto / 2.0f, VentanaAlto * 0.5f, escalaTexto, negro, projection);
                }

                renderizarBoton(botonVolver, "Volver", verde);
                glBindVertexArray(VAO_generico);
                glBindBuffer(GL_ARRAY_BUFFER, VBO_generico);

                // Restaurar formato de vértices para Quads con textura (x,y,u,v)
                glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
                glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
                glEnableVertexAttribArray(1);

                glEnableVertexAttribArray(0);
                glEnableVertexAttribArray(1);

                // Restaurar el grosor de la línea al valor por defecto
                glLineWidth(1.0f);
                break;
            }
        }
    }

    // En aplicacion.h, dentro de la clase Aplicacion

    bool estaDentroDelPoligono(const glm::vec2& punto, const std::vector<glm::vec2>& poligono) {
        bool dentro = false;
        int n = poligono.size();
        for (int i = 0, j = n - 1; i < n; j = i++) {
            if (((poligono[i].y > punto.y) != (poligono[j].y > punto.y)) &&
                (punto.x < (poligono[j].x - poligono[i].x) * (punto.y - poligono[i].y) / (poligono[j].y - poligono[i].y) + poligono[i].x)) {
                dentro = !dentro;
            }
        }
        return dentro;
    }
    void renderizarRectanguloSolido(float x, float y, float w, float h, float* color) {
        shaderBoton->usar();
        shaderBoton->setMat4("projection", projection);
        shaderBoton->setVec3("buttonColor", color[0], color[1], color[2]);
        shaderBoton->setVec4("buttonRect", x, y, w, h);
        shaderBoton->setFloat("borderRadius", 0.0f); // Sin bordes redondeados
        glBindVertexArray(VAO_generico);
        glEnableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        renderizarQuad(nullptr, x, y, w, h);
        glEnableVertexAttribArray(1);
    }
/*
    void renderizarPantallaPlato(Textura* texturaPlato, const std::string& titulo) {
        float negro[] = {0.0f, 0.0f, 0.0f};
        float blanco[] = {1.0f, 1.0f, 1.0f};
        float escalaTexto = VentanaAlto / 600.0f;

        float anchoTexto = texto->getTextWidth(titulo, escalaTexto);
        texto->render(titulo, (VentanaAncho - anchoTexto) / 2.0f, VentanaAlto - 150 * escalaTexto, escalaTexto, negro, projection);

        shaderImagen->usar();
        shaderImagen->setMat4("projection", projection);
        glBindVertexArray(VAO_generico);

       // renderizarQuad(texturaPlato, imgX, imgY, imgW, imgH);

        renderizarQuad(texIspi, 0.15f * VentanaAncho, 0.5f * VentanaAlto, 0.5f * VentanaAncho, 0.3f * VentanaAlto);
        renderizarQuad(texEsquina, 0.0f * VentanaAncho, 0.0f * VentanaAlto, 0.1f * VentanaAncho, 0.1f * VentanaAlto);

        renderizarBoton(botonVolver, "Volver", negro);
    }
    */

    void renderizarPantallaPlato(Textura* texturaPlato, const std::string& titulo) {
        float negro[] = {0.0f, 0.0f, 0.0f};
        float escalaTexto = VentanaAlto / 1000.0f;
        float gris[] = {0.851f, 0.851f, 0.851f};
        float verde[] = {0.2f, 0.7f, 0.5f}; // Color verde para el cuadro superior derecho

        // Renderizar el cuadro principal con la imagen del plato
        shaderImagen->usar();
        shaderImagen->setMat4("projection", projection);
        glBindVertexArray(VAO_generico);

        // Dimensiones y posición del cuadro principal imagen
        float imgW = 0.37f * VentanaAncho; // Ancho del cuadro principal
        float imgH = imgW;               // Alto del cuadro principal (cuadrado)
        float imgX = 0.05f * VentanaAncho; // Posición x del cuadro principal
        float imgY = 0.45f * VentanaAlto;   // Posición y del cuadro principal

        // Renderizar el fondo colorido (puede ser una textura o un rectángulo sólido)
        //renderizarRectanguloSolido(imgX, imgY, imgW, imgH, gris); // Fondo gris

        // Renderizar la imagen del plato
        renderizarQuad(texturaPlato, imgX, imgY, imgW, imgH);

        // Renderizar el título del plato
        float anchoTexto = texto->getTextWidth(titulo, escalaTexto);

        // Renderizar el cuadro verde en la parte superior derecha
        float cuadroVerdeW = 0.4f * VentanaAncho; // Ancho del cuadro verde
        float cuadroVerdeH = 0.1f * VentanaAlto;  // Alto del cuadro verde
        float cuadroVerdeX = 0.55f * VentanaAncho; // Posición x del cuadro verde
        float cuadroVerdeY = 0.85f * VentanaAlto;  // Posición y del cuadro verde

        renderizarRectanguloSolido(cuadroVerdeX, cuadroVerdeY, cuadroVerdeW, cuadroVerdeH, verde);

        // Renderizar el título dentro del cuadro verde
        float posXTexto = cuadroVerdeX + (cuadroVerdeW - anchoTexto) / 2.0f;
        float posYTexto = cuadroVerdeY + (cuadroVerdeH / 2.0f) - (30.0f * escalaTexto);
        texto->render(titulo, posXTexto, posYTexto, escalaTexto, negro, projection);

        // Renderizar el cuadro gris debajo del cuadro verde
        float cuadroGrisAdicionalW = 0.4f * VentanaAncho; // Ancho del cuadro gris adicional
        float cuadroGrisAdicionalH = 0.35f * VentanaAlto;  // Alto del cuadro gris adicional
        float cuadroGrisAdicionalX = 0.55f * VentanaAncho; // Posición x del cuadro gris adicional
        float cuadroGrisAdicionalY = cuadroVerdeY - cuadroGrisAdicionalH; // Posición y del cuadro gris adicional

        renderizarRectanguloSolido(cuadroGrisAdicionalX, cuadroGrisAdicionalY, cuadroGrisAdicionalW, cuadroGrisAdicionalH, gris);

        // Renderizar el cuadro gris en la parte inferior
        float cuadroGrisW = 0.9f * VentanaAncho; // Ancho del cuadro gris
        float cuadroGrisH = 0.3f * VentanaAlto;  // Alto del cuadro gris
        float cuadroGrisX = 0.05f * VentanaAncho; // Posición x del cuadro gris
        float cuadroGrisY = 0.10f * VentanaAlto;  // Posición y del cuadro gris

        renderizarRectanguloSolido(cuadroGrisX, cuadroGrisY, cuadroGrisW, cuadroGrisH, gris);

        // Botón "Volver"
        renderizarBoton(botonVolver, "Volver", negro);
    }

    void renderizarBotonImagen(Textura* textura, const BotonImagen& boton) {
        shaderImagen->usar();
        if (boton.hovered) {
            float scale = 1.05f;
            float newW = boton.w * scale;
            float newH = boton.h * scale;
            float newX = boton.x - (newW - boton.w) / 2.0f;
            float newY = boton.y - (newH - boton.h) / 2.0f;
            renderizarQuad(textura, newX, newY, newW, newH);
        } else {
            renderizarQuad(textura, boton.x, boton.y, boton.w, boton.h);
        }
    }

    void renderizarBoton(const BotonImagen& boton, const std::string& textoBoton, float* colorTexto) {
         shaderBoton->usar();
         shaderBoton->setMat4("projection", projection);
        if (boton.hovered) {
            shaderBoton->setVec3("buttonColor", (217.0f / 255.0f) * 0.85f, (108.0f / 255.0f) * 0.85f, (59.0f / 255.0f) * 0.85f);
        } else {
            shaderBoton->setVec3("buttonColor", 217.0f / 255.0f, 108.0f / 255.0f, 59.0f / 255.0f);
        }
        shaderBoton->setVec4("buttonRect", boton.x, boton.y, boton.w, boton.h);
        shaderBoton->setFloat("borderRadius", 20.0f * (VentanaAlto / 1024.0f));
        glEnableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        renderizarQuad(nullptr, boton.x, boton.y, boton.w, boton.h);
        glEnableVertexAttribArray(1);

        float escalaTextoBoton = VentanaAlto / 1600.0f;
        float anchoTextoBoton = texto->getTextWidth(textoBoton, escalaTextoBoton);
        float xTexto = boton.x + (boton.w - anchoTextoBoton) / 2.0f;
        float yTexto = boton.y + (boton.h / 2.0f) + (-25.0f * escalaTextoBoton);
        texto->render(textoBoton, xTexto, yTexto, escalaTextoBoton, colorTexto, projection);
    }

    void renderizarQuad(Textura* textura, float x, float y, float w, float h) {
        if (textura) {
            textura->bind();
        }
        float vertices[] = {
            x,     y,     0.0f, 0.0f,
            x,     y + h, 0.0f, 1.0f,
            x + w, y + h, 1.0f, 1.0f,
            x,     y,     0.0f, 0.0f,
            x + w, y + h, 1.0f, 1.0f,
            x + w, y,     1.0f, 0.0f
        };
        glBindBuffer(GL_ARRAY_BUFFER, VBO_generico);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    void ortho(float left, float right, float bottom, float top, float* m) {
        for (int i = 0; i < 16; i++) m[i] = 0;
        m[0] = 2.0f / (right - left);
        m[5] = 2.0f / (top - bottom);
        m[10] = -2.0f;
        m[12] = -(right + left) / (right - left);
        m[13] = -(top + bottom) / (top - bottom);
        m[14] = -1.0f;
        m[15] = 1.0f;
    }

    void framebuffer_size_callback(int width, int height) {
        VentanaAncho = width;
        VentanaAlto = height;
        glViewport(0, 0, width, height);
        ortho(0, (float)width, 0, (float)height, projection);

        botonEmpezar.w = 0.25f * VentanaAncho;
        botonEmpezar.h = 0.1f * VentanaAlto;
        botonEmpezar.x = (VentanaAncho - botonEmpezar.w) / 2.0f;
        botonEmpezar.y = 0.10f * VentanaAlto;

        // Se definen las posiciones de los botones-plato
        botonTrucha.w = 0.25f * VentanaAncho;
        botonTrucha.h = 0.30f * VentanaAlto;
        botonTrucha.x = 0.05f * VentanaAncho;
        botonTrucha.y = 0.30f * VentanaAlto;

        botonIspi.w = 0.25f * VentanaAncho;
        botonIspi.h = 0.30f * VentanaAlto;
        botonIspi.x = (VentanaAncho - botonEmpezar.w) / 2.0f;       //0.375f * VentanaAncho;
        botonIspi.y = 0.30f * VentanaAlto;

        botonChicharron.w = 0.25f * VentanaAncho;
        botonChicharron.h = 0.30f * VentanaAlto;
        botonChicharron.x = 0.70f * VentanaAncho;
        botonChicharron.y = 0.30f * VentanaAlto;

        botonVolver.w = 0.15f * VentanaAncho;
        botonVolver.h = 0.08f * VentanaAlto;
        botonVolver.x = 0.05f * VentanaAncho;
        botonVolver.y = 0.05f * VentanaAlto;

        botonInformacion.w = 0.15f * VentanaAncho;
        botonInformacion.h = 0.08f * VentanaAlto;
        botonInformacion.x = 0.02f * VentanaAncho;
        botonInformacion.y = 0.02f * VentanaAlto;

        botonMapa.w = 0.10f * VentanaAncho;
        botonMapa.h = 0.08f * VentanaAlto;
        botonMapa.x = 0.87f  * VentanaAncho;
        botonMapa.y = 0.02f * VentanaAlto;
    }

    bool check_hover(const BotonImagen& boton, float mouseX, float mouseY) {
        return mouseX >= boton.x && mouseX <= (boton.x + boton.w) &&
               mouseY >= boton.y && mouseY <= (boton.y + boton.h);
    }

    void cursor_pos_callback(double xpos, double ypos) {
        float mouseX = static_cast<float>(xpos);
        float mouseY = static_cast<float>(VentanaAlto - ypos);

        // Se resetean todos los hovers para evitar estados inconsistentes
        botonEmpezar.hovered = botonTrucha.hovered = botonIspi.hovered = botonChicharron.hovered = botonVolver.hovered = false;

        if (estadoActual == EstadoApp::TITULO_INICIO) {
            botonEmpezar.hovered = check_hover(botonEmpezar, mouseX, mouseY);
        }
        else if (estadoActual == EstadoApp::PANTALLA_MENU) {
            botonTrucha.hovered = check_hover(botonTrucha, mouseX, mouseY);
            botonIspi.hovered = check_hover(botonIspi, mouseX, mouseY);
            botonChicharron.hovered = check_hover(botonChicharron, mouseX, mouseY);
            //lineas clave
            botonMapa.hovered = check_hover(botonMapa, mouseX, mouseY);
            botonInformacion.hovered = check_hover(botonInformacion, mouseX, mouseY);
        }
        else if (estadoActual == EstadoApp::PANTALLA_TRUCHA || estadoActual == EstadoApp::PANTALLA_ISPI || estadoActual == EstadoApp::PANTALLA_CHICHARRON || estadoActual == EstadoApp::PANTALLA_INFORMACION) {
            botonVolver.hovered = check_hover(botonVolver, mouseX, mouseY);
        }
        else if (estadoActual == EstadoApp::PANTALLA_MAPA) {
            float mouseX = static_cast<float>(xpos);
            float mouseY = static_cast<float>(VentanaAlto - ypos);

            // Coordenadas del mouse en el sistema del mapa (inversa de la transformación en renderizarMapa)
            // Esto es complejo, una aproximación más simple es transformar los polígonos a coordenadas de pantalla
            // y luego comparar, como haremos aquí:

            // (Re-calcular transformaciones igual que en renderizarMapa)
            float margen = 0.1f;
            float targetAncho = VentanaAncho * (1.0f - 2.0f * margen);
            float targetAlto = VentanaAlto * (1.0f - 2.0f * margen);
            float minX = 1e9, minY = 1e9, maxX = -1e9, maxY = -1e9;
            for (const auto& prov : provincias) {
                if (prov.boundingBox.x < minX) minX = prov.boundingBox.x;
                if (prov.boundingBox.y < minY) minY = prov.boundingBox.y;
                if (prov.boundingBox.z > maxX) maxX = prov.boundingBox.z;
                if (prov.boundingBox.w > maxY) maxY = prov.boundingBox.w;
            }
            float mapaAnchoGeo = maxX - minX;
            float mapaAltoGeo = maxY - minY;
            float escalaX = targetAncho / mapaAnchoGeo;
            float escalaY = targetAlto / mapaAltoGeo;
            float escalaMapa = std::min(escalaX, escalaY);
            float centroMapaGeoX = minX + mapaAnchoGeo / 2.0f;
            float centroMapaGeoY = minY + mapaAltoGeo / 2.0f;
            float centroVentanaX = VentanaAncho / 4.0f;
            float centroVentanaY = VentanaAlto / 2.0f;

            // Revisar cada provincia
            for (auto& prov : provincias) { // Usamos referencia para poder modificar 'hovered'
                std::vector<glm::vec2> poligonoEnPantalla;
                for (const auto& punto : prov.poligono) {
                     float pixelX = centroVentanaX + (punto.x - centroMapaGeoX) * escalaMapa;
                     float pixelY = centroVentanaY + (punto.y - centroMapaGeoY) * escalaMapa;
                     poligonoEnPantalla.push_back({pixelX, pixelY});
                }

                if (estaDentroDelPoligono({mouseX, mouseY}, poligonoEnPantalla)) {
                    prov.hovered = true;
                } else {
                    prov.hovered = false;
                }
            }
            botonVolver.hovered = check_hover(botonVolver, mouseX, mouseY);
        }
    }

    void mouse_button_callback(int button, int action, int mods) {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {

            if (estadoActual == EstadoApp::TITULO_INICIO) {
                if (botonEmpezar.hovered) {
                    estadoActual = EstadoApp::PANTALLA_MENU;
                }
            }
            else if (estadoActual == EstadoApp::PANTALLA_MENU) {
                if (botonTrucha.hovered) estadoActual = EstadoApp::PANTALLA_TRUCHA;
                else if (botonIspi.hovered) estadoActual = EstadoApp::PANTALLA_ISPI;
                else if (botonChicharron.hovered) estadoActual = EstadoApp::PANTALLA_CHICHARRON;
                else if (botonInformacion.hovered) estadoActual = EstadoApp::PANTALLA_INFORMACION;
                else if (botonMapa.hovered) estadoActual = EstadoApp::PANTALLA_MAPA;
            }
            // Lógica para todas las pantallas que tienen un botón "Volver"
            else if (estadoActual == EstadoApp::PANTALLA_TRUCHA ||
                     estadoActual == EstadoApp::PANTALLA_ISPI ||
                     estadoActual == EstadoApp::PANTALLA_CHICHARRON ||
                     estadoActual == EstadoApp::PANTALLA_INFORMACION)
            {
                if (botonVolver.hovered) {
                    estadoActual = EstadoApp::PANTALLA_MENU;
                }
            }
            // Lógica específica y anidada para la PANTALLA_MAPA
            else if (estadoActual == EstadoApp::PANTALLA_MAPA) {
                // Primero, revisamos si se hizo clic en el botón "Volver"
                if (botonVolver.hovered) {
                    estadoActual = EstadoApp::PANTALLA_MENU;
                    provinciaSeleccionada = nullptr; // Limpiamos la selección
                    return; // Salimos para no procesar más clics
                }

                // Si no fue en "Volver", revisamos si se hizo clic en una provincia
                bool provinciaClickeada = false;
                for (auto& prov : provincias) {
                    if (prov.hovered) {
                        provinciaSeleccionada = &prov;
                        provinciaClickeada = true;
                        break;
                    }
                }

                // Si se hizo clic, pero no fue sobre una provincia ni sobre "Volver"
                if (!provinciaClickeada) {
                    provinciaSeleccionada = nullptr; // Limpiamos la selección
                }
            }
        }
    }
};
