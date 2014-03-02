#include "GLRenderer.h"

#include <GL/gl.h>

GLTextureData::GLTextureData(unsigned char *pixels, int width, int height)
    : NP::TextureData(width, height)
    , m_texture(0)
    , m_subwidth(1.f)
    , m_subheight(1.f)
{
    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    int w = 1;
    while (w < width) w *= 2;
    int h = 1;
    while (h < height) h *= 2;

    m_subwidth = (float)width / (float)w;
    m_subheight = (float)height / (float)h;

    unsigned char *blackness = new unsigned char[w*h*4];
    memset(blackness, 0, w*h*4);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA,
            GL_UNSIGNED_BYTE, blackness);
    delete [] blackness;

    if (pixels) {
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height,
                GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    }

    glBindTexture(GL_TEXTURE_2D, 0);
}

GLTextureData::~GLTextureData()
{
    glDeleteTextures(1, &m_texture);
}


GLRenderer::GLRenderer(int w, int h)
    : m_width(w)
    , m_height(h)
{
}

GLRenderer::~GLRenderer()
{
}

void
GLRenderer::init()
{
    glClearColor(1.f, 1.f, 1.f, 1.f);
    glOrtho(0, m_width, m_height, 0, 0, 1);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void
GLRenderer::size(int *width, int *height)
{
    *width = m_width;
    *height = m_height;
}

NP::Texture
GLRenderer::load(unsigned char *pixels, int w, int h)
{
    return NP::Texture(new GLTextureData(pixels, w, h));
}

void
GLRenderer::image(const NP::Texture &texture, int x, int y)
{
    GLTextureData *data = static_cast<GLTextureData *>(texture.get());

    float texcoords[] = {
        0, 0,
        0, data->m_subheight,
        data->m_subwidth, 0,
        data->m_subwidth, data->m_subheight,
    };

    float vtxcoords[] = {
        x, y,
        x, y + data->h,
        x + data->w, y,
        x + data->w, y + data->h,
    };

    glColor4f(1.f, 1.f, 1.f, 1.f);
    glVertexPointer(2, GL_FLOAT, 0, vtxcoords);
    glTexCoordPointer(2, GL_FLOAT, 0, texcoords);
    glEnable(GL_VERTEX_ARRAY);
    glEnable(GL_TEXTURE_COORD_ARRAY);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, data->m_texture);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_COORD_ARRAY);
    glDisable(GL_VERTEX_ARRAY);
}

static void rgba_split(int rgba, float &r, float &g, float &b, float &a)
{
    a = ((rgba & 0xff000000) >> 24) / 255.;
    r = ((rgba & 0x00ff0000) >> 16) / 255.;
    g = ((rgba & 0x0000ff00) >> 8) / 255.;
    b = ((rgba & 0x000000ff)) / 255.;
}

void
GLRenderer::rectangle(const Rect &rect, int rgba, bool fill)
{
    float r, g, b, a;
    rgba_split(rgba, r, g, b, a);
    glColor4f(r, g, b, a);

    float vtxcoords[] = {
        rect.tl.x, rect.tl.y,
        rect.tl.x, rect.br.y,
        rect.br.x, rect.tl.y,
        rect.br.x, rect.br.y,
    };

    glVertexPointer(2, GL_FLOAT, 0, vtxcoords);
    glEnable(GL_VERTEX_ARRAY);
    glDisable(GL_TEXTURE_2D);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glDisable(GL_VERTEX_ARRAY);
    // TODO: Handle "fill", draw lines if not fill
}

void
GLRenderer::path(const Path &path, int rgba)
{
    float r, g, b, a;
    rgba_split(rgba, r, g, b, a);
    glColor4f(r, g, b, a);

    float *points = new float[2 * path.numPoints()];
    for (int i=0; i<path.numPoints(); i++) {
        points[i*2 + 0] = path[i].x;
        points[i*2 + 1] = path[i].y;
    }
    // TODO: make antialiased lines
    glVertexPointer(2, GL_FLOAT, 0, points);
    glEnable(GL_VERTEX_ARRAY);
    glDisable(GL_TEXTURE_2D);
    glDrawArrays(GL_LINE_STRIP, 0, path.numPoints());
    glDisable(GL_VERTEX_ARRAY);

    delete points;
}

void
GLRenderer::clear()
{
    glClear(GL_COLOR_BUFFER_BIT);
}
