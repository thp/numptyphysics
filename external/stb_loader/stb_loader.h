#ifndef GABERLN_STB_LOADER_H
#define GABERLN_STB_LOADER_H

#include <stdint.h>
#include <stdlib.h>

#include <sys/types.h>

struct StbLoader_RGBA {
    StbLoader_RGBA(char *data, int w, int h, void (*freefunc)(char *)=0) : data(data), w(w), h(h), freefunc(freefunc) {}
    ~StbLoader_RGBA() { if (freefunc) freefunc(data); }

    char *data;
    int w;
    int h;
    void (*freefunc)(char *);
};

struct StbLoader_Color {
    StbLoader_Color(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}

    float r;
    float g;
    float b;
    float a;
};

class StbLoader {
public:
    static StbLoader_RGBA *
    decode_image(void *buffer, size_t len);

    static StbLoader_RGBA *
    render_font(void *buffer, size_t len, StbLoader_Color color, int size, const char *text);
};

#endif /* GABERLN_STB_LOADER_H */
