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
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, data->m_texture);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
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
    glEnableClientState(GL_VERTEX_ARRAY);
    glDisable(GL_TEXTURE_2D);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glDisableClientState(GL_VERTEX_ARRAY);
    // TODO: Handle "fill", draw lines if not fill
}

static b2Vec2 operator*(b2Vec2 v, float m)
{
    b2Vec2 x = v;
    x *= m;
    return x;
}

static float *
make_segment(b2Vec2 aa, b2Vec2 a, b2Vec2 b, b2Vec2 bb)
{
    static b2Vec2 data[9];

    b2Vec2 a_to_b = 0.5 * (b - a) + 0.5 * (a - aa);
    a_to_b.Normalize();
    b2Vec2 a_to_b_90(-a_to_b.y, a_to_b.x);

    b2Vec2 b_to_a = 0.5 * (bb - b) + 0.5 * (b - a);
    b_to_a.Normalize();
    b2Vec2 b_to_a_90(-b_to_a.y, b_to_a.x);

    float w = 0.9;
    float e = 1.0;

    int offset = 0;
    data[offset++] = a + a_to_b_90 * (w + e);
    data[offset++] = b + b_to_a_90 * (w + e);
    data[offset++] = a + a_to_b_90 * w;
    data[offset++] = b + b_to_a_90 * w;

    data[offset++] = a - a_to_b_90 * w;
    data[offset++] = b - b_to_a_90 * w;
    data[offset++] = a - a_to_b_90 * (w + e);
    data[offset++] = b - b_to_a_90 * (w + e);

    data[offset++] = b - b_to_a_90 * (w + e);

    return &(data[0].x);
}

void
GLRenderer::path(const Path &path, int rgba)
{
    float r, g, b, a;
    rgba_split(rgba, r, g, b, a);
    glColor4f(r, g, b, a);
    int segments = path.numPoints() - 1;

    float *points = new float[(2 + 4) * 9 * segments];
    int offset = 0;
    for (int i=0; i<segments; i++) {
        // Segment P1 -> P2
        float *segment = make_segment(
              (i > 0) ? path[i-1] : path[i],
              path[i],
              path[i+1],
              (i < segments - 1) ? path[i+2] : path[i+1]
        );

        int soffset = 0;
        for (int j=0; j<9; j++) {
            // Vertices
            points[offset++] = segment[soffset++];
            points[offset++] = segment[soffset++];

            // Color
            points[offset++] = r;
            points[offset++] = g;
            points[offset++] = b;
            if ((j < 2 || j > 5)) {
                // Transparent
                points[offset++] = 0.f;
            } else {
                // Filled
                points[offset++] = 1.f;
            }
        }
    }

    glVertexPointer(2, GL_FLOAT, sizeof(float) * (2 + 4), points);
    glColorPointer(4, GL_FLOAT, sizeof(float) * (2 + 4), points + 2);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glDisable(GL_TEXTURE_2D);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 9 * (path.numPoints() - 1));
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

    delete points;
}

void
GLRenderer::clear()
{
    glClear(GL_COLOR_BUFFER_BIT);
}
