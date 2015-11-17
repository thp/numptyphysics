
#include "stb_loader.h"


#include "stb_image.c"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include "utf8decoder.h"

#include <algorithm>


static void do_free(char *ptr)
{
    free((void *)ptr);
}

static void do_free_stbi(char *ptr)
{
    stbi_image_free((stbi_uc *)ptr);
}

StbLoader_RGBA *
StbLoader::decode_image(void *buffer, size_t len)
{
    int w, h;
    stbi_uc *pixels = stbi_load_from_memory((stbi_uc *)buffer, len, &w, &h, NULL, 4);
    return new StbLoader_RGBA((char *)pixels, w, h, do_free_stbi);
}

StbLoader_RGBA *
StbLoader::render_font(void *buffer, size_t len, StbLoader_Color color, int size, const char *text)
{
    stbtt_fontinfo font;
    stbtt_fontinfo *f = &font;

    stbtt_InitFont(f, (const unsigned char *)buffer,
        stbtt_GetFontOffsetForIndex((const unsigned char *)buffer, 0));

    // Need to scale this a bit, so it looks roughly the same
    // size as the font rendered in other renderers
    float addscale = 1.25;

    int w = 0;
    int h = size * addscale;

    float scale = stbtt_ScaleForPixelHeight(f, size * addscale);

    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(f, &ascent, &descent, &lineGap);
    ascent *= scale;
    descent *= scale;
    lineGap *= scale;

    uint32_t codepoint = 0;
    uint32_t state = 0;
    const unsigned char *p = (const unsigned char *)text;

    // First pass: Get size of bitmap to render
    for (; *p; ++p) {
        if (utf8_decode(&state, &codepoint, *p) != UTF8_ACCEPT) {
            continue;
        }

        int cw, ch;
        int xo, yo;
        stbtt_FreeBitmap(stbtt_GetCodepointBitmap(f, scale, scale,
                    codepoint, &cw, &ch, &xo, &yo), nullptr);

        int advance, bearing;
        stbtt_GetCodepointHMetrics(f, codepoint, &advance, &bearing);
        advance *= scale;
        bearing *= scale;

        w += std::max(cw + xo, advance);
    }

    unsigned char *pixels = (unsigned char *)malloc(w * h * 4);
    memset(pixels, 0x00 /* set to 0x77 for debugging */, w * h * 4);

    int x = 0;

    codepoint = 0; state = 0; p = (const unsigned char *)text;

    // Second pass: Render RGBA bitmap
    for (; *p; ++p) {
        if (utf8_decode(&state, &codepoint, *p) != UTF8_ACCEPT) {
            continue;
        }

        int cw, ch;
        int xo, yo;
        unsigned char *cb = stbtt_GetCodepointBitmap(f, scale, scale,
                codepoint, &cw, &ch, &xo, &yo);

        int advance, bearing;
        stbtt_GetCodepointHMetrics(f, codepoint, &advance, &bearing);
        advance *= scale;
        bearing *= scale;

        x += xo;

        int y = h + yo + descent;

        for (int dx=0; dx<cw; dx++) {
            for (int dy=0; dy<ch; dy++) {
                /* Clipping */
                if (x + dx < 0 || x + dx >= w) {
                    continue;
                } else if (y + dy < 0 || y + dy >= h) {
                    continue;
                }

                // RGB
                pixels[4 * ((x + dx) + w * (y + dy)) + 0] = 255*color.r;
                pixels[4 * ((x + dx) + w * (y + dy)) + 1] = 255*color.g;
                pixels[4 * ((x + dx) + w * (y + dy)) + 2] = 255*color.b;

                // Alpha
                unsigned char &a = pixels[4 * ((x + dx) + w * (y + dy)) + 3];
                a = std::min(0xFF, a + cb[dx+dy*cw]);
            }
        }

        x += advance - bearing;

        stbtt_FreeBitmap(cb, nullptr);
    }

    return new StbLoader_RGBA((char *)pixels, w, h, do_free);
}
