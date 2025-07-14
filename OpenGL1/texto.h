#ifndef TEXTO_H
#define TEXTO_H

#include <windows.h>
#include <gl/GL.h>
#include <cstring>

class Texto {
private:
    GLuint base;
    HDC hdc;
    HFONT fuenteOriginal;
    HFONT fuenteNueva;
    std::string rutaFuente;
    bool creado;

public:
    Texto() : base(0), hdc(nullptr), fuenteOriginal(nullptr), fuenteNueva(nullptr), creado(false) {}

    // Inicializa la fuente TTF desde archivo
    bool inicializar(HDC contextoDC, const char* rutaTTF, int tamanio = 24) {
        hdc = contextoDC;
        base = glGenLists(96);
        rutaFuente = rutaTTF;

        // Carga la fuente TTF temporalmente
        if (!AddFontResourceExA(rutaTTF, FR_PRIVATE, nullptr))
            return false;

        fuenteOriginal = (HFONT)GetStockObject(SYSTEM_FONT);

        fuenteNueva = CreateFontA(
            tamanio, 0, 0, 0,
            FW_NORMAL, FALSE, FALSE, FALSE,
            ANSI_CHARSET,
            OUT_TT_PRECIS,
            CLIP_DEFAULT_PRECIS,
            ANTIALIASED_QUALITY,
            FF_DONTCARE | DEFAULT_PITCH,
            NULL // NULL para usar la fuente recién cargada
        );

        if (!fuenteNueva)
            return false;

        SelectObject(hdc, fuenteNueva);
        wglUseFontBitmapsA(hdc, 32, 96, base);

        creado = true;
        return true;
    }

    void imprimir(float x, float y, const char* texto, float r = 0, float g = 0, float b = 0) {
        if (!creado) return;
        glColor3f(r, g, b);
        glRasterPos2f(x, y);
        glPushAttrib(GL_LIST_BIT);
        glListBase(base - 32);
        glCallLists(static_cast<GLsizei>(strlen(texto)), GL_UNSIGNED_BYTE, texto);
        glPopAttrib();
    }

    void liberar() {
        if (creado) {
            glDeleteLists(base, 96);
            if (fuenteNueva) DeleteObject(fuenteNueva);
            if (!rutaFuente.empty())
                RemoveFontResourceExA(rutaFuente.c_str(), FR_PRIVATE, nullptr);
            creado = false;
        }
    }
};

#endif
