// CLASE: Aplicacion
// Responsabilidad: Orquestar toda la aplicación.
// ===================================================================
class Aplicacion {
public:
    int VentanaAncho, VentanaAlto;

    Aplicacion(int ancho, int alto, const char* titulo) {
        VentanaAncho = ancho;
        VentanaAlto = alto;
        buttonHovered = false;
        estadoActual = EstadoApp::TITULO_MENU; // Estado inicial

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
        glfwSetWindowUserPointer(ventana, this); // Apuntar a esta instancia

        // Configurar callbacks
        glfwSetFramebufferSizeCallback(ventana, [](GLFWwindow* w, int width, int height){
            static_cast<Aplicacion*>(glfwGetWindowUserPointer(w))->framebuffer_size_callback(width, height);
        });
        glfwSetCursorPosCallback(ventana, [](GLFWwindow* w, double x, double y){
            static_cast<Aplicacion*>(glfwGetWindowUserPointer(w))->cursor_pos_callback(x, y);
        });
        // NUEVO: Callback de click del mouse
        glfwSetMouseButtonCallback(ventana, [](GLFWwindow* w, int button, int action, int mods){
            static_cast<Aplicacion*>(glfwGetWindowUserPointer(w))->mouse_button_callback(button, action, mods);
        });

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            std::cerr << "Fallo al inicializar GLAD" << std::endl;
            return;
        }

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        inicializarRecursos();
        framebuffer_size_callback(VentanaAncho, VentanaAlto); // Llamada inicial
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
        delete texto;
        glDeleteVertexArrays(1, &VAO_generico);
        glDeleteBuffers(1, &VBO_generico);
        glfwTerminate();
    }

private:
    GLFWwindow* ventana;
    Shader* shaderImagen;
    Shader* shaderBoton;
    Textura* texChicharron;
    Textura* texBandera;
    Textura* texSistemas;
    RenderizadorTexto* texto;
    unsigned int VAO_generico, VBO_generico;
    float projection[16];
    float buttonX, buttonY, buttonWidth, buttonHeight;
    bool buttonHovered;
    EstadoApp estadoActual; // Variable de estado

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

        // --- Texturas ---
        texChicharron = new Textura("PNG/Chicharron.png");
        texBandera = new Textura("PNG/bandera_Puno.png");
        texSistemas = new Textura("PNG/sistemas1.png");

        // --- Texto ---
        // Verificación de existencia del archivo antes de cargar
        if (!std::filesystem::exists("font/LondrinaSolid-Regular.ttf")) {
            std::cerr << "Error: No se encontró la fuente en 'font/LondrinaSolid-Regular.ttf'\n";
            // Aquí podrías manejar el error de forma más robusta,
            // por ejemplo, usando una fuente por defecto o saliendo.
            exit(EXIT_FAILURE);
        }
        texto = new RenderizadorTexto("font/LondrinaSolid-Regular.ttf", 85);

        // --- Geometría ---
        glGenVertexArrays(1, &VAO_generico);
        glGenBuffers(1, &VBO_generico);
        glBindVertexArray(VAO_generico);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_generico);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
        // Posición (vec2) y texCoords (vec2)
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glBindVertexArray(0);
    }

    void renderizar() {
        glClearColor(1.0f, 0.95f, 0.88f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        float negro[] = { 0.0f, 0.0f, 0.0f };
        float blanco[] = { 1.0f, 1.0f, 1.0f };
        float azul[] = { 0.0f, 0.0f, 1.0f }; // Color para la segunda pantalla

        // Usamos la variable de estado para decidir qué dibujar
        switch (estadoActual) {
            case EstadoApp::TITULO_MENU: {
                // --- Renderizar texto del título ---
                float escalaTextoTitulo = VentanaAlto / 600.0f;
                texto->render("Un Viaje", 0.1f * VentanaAncho, VentanaAlto - 215 * escalaTextoTitulo, escalaTextoTitulo, negro, projection);
                texto->render("Culinario", 0.1f * VentanaAncho, VentanaAlto - 300 * escalaTextoTitulo, escalaTextoTitulo, negro, projection);
                texto->render("Interactivo", 0.1f * VentanaAncho, VentanaAlto - 385 * escalaTextoTitulo, escalaTextoTitulo, negro, projection);

                // --- Renderizar imágenes ---
                shaderImagen->usar();
                shaderImagen->setMat4("projection", projection);
                glBindVertexArray(VAO_generico);
                // Chicharrón
                renderizarQuad(texChicharron, 0.45f * VentanaAncho, 0.3f * VentanaAlto, 0.5f * VentanaAncho, 0.5f * VentanaAlto);
                // Bandera
                renderizarQuad(texBandera, 0.02f * VentanaAncho, 0.83f * VentanaAlto, 0.15f * VentanaAncho, 0.15f * VentanaAlto);
                // Sistemas
                renderizarQuad(texSistemas, 0.86f * VentanaAncho, 0.82f * VentanaAlto, 0.12f * VentanaAncho, 0.15f * VentanaAlto);

                // --- Renderizar Botón ---
                shaderBoton->usar();
                shaderBoton->setMat4("projection", projection);
                if (buttonHovered) {
                    shaderBoton->setVec3("buttonColor", (217.0f / 255.0f) * 0.85f, (108.0f / 255.0f) * 0.85f, (59.0f / 255.0f) * 0.85f);
                } else {
                    shaderBoton->setVec3("buttonColor", 217.0f / 255.0f, 108.0f / 255.0f, 59.0f / 255.0f);
                }
                shaderBoton->setVec4("buttonRect", buttonX, buttonY, buttonWidth, buttonHeight);
                shaderBoton->setFloat("borderRadius", 20.0f * (VentanaAlto / 1024.0f));
                glEnableVertexAttribArray(0); // Asegurarse de que esté habilitado para el botón
                glDisableVertexAttribArray(1); // Deshabilitar texCoords para el botón
                renderizarQuad(nullptr, buttonX, buttonY, buttonWidth, buttonHeight);
                glEnableVertexAttribArray(1); // Reactivar texCoords para futuras imágenes

                // --- Renderizar texto del botón ---
                std::string textoBoton = "Empezar";
                float escalaTextoBoton = VentanaAlto / 1100.0f;
                float anchoTextoBoton = texto->getTextWidth(textoBoton, escalaTextoBoton);
                float xTexto = buttonX + (buttonWidth - anchoTextoBoton) / 2.0f;
                float yTexto = buttonY + (buttonHeight / 2.0f) + (-25.0f * escalaTextoBoton);
                texto->render(textoBoton, xTexto, yTexto, escalaTextoBoton, blanco, projection);
                break;
            }
            case EstadoApp::PANTALLA_JUEGO: {
                // Aquí dibujamos el contenido de la "nueva ventana" o "pantalla de juego"
                glClearColor(0.7f, 0.8f, 1.0f, 1.0f); // Un color de fondo diferente (azul claro)
                glClear(GL_COLOR_BUFFER_BIT);

                std::string mensajeJuego = "¡Bienvenido al Juego!";
                float escalaMensaje = VentanaAlto / 400.0f;
                float anchoMensaje = texto->getTextWidth(mensajeJuego, escalaMensaje);
                float xMensaje = (VentanaAncho - anchoMensaje) / 2.0f;
                float yMensaje = VentanaAlto / 2.0f;
                texto->render(mensajeJuego, xMensaje, yMensaje, escalaMensaje, azul, projection);

                // Podrías añadir un botón de "Volver al menú" aquí si lo deseas
                // O cualquier otro elemento de la pantalla de juego
                break;
            }
        }
    }

    void renderizarQuad(Textura* textura, float x, float y, float w, float h) {
        if (textura) {
            textura->bind();
        }
        float vertices[] = {
            // pos      // tex
            x,      y,      0.0f, 0.0f,
            x,      y + h, 0.0f, 1.0f,
            x + w, y + h, 1.0f, 1.0f,
            x,      y,      0.0f, 0.0f,
            x + w, y + h, 1.0f, 1.0f,
            x + w, y,      1.0f, 0.0f
        };
        glBindBuffer(GL_ARRAY_BUFFER, VBO_generico);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    void ortho(float left, float right, float bottom, float top, float* m) {
        float near_val = -1.0f, far_val = 1.0f;
        for (int i = 0; i < 16; i++) m[i] = 0;
        m[0] = 2.0f / (right - left);
        m[5] = 2.0f / (top - bottom);
        m[10] = -2.0f / (far_val - near_val);
        m[12] = -(right + left) / (right - left);
        m[13] = -(top + bottom) / (top - bottom);
        m[14] = -(far_val + near_val) / (far_val - near_val);
        m[15] = 1.0f;
    }

    void framebuffer_size_callback(int width, int height) {
        VentanaAncho = width;
        VentanaAlto = height;
        glViewport(0, 0, width, height);
        ortho(0, (float)width, 0, (float)height, projection);
        // Recalcular el tamaño del botón para responsividad
        buttonWidth = 0.25f * VentanaAncho;
        buttonHeight = 0.1f * VentanaAlto;
        buttonX = (VentanaAncho - buttonWidth) / 2.0f;
        buttonY = 0.10f * VentanaAlto;
    }

    void cursor_pos_callback(double xpos, double ypos) {
        float mouseX = static_cast<float>(xpos);
        float mouseY = static_cast<float>(VentanaAlto - ypos); // Invertir Y para OpenGL

        if (estadoActual == EstadoApp::TITULO_MENU) { // Solo si estamos en el menú
            if (mouseX >= buttonX && mouseX <= (buttonX + buttonWidth) &&
                mouseY >= buttonY && mouseY <= (buttonY + buttonHeight)) {
                buttonHovered = true;
            } else {
                buttonHovered = false;
            }
        } else {
            buttonHovered = false; // Deshabilitar hover si no estamos en el menú
        }
    }

    void mouse_button_callback(int button, int action, int mods) {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
            double xpos, ypos;
            glfwGetCursorPos(ventana, &xpos, &ypos);
            float mouseX = static_cast<float>(xpos);
            float mouseY = static_cast<float>(VentanaAlto - ypos); // Invertir Y

            if (estadoActual == EstadoApp::TITULO_MENU) {
                // Si el clic fue en el botón "Empezar"
                if (mouseX >= buttonX && mouseX <= (buttonX + buttonWidth) &&
                    mouseY >= buttonY && mouseY <= (buttonY + buttonHeight)) {
                    estadoActual = EstadoApp::PANTALLA_JUEGO; // ¡Cambiar al estado de juego!
                    // Si quieres, podrías resetear la posición del mouse o el estado de hover aquí
                    buttonHovered = false;
                }
            }
            // Podrías añadir lógica para clics en la pantalla de juego si es necesario
        }
    }
};
