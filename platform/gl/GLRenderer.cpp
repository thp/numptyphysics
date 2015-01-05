/*
 * This file is part of NumptyPhysics <http://thp.io/2015/numptyphysics/>
 * Coyright (c) 2014 Thomas Perl <m@thp.io>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 */

#include "GLRenderer.h"

#include "Config.h"

#include <initializer_list>
#include <cstring>
#include <cstdlib>


struct fRect {
    fRect(float x1, float y1, float x2, float y2) : x1(x1), y1(y1), x2(x2), y2(y2) {}
    fRect(const Rect &r) : x1(r.tl.x), y1(r.tl.y), x2(r.br.x), y2(r.br.y) {}

    float x1, y1, x2, y2;
};

struct FloatArray {
    FloatArray(const std::initializer_list<float> &values)
        : data(static_cast<float *>(calloc(values.size(), sizeof(float))))
        , size(values.size() * sizeof(float))
    {
        int i = 0;
        for (auto &v: values) {
            data[i++] = v;
        }
    }

    FloatArray(float *data, size_t size)
        : data(static_cast<float *>(malloc(size)))
        , size(size)
    {
        memcpy(this->data, data, size);
    }

    FloatArray(const FloatArray &other)
        : data(static_cast<float *>(malloc(other.size)))
        , size(other.size)
    {
        memcpy(this->data, data, size);
    }

    FloatArray &operator=(const FloatArray &other)
    {
        free(data);
        this->data = static_cast<float *>(malloc(other.size));
        memcpy(this->data, other.data, other.size);
        this->size = other.size;

        return *this;
    }

    ~FloatArray()
    {
        free(data);
    }

    float *data;
    size_t size;
};

template <typename T>
struct Use {
    Use(const T &v) : v(v) { v->enable(); }
    ~Use() { v->disable(); }

    const T &v;
};

template <typename T, typename C>
void
with(const T &v, C callback)
{
    Use<T> use(v);
    callback(v);
}

class GLRendererPriv {
public:
    GLRendererPriv(int width, int height);
    ~GLRendererPriv();

    void submitTextured(Glaserl::Texture &texture, const FloatArray &data);
    void submitRewind(Glaserl::Texture &texture, const FloatArray &data);
    void submitSaturation(Glaserl::Texture &texture, const FloatArray &data);
    void submitBlur(Glaserl::Texture &texture, const FloatArray &data);
    void submitPath(float *data, size_t size);
    void flush();

    void flipVertically(bool flip);

private:
    void drawTextured();
    void drawPath();

    Glaserl::Matrix projection;

    Glaserl::Program textured_program;
    Glaserl::Buffer textured_buffer;

    Glaserl::Program blur_program;
    Glaserl::Buffer blur_buffer;

    Glaserl::Program path_program;
    Glaserl::Buffer path_buffer;

    Glaserl::Program rewind_program;
    Glaserl::Buffer rewind_buffer;

    Glaserl::Program saturation_program;
    Glaserl::Buffer saturation_buffer;

    enum ProgramType {
        NONE,
        TEXTURED,
        BLUR,
        PATH,
        REWIND,
    };

    enum ProgramType active_program;
    int width;
    int height;
    Rect last_clip;

    friend class GLRenderer;
};

const char *textured_vertex_shader_src =
"attribute vec4 vtxcoord;\n"
"attribute vec2 texcoord;\n"
"uniform mat4 projection;\n"
"varying vec2 tex;\n"
"\n"
"void main() {\n"
"    gl_Position = projection * vtxcoord;\n"
"    tex = texcoord;\n"
"}\n"
;

const char *textured_fragment_shader_src =
"varying vec2 tex;\n"
"uniform sampler2D texture;\n"
"\n"
"void main() {\n"
"    gl_FragColor = texture2D(texture, tex);\n"
"}\n"
;

const char *saturation_fragment_shader_src =
"varying vec2 tex;\n"
"uniform sampler2D texture;\n"
"uniform float alpha;\n"
"\n"
"void main() {\n"
"    vec4 col = texture2D(texture, tex);\n"
"    float g = (col.r + col.g + col.b) / 3.0;\n"
"    gl_FragColor = alpha * col + (1.0 -alpha) * vec4(g, g, g, col.a);\n"
"}\n"
;

const char *blur_vertex_shader_src =
"attribute vec4 vtxcoord;\n"
"attribute vec2 texcoord;\n"
"uniform mat4 projection;\n"
"varying vec2 sample[7];\n"
"uniform vec2 pixelgrid;\n"
"\n"
"void main() {\n"
"    gl_Position = projection * vtxcoord;\n"
"    sample[0] = texcoord;\n"
"    sample[1] = texcoord - 3.0 * pixelgrid;\n"
"    sample[2] = texcoord - 2.0 * pixelgrid;\n"
"    sample[3] = texcoord - 1.0 * pixelgrid;\n"
"    sample[4] = texcoord + 1.0 * pixelgrid;\n"
"    sample[5] = texcoord + 2.0 * pixelgrid;\n"
"    sample[6] = texcoord + 3.0 * pixelgrid;\n"
"}\n"
;

const char *blur_fragment_shader_src =
"varying vec2 sample[7];\n"
"uniform sampler2D texture;\n"
"\n"
"void main() {\n"
"    gl_FragColor.rgb = texture2D(texture, sample[1]).rgb * 0.006 +\n"
"                       texture2D(texture, sample[2]).rgb * 0.061 +\n"
"                       texture2D(texture, sample[3]).rgb * 0.242 +\n"
"                       texture2D(texture, sample[0]).rgb * 0.383 +\n"
"                       texture2D(texture, sample[4]).rgb * 0.242 +\n"
"                       texture2D(texture, sample[5]).rgb * 0.061 +\n"
"                       texture2D(texture, sample[6]).rgb * 0.006;\n"
"    gl_FragColor.a = 1.0;\n"
"}\n"
;

const char *rewind_vertex_shader_src =
"attribute vec4 vtxcoord;\n"
"attribute vec2 texcoord;\n"
"uniform mat4 projection;\n"
"varying vec2 tex;\n"
"varying vec2 vtx;\n"
"\n"
"void main() {\n"
"    gl_Position = projection * vtxcoord;\n"
"    tex = texcoord;\n"
"    vtx = vtxcoord.xy;\n"
"}\n"
;

const char *rewind_fragment_shader_src =
"varying vec2 tex;\n"
"varying vec2 vtx;\n"
"uniform sampler2D texture;\n"
"uniform float time;\n"
"uniform float alpha;\n"
"uniform vec2 texsize;\n"
"\n"
"void main() {\n"
"    vec2 offset = tex;\n"
// Wobbly
"    offset.x += alpha * 0.09 * pow(sin(vtx.y * 0.04 + time * 0.004), 30.0);\n"
// Noise
"    offset.x += alpha * 0.003 * sin((vtx.y * 10000.0 + time * 100.0));\n"
// Don't go offscreen left and right
"    offset.x = max(0.0, min(texsize.x - 0.01, offset.x));\n"
"    gl_FragColor = texture2D(texture, offset);\n"
"}\n"
;

const char *path_vertex_shader_src =
"attribute vec4 vtxcoord;\n"
"attribute vec4 color;\n"
"uniform mat4 projection;\n"
"varying vec4 col;\n"
"\n"
"void main() {\n"
"    gl_Position = projection * vtxcoord;\n"
"    col = color;\n"
"}\n"
;

const char *path_fragment_shader_src =
"varying vec4 col;\n"
"\n"
"void main() {\n"
"    gl_FragColor = col;\n"
"}\n"
;

GLRendererPriv::GLRendererPriv(int width, int height)
    : projection(Glaserl::matrix())
    , textured_program(Glaserl::program(
                textured_vertex_shader_src,
                textured_fragment_shader_src,
                // Attributes
                "vtxcoord", 2,
                "texcoord", 2,
                NULL,
                // Uniforms
                "projection",
                "texture",
                NULL))
    , textured_buffer(Glaserl::buffer())
    , blur_program(Glaserl::program(
                blur_vertex_shader_src,
                blur_fragment_shader_src,
                // Attributes
                "vtxcoord", 2,
                "texcoord", 2,
                NULL,
                // Uniforms
                "projection",
                "texture",
                "pixelgrid",
                NULL))
    , blur_buffer(Glaserl::buffer())
    , path_program(Glaserl::program(
                path_vertex_shader_src,
                path_fragment_shader_src,
                // Attributes
                "vtxcoord", 2,
                "color", 4,
                NULL,
                // Uniforms
                "projection",
                NULL))
    , path_buffer(Glaserl::buffer())
    , rewind_program(Glaserl::program(
                rewind_vertex_shader_src,
                rewind_fragment_shader_src,
                // Attributes
                "vtxcoord", 2,
                "texcoord", 2,
                NULL,
                // Uniforms
                "projection",
                "texture",
                "time",
                "alpha",
                "texsize",
                NULL))
    , rewind_buffer(Glaserl::buffer())
    , saturation_program(Glaserl::program(
                textured_vertex_shader_src,
                saturation_fragment_shader_src,
                // Attributes
                "vtxcoord", 2,
                "texcoord", 2,
                NULL,
                // Uniforms
                "projection",
                "texture",
                "alpha",
                NULL))
    , saturation_buffer(Glaserl::buffer())
    , active_program(NONE)
    , width(width)
    , height(height)
    , last_clip(Rect(0, 0, width, height))
{
    flipVertically(false);
}

GLRendererPriv::~GLRendererPriv()
{
}

void
GLRendererPriv::flipVertically(bool flip)
{
    projection->identity();
    if (flip) {
        projection->ortho(0, width, 0, height, 0, 1);
    } else {
        projection->ortho(0, width, height, 0, 0, 1);
    }

    for (auto p: {textured_program, blur_program, path_program, rewind_program, saturation_program}) {
        Glaserl::Util::load_matrix(p, "projection", projection);
    }
}

void
GLRendererPriv::submitTextured(Glaserl::Texture &texture, const FloatArray &data)
{
    if (active_program != NONE) {
        flush();
    }

    textured_buffer->append(data.data, data.size);

    with(texture, [this] (const Glaserl::Texture &texture) {
        Glaserl::Util::render_triangle_strip(textured_program, textured_buffer);
    });
}

void
GLRendererPriv::submitBlur(Glaserl::Texture &texture, const FloatArray &data)
{
    if (active_program != NONE) {
        flush();
    }

    blur_buffer->append(data.data, data.size);

    with(texture, [this] (const Glaserl::Texture &texture) {
        Glaserl::Util::render_triangle_strip(blur_program, blur_buffer);
    });
}

void
GLRendererPriv::submitRewind(Glaserl::Texture &texture, const FloatArray &data)
{
    if (active_program != NONE) {
        flush();
    }

    rewind_buffer->append(data.data, data.size);

    with(texture, [this] (const Glaserl::Texture &texture) {
        Glaserl::Util::render_triangle_strip(rewind_program, rewind_buffer);
    });
}

void
GLRendererPriv::submitSaturation(Glaserl::Texture &texture, const FloatArray &data)
{
    if (active_program != NONE) {
        flush();
    }

    saturation_buffer->append(data.data, data.size);

    with(texture, [this] (const Glaserl::Texture &texture) {
        Glaserl::Util::render_triangle_strip(saturation_program, saturation_buffer);
    });
}

void
GLRendererPriv::submitPath(float *data, size_t size)
{
    if (active_program != PATH) {
        flush();
    }

    path_buffer->append(data, size);
    active_program = PATH;
}

void
GLRendererPriv::drawPath()
{
    Glaserl::Util::render_triangle_strip(path_program, path_buffer);
    active_program = NONE;
}

void
GLRendererPriv::flush()
{
    if (active_program == PATH) {
        drawPath();
    }
}

GLTextureData::GLTextureData(unsigned char *pixels, int width, int height)
    : NP::TextureData(width, height)
    , texture(Glaserl::texture(pixels, width, height))
{
}

GLTextureData::GLTextureData(Glaserl::Texture texture)
    : NP::TextureData(texture->width(), texture->height())
    , texture(texture)
{
}

GLTextureData::~GLTextureData()
{
}

GLFramebufferData::GLFramebufferData(int w, int h)
    : NP::FramebufferData(w, h)
    , framebuffer(Glaserl::framebuffer(w, h))
{
}

GLFramebufferData::~GLFramebufferData()
{
}

GLRenderer::GLRenderer(int w, int h)
    : m_width(w)
    , m_height(h)
    , priv(NULL)
{
}

GLRenderer::~GLRenderer()
{
    delete priv;
}

void
GLRenderer::init()
{
    priv = new GLRendererPriv(m_width, m_height);
    glClearColor(1.f, 1.f, 1.f, 1.f);
    Glaserl::Util::default_blend();
    Glaserl::Util::enable_scissor();
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

NP::Framebuffer
GLRenderer::framebuffer(int width, int height)
{
    return NP::Framebuffer(new GLFramebufferData(width, height));
}

void
GLRenderer::begin(NP::Framebuffer &rendertarget)
{
    GLFramebufferData *data = static_cast<GLFramebufferData *>(rendertarget.get());
    data->framebuffer->enable();
    priv->flipVertically(true);
}

void
GLRenderer::end(NP::Framebuffer &rendertarget)
{
    GLFramebufferData *data = static_cast<GLFramebufferData *>(rendertarget.get());
    data->framebuffer->disable();
    priv->flipVertically(false);
}

NP::Texture
GLRenderer::retrieve(NP::Framebuffer &rendertarget)
{
    GLFramebufferData *data = static_cast<GLFramebufferData *>(rendertarget.get());
    return NP::Texture(new GLTextureData(data->framebuffer->texture));
}

Rect
GLRenderer::clip(Rect rect)
{
    // TODO: Map rect.tl and rect.br using priv->pojection, then use set_scissor
    Glaserl::Util::set_scissor(rect.tl.x, m_height - rect.tl.y - rect.h(),
                               rect.w(), rect.h());

    std::swap(rect, priv->last_clip);
    return rect;
}

void
GLRenderer::image(const NP::Texture &texture, int x, int y, int w, int h)
{
    GLTextureData *data = static_cast<GLTextureData *>(texture.get());

    Rect src(0, 0, data->texture->width(), data->texture->height());
    Rect dst(x, y, x+w, y+h);

    subimage(texture, src, dst);
}

static fRect
mapTexture(const NP::Texture &texture, const Rect &src)
{
    GLTextureData *data = static_cast<GLTextureData *>(texture.get());

    float w = data->texture->width();
    float h = data->texture->height();

    float tx1 = float(src.tl.x) / w;
    float ty1 = float(src.tl.y) / h;
    data->texture->map_uv(tx1, ty1);

    float tx2 = float(src.br.x) / w;
    float ty2 = float(src.br.y) / h;
    data->texture->map_uv(tx2, ty2);

    return fRect(tx1, ty1, tx2, ty2);
}

static FloatArray
vtxtex(const fRect &vtx, const fRect &tex)
{
    return FloatArray({
        vtx.x1, vtx.y1, tex.x1, tex.y1,
        vtx.x1, vtx.y2, tex.x1, tex.y2,
        vtx.x2, vtx.y1, tex.x2, tex.y1,
        vtx.x2, vtx.y2, tex.x2, tex.y2,
    });
}

void
GLRenderer::subimage(const NP::Texture &texture, const Rect &src, const Rect &dst)
{
    GLTextureData *data = static_cast<GLTextureData *>(texture.get());

    priv->submitTextured(data->texture, vtxtex(dst, mapTexture(texture, src)));
}

void
GLRenderer::blur(const NP::Texture &texture, const Rect &src, const Rect &dst, float rx, float ry)
{
    GLTextureData *data = static_cast<GLTextureData *>(texture.get());

    rx /= data->texture->width();
    ry /= data->texture->height();
    data->texture->map_uv(rx, ry);

    with(priv->blur_program, [rx, ry] (const Glaserl::Program &program) {
        glUniform2f(program->uniform_location("pixelgrid"), rx, ry);
    });

    priv->submitBlur(data->texture, vtxtex(dst, mapTexture(texture, src)));
}

void
GLRenderer::rewind(const NP::Texture &texture, const Rect &src, const Rect &dst, float t, float a)
{
    GLTextureData *data = static_cast<GLTextureData *>(texture.get());

    float tx = 1.f;
    float ty = 1.f;
    data->texture->map_uv(tx, ty);

    with(priv->rewind_program, [t, a, tx, ty] (const Glaserl::Program &program) {
        glUniform1f(program->uniform_location("time"), t);
        glUniform1f(program->uniform_location("alpha"), a);
        glUniform2f(program->uniform_location("texsize"), tx, ty);
    });

    priv->submitRewind(data->texture, vtxtex(dst, mapTexture(texture, src)));
}

void
GLRenderer::saturation(const NP::Texture &texture, const Rect &src, const Rect &dst, float a)
{
    GLTextureData *data = static_cast<GLTextureData *>(texture.get());

    with(priv->saturation_program, [a] (const Glaserl::Program &program) {
        glUniform1f(program->uniform_location("alpha"), a);
    });

    priv->submitSaturation(data->texture, vtxtex(dst, mapTexture(texture, src)));
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
    if (!fill) {
        Vec2 corners[] = { rect.tl, rect.tr(), rect.br, rect.bl(), rect.tl };
        Path p(5, corners);
        path(p, rgba);
        return;
    }

    float r, g, b, a;
    rgba_split(rgba, r, g, b, a);

    float vtxcoords[] = {
        (float)rect.tl.x, (float)rect.tl.y, r, g, b, a,
        (float)rect.tl.x, (float)rect.tl.y, r, g, b, a,
        (float)rect.tl.x, (float)rect.br.y, r, g, b, a,
        (float)rect.br.x, (float)rect.tl.y, r, g, b, a,
        (float)rect.br.x, (float)rect.br.y, r, g, b, a,
        (float)rect.br.x, (float)rect.br.y, r, g, b, a,
    };
    priv->submitPath(vtxcoords, sizeof(vtxcoords));
}

static float *
make_segment(b2Vec2 aa, b2Vec2 a, b2Vec2 b, b2Vec2 bb)
{
    static b2Vec2 data[10];

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

    if (path.numPoints() < 2) {
        return;
    }

    int segments = path.numPoints() - 1;
    int points_len = (2 + 4) * 10 * segments;
    float *points = new float[points_len];
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
        for (int j=0; j<10; j++) {
            // Vertices
            points[offset++] = segment[soffset++];
            points[offset++] = segment[soffset++];

            // Color
            points[offset++] = r;
            points[offset++] = g;
            points[offset++] = b;
            if ((j < 3 || j > 6)) {
                // Transparent
                points[offset++] = 0.f;
            } else {
                // Filled
                points[offset++] = a;
            }
        }
    }
    priv->submitPath(points, points_len * sizeof(float));
    delete points;
}

void
GLRenderer::clear()
{
    glClear(GL_COLOR_BUFFER_BIT);
}

void
GLRenderer::flush()
{
    priv->flush();
}
