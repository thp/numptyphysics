#include "GLRenderer.h"

class GLRendererPriv {
public:
    GLRendererPriv(int width, int height);
    ~GLRendererPriv();

    void submitTextured(Glaserl::Texture &texture, float *data, size_t size);
    void submitPath(float *data, size_t size);
    void flush();

private:
    void drawTextured();
    void drawPath();

    Glaserl::Matrix projection;

    Glaserl::Program textured_program;
    Glaserl::Buffer textured_buffer;

    Glaserl::Program path_program;
    Glaserl::Buffer path_buffer;

    enum ProgramType {
        NONE,
        TEXTURED,
        PATH,
    };

    enum ProgramType active_program;
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
    , active_program(NONE)
{
    projection->ortho(0, width, height, 0, 0, 1);

    textured_program->enable();
    // Projection
    glUniformMatrix4fv(textured_program->uniform(0), 1, GL_FALSE, projection->data());
    // Use Texture Unit 0 for textured_program
    glUniform1i(textured_program->uniform(1), 0);
    textured_program->disable();

    path_program->enable();
    // Projection
    glUniformMatrix4fv(path_program->uniform(0), 1, GL_FALSE, projection->data());
    path_program->disable();
}

GLRendererPriv::~GLRendererPriv()
{
}

// Scoped usage of a Glaserl::Program together with a Glaserl::Buffer
class ProgramUsage {
public:
    ProgramUsage(Glaserl::Program program, Glaserl::Buffer buffer)
        : m_program(program)
        , m_buffer(buffer)
        , m_size(0)
    {
        m_size = m_buffer->enable();
        m_program->enable();
        m_buffer->disable();
    }

    ~ProgramUsage()
    {
        m_program->disable();
    }

    size_t elements() { return m_size / m_program->stride(); }

private:
    Glaserl::Program m_program;
    Glaserl::Buffer m_buffer;
    size_t m_size;
};


void
GLRendererPriv::submitTextured(Glaserl::Texture &texture, float *data, size_t size)
{
    if (active_program != NONE) {
        flush();
    }

    textured_buffer->append(data, size);

    ProgramUsage usage(textured_program, textured_buffer);
    texture->enable();
    glDrawArrays(GL_TRIANGLE_STRIP, 0, usage.elements());
    texture->disable();
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
    ProgramUsage usage(path_program, path_buffer);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, usage.elements());

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

GLTextureData::~GLTextureData()
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

    float tx1 = 0.f, ty1 = 0.f;
    data->texture->map_uv(tx1, ty1);

    float tx2 = 1.f, ty2 = 1.f;
    data->texture->map_uv(tx2, ty2);

    // Layout: vtx.x, vtx.y, tex.x, tex.y
    float vtxdata[] = {
        (float)x, (float)y, tx1, ty1,
        (float)x, (float)(y + data->h), tx1, ty2,
        (float)(x + data->w), (float)y, tx2, tx1,
        (float)(x + data->w), (float)(y + data->h), tx2, ty2,
    };
    priv->submitTextured(data->texture, vtxdata, sizeof(vtxdata));
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

    int segments = path.numPoints() - 1;
    int points_len = (2 + 4) * 9 * segments;
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
