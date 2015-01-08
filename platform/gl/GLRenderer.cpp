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

#include "vector_math.h"

#include "petals_log.h"

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
    GLRendererPriv(Vec2 world_size);
    ~GLRendererPriv();

    void submitTextured(Glaserl::Texture &texture, const FloatArray &data);
    void submitRewind(Glaserl::Texture &texture, const FloatArray &data);
    void submitSaturation(Glaserl::Texture &texture, const FloatArray &data);
    void submitBlur(Glaserl::Texture &texture, const FloatArray &data);
    void submitPath(float *data, size_t size);
    void flush();

    void setupProjection(Rect world_rect, Vec2 framebuffer_size, bool offscreen);

private:
    void drawTextured();
    void drawPath();

    vmath::mat4<float> projection;

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

    Vec2 world_size;
    Vec2 framebuffer_size;
    Vec2 framebuffer_target_size;
    bool rotate90;

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

GLRendererPriv::GLRendererPriv(Vec2 world_size)
    : projection()
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
    , world_size(world_size)
    , framebuffer_size(world_size)
    , framebuffer_target_size(framebuffer_size)
    , rotate90(framebuffer_size.x < framebuffer_size.y)
    , last_clip(Rect(Vec2(0, 0), world_size))
{
    setupProjection(Rect(Vec2(0, 0), world_size), framebuffer_size, false);
}

GLRendererPriv::~GLRendererPriv()
{
}

void
GLRendererPriv::setupProjection(Rect world_rect, Vec2 framebuffer_size, bool offscreen)
{
    framebuffer_target_size = framebuffer_size;

    projection = vmath::identity4<float>();

    // Only rotate by 90 degrees if we render directly to the screen
    if (framebuffer_target_size.x < framebuffer_target_size.y && !offscreen) {
        projection *= vmath::rotation_matrix<float>(90.f, vmath::vec3<float>(0.f, 0.f, -1.f));
        std::swap(framebuffer_size.x, framebuffer_size.y);
    }

    // TODO: Zoom and center WORLD_WIDTH, WORLD_HEIGHT into available space
    if (offscreen) {
        projection *= vmath::ortho_matrix<float>(0, framebuffer_size.x, 0, framebuffer_size.y, 0, 1);
    } else {
        projection *= vmath::ortho_matrix<float>(0, framebuffer_size.x, framebuffer_size.y, 0, 0, 1);
    }

    if (!offscreen) {
        float w = framebuffer_size.x, W = world_rect.w(), wscale = w / W;
        float h = framebuffer_size.y, H = world_rect.h(), hscale = h / H;

        float scale = std::min(wscale, hscale);
        float xoffset = (w - W * scale) / 2.f;
        float yoffset = (h - H * scale) / 2.f;

        projection *= vmath::translation_matrix<float>(xoffset, yoffset, 0.f);
        projection *= vmath::scaling_matrix<float>(scale, scale, 1.f);
    }

    auto m = vmath::transpose(projection);
    for (auto p: {textured_program, blur_program, path_program, rewind_program, saturation_program}) {
        with (p, [&m] (const Glaserl::Program &program) {
            glUniformMatrix4fv(program->uniform_location("projection"), 1, GL_FALSE, m);
        });
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

GLRenderer::GLRenderer(Vec2 world_size)
    : _world_size(world_size)
    , priv(nullptr)
{
}

GLRenderer::~GLRenderer()
{
    delete priv;
}

void
GLRenderer::init(Vec2 framebuffer_size)
{
    priv = new GLRendererPriv(_world_size);
    glClearColor(0.f, 0.f, 0.f, 1.f);
    Glaserl::Util::default_blend();
    Glaserl::Util::enable_scissor(true);
    priv->framebuffer_size = framebuffer_size;
    priv->rotate90 = (priv->framebuffer_size.x < priv->framebuffer_size.y);
}

Vec2
GLRenderer::framebuffer_size()
{
    if (priv->rotate90) {
        return Vec2(priv->framebuffer_size.y, priv->framebuffer_size.x);
    }

    return priv->framebuffer_size;
}

Vec2
GLRenderer::world_size()
{
    return priv->world_size;
}

void
GLRenderer::mapXY(int &x, int &y)
{
    auto fbsize = vmath::vec2<float>(priv->framebuffer_size.x, priv->framebuffer_size.y);

    auto pos = (vmath::vec2<float>(x, y) / fbsize) * 2.f - vmath::vec2<float>(1.0f, 1.0f);
    pos.y *= -1.f;

    vmath::vec3<float> pos3(pos, 0.f);
    pos3 = vmath::transform_point(vmath::inverse(priv->projection), pos3);

    x = pos3.x;
    y = pos3.y;
}

void
GLRenderer::projectXY(int &x, int &y)
{
    auto fbsize = vmath::vec2<float>(priv->framebuffer_target_size.x, priv->framebuffer_target_size.y);

    auto pos3 = vmath::transform_point(priv->projection, vmath::vec3<float>(x, y, 0.f));

    auto pos = (vmath::vec2<float>(pos3.x, pos3.y) / 2.f + vmath::vec2<float>(0.5f, 0.5f)) * fbsize;

    x = pos.x;
    y = pos.y;
}

NP::Texture
GLRenderer::load(unsigned char *pixels, int w, int h)
{
    return NP::Texture(new GLTextureData(pixels, w, h));
}

NP::Framebuffer
GLRenderer::framebuffer(Vec2 size)
{
    return NP::Framebuffer(new GLFramebufferData(size.x, size.y));
}

void
GLRenderer::begin(NP::Framebuffer &rendertarget, Rect world_rect)
{
    GLFramebufferData *data = static_cast<GLFramebufferData *>(rendertarget.get());
    data->framebuffer->enable();
    priv->setupProjection(world_rect, Vec2(data->w, data->h), true);
}

void
GLRenderer::end(NP::Framebuffer &rendertarget)
{
    GLFramebufferData *data = static_cast<GLFramebufferData *>(rendertarget.get());
    data->framebuffer->disable();
    priv->setupProjection(Rect(Vec2(0, 0), priv->world_size), priv->framebuffer_size, false);
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
    int x1 = rect.tl.x;
    int y1 = rect.tl.y;
    int x2 = rect.br.x;
    int y2 = rect.br.y;

    projectXY(x1, y1);
    projectXY(x2, y2);

    if (x1 > x2) {
        std::swap(x1, x2);
    }

    if (y1 > y2) {
        std::swap(y1, y2);
    }

    int w = x2 - x1;
    int h = y2 - y1;

    Glaserl::Util::set_scissor(x1, y1, w, h);

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
    Glaserl::Util::enable_scissor(false);
    glClear(GL_COLOR_BUFFER_BIT);
    Glaserl::Util::enable_scissor(true);
}

void
GLRenderer::flush()
{
    priv->flush();
}
