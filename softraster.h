// Based on sronsse's software rasterizer for SDL https://github.com/sronsse/imgui/blob/sw_rasterizer_example/examples/sdl_sw_example/imgui_impl_sdl.cpp
#ifndef SOFTRASTER_H
#define SOFTRASTER_H
#ifdef ARDUINO
#include "arduino.h"
#endif
#include "stdint.h"
#include "math.h"
#include "imgui.h"

typedef uint8_t color8_t; //256 color

typedef uint16_t color16_t; //65k color (high color)

struct color24_t { //16M color (true color)
    color8_t r;
    color8_t g;
    color8_t b;
    bool operator==(const color24_t& rhs) const
    {
        return (r == rhs.r) && (g == rhs.g) && (b == rhs.b);
    }
    bool operator!=(const color24_t& rhs) const { return !(*this == rhs); }
};

struct color32_t { //16M color (true color) + 256 alpha
    color8_t r;
    color8_t g;
    color8_t b;
    color8_t a;
    bool operator==(const color32_t& rhs) const
    {
        return (r == rhs.r) && (g == rhs.g) && (b == rhs.b) && (a == rhs.a);
    }
    bool operator!=(const color32_t& rhs) const { return !(*this == rhs); }
};

#define COLOR8 sizeof(color8_t)
#define COLOR16 sizeof(color16_t)
#define COLOR24 sizeof(color24_t)
#define COLOR32 sizeof(color32_t)

struct clip_t
{
    int16_t x1, y1, x2, y2;
};

#define position_t int32_t

struct pixel_t
{
    position_t x, y;
    float u, v;
    color32_t c;
};

struct line_t
{
    pixel_t p1, p2;
};

struct triangle_t
{
    pixel_t p1, p2, p3;
};

struct rectangle_t
{
    pixel_t p1, p2;
};

//Allows multiple texture modes/types to be stored in a type that can be handled by ImGui
// struct texture_t
// {
//     bool isSetup = false;
//     uint8_t colorMode = 0;
//     union
//     {
//         texture8_t tex8;
//         ctexture8_t ctex8;
//         texture16_t tex16;
//         texture24_t tex24;
//         texture32_t tex32;
//     };
//     size_t* w()
//     {
//         switch(colorMode)
//         {
//             case COLOR8:
//                 return &tex8.w;
//                 break;
//             case COLOR16:
//                 return &tex16.w;
//                 break;
//             case COLOR24:
//                 return &tex24.w;
//                 break;
//             case COLOR32:
//                 return &tex32.w;
//                 break;
//             default:
//                 return NULL;
//                 break;
//         }
//     }
//     size_t* h()
//     {
//         switch(colorMode)
//         {
//             case COLOR8:
//                 return &tex8.h;
//                 break;
//             case COLOR16:
//                 return &tex16.h;
//                 break;
//             case COLOR24:
//                 return &tex24.h;
//                 break;
//             case COLOR32:
//                 return &tex32.h;
//                 break;
//             default:
//                 return NULL;
//                 break;
//         }
//     }
//     void clear()
//     {
//         switch(colorMode)
//         {
//             case COLOR8:
//                 tex8.clear();
//                 break;
//             case COLOR16:
//                 tex16.clear();
//                 break;
//             case COLOR24:
//                 tex24.clear();
//                 break;
//             case COLOR32:
//                 tex32.clear();
//                 break;
//             default: break;
//         }
//     }
//     void pre_init(uint8_t mode)
//     {
//         colorMode = mode;
//         switch(colorMode)
//         {
//             case COLOR8:
//                 tex8.pre_init();
//                 break;
//             case COLOR16:
//                 tex16.pre_init();
//                 break;
//             case COLOR24:
//                 tex24.pre_init();
//                 break;
//             case COLOR32:
//                 tex32.pre_init();
//                 break;
//             default: break;
//         }
//         isSetup = true;
//     }
//     void init(size_t x, size_t y, uint8_t mode)
//     {
//         colorMode = mode;
//         switch(colorMode)
//         {
//             case COLOR8:
//                 tex8.init(x, y);
//                 break;
//             case COLOR16:
//                 tex16.init(x, y);
//                 break;
//             case COLOR24:
//                 tex24.init(x, y);
//                 break;
//             case COLOR32:
//                 tex32.init(x, y);
//                 break;
//             default: break;
//         }
//         isSetup = true;
//     }
//     texture_t() : isSetup(false){}
//     texture_t(size_t x, size_t y, uint8_t mode)
//     {
//         init(x, y, mode);
//     }
//     ~texture_t()
//     {
//         if (isSetup)
//         {
//             switch(colorMode)
//             {
//                 case COLOR8:
//                     delete &tex8;
//                     break;
//                 case COLOR16:
//                     delete &tex16;
//                     break;
//                 case COLOR24:
//                     delete &tex24;
//                     break;
//                 case COLOR32:
//                     delete &tex32;
//                     break;
//                 default: break;
//             }
//         }
//         memset(&tex32, NULL, sizeof(tex32));
//         isSetup = false;
//     }
// };

//extern texture_t fontAtlas;

// Template the texture struct so we can use different color modes/types
template <typename COL_T>
struct texture_t
{
    bool isSetup = false;
    uint8_t colorMode = 0;
    size_t w, h;
    COL_T** col = NULL;
    void clear()
    {
        for(int i = 0; i < w; i++)
        {
            for(int j = 0; j < h; j++)
            {
                memset(&(col[i][j]), 0x0, colorMode);
            }
        }
    }
    void pre_init()
    {
        colorMode = sizeof(COL_T);
        col = (COL_T**)malloc(w*sizeof(COL_T*));
    }
    void init(size_t x, size_t y)
    {
        w = x;
        h = y;
        pre_init();
        for (size_t i = 0; i < w; i++)
        {
            col[i] = (COL_T*)malloc(h*sizeof(COL_T));
        }
        isSetup = true;
    }
    texture_t() : col(NULL), isSetup(false){}
    texture_t(size_t x, size_t y)
    {
        init(x, y);
    }
    ~texture_t()
    {
        if (col != NULL && isSetup)
        {
            for (size_t i = 0; i < w; i++)
            {
                if (col[i] != NULL)
                {
                    free(col[i]);
                    col[i] = NULL;
                }
            }
            free(col);
            col = NULL;
        }
        isSetup = false;
    }
};

struct void_texture_t
{
    uint8_t mode;
    void* texture;
};

template<typename colType>
struct screen_t
{
    texture_t<colType>* buffer;
    clip_t* clip;
    size_t w;
    size_t h;
};

template<typename screenColType, typename texColType>
struct renderData_t
{
    screen_t<screenColType>* screen;
    ImVec4 clipRect;
    texture_t<texColType>* texture;
    renderData_t(screen_t<screenColType>* s, texture_t<texColType>* t, ImVec4 c) : screen(s), texture(t), clipRect(c){}
};

position_t dot(const pixel_t& a, const pixel_t& b);

// Linear interpolation functions
template<typename T>
inline T lerp(T a, T b, uint8_t f) // [0, 255]
{
    if (a == b) return a;
    return a + ((f * (b - a)) / 0xFF);
}

template<typename T>
inline T lerp(T a, T b, float f) // [0.0f, 1.0f]
{
    if (a == b) return a;
    return a + (f * (b - a));
}

// Color type conversions
template<typename C1, typename C2>
inline C1 convCol(const C2& c)
{
    C1 c1;
    return c1;
}

template<> inline color8_t convCol(const color8_t& c) { return c; }
template<> inline color16_t convCol(const color16_t& c) { return c; }
template<> inline color24_t convCol(const color24_t& c) { return c; }
template<> inline color32_t convCol(const color32_t& c) { return c; }

template<> inline color16_t convCol(const color8_t& c)
{
    return (((c * 31) / 255) << 11) |
        (((c * 63) / 255) << 5) |
        ((c * 31) / 255);
}
template<> inline color24_t convCol (const color8_t& c)
{
    color24_t rtn;
    rtn.r = c;
    rtn.g = c;
    rtn.b = c;
    return rtn;
}
#if defined(COL8_ALPHACOLOR)
template<> inline color8_t convCol (const color32_t& c)
{
    return (((c.r + c.g + c.b) / 3) * c.a) / 255;
}
template<> inline color32_t convCol (const color8_t& c)
{
    color32_t rtn;
    rtn.r = c;
    rtn.g = c;
    rtn.b = c;
    rtn.a = c;
    return rtn;
}
#elif defined(COL8_COLOR)
template<> inline color8_t convCol (const color32_t& c)
{
    return (c.r + c.g + c.b) / 3;
}
template<> inline color32_t convCol (const color8_t& c)
{
    color32_t rtn;
    rtn.r = c;
    rtn.g = c;
    rtn.b = c;
    rtn.a = 0xFF;
    return rtn;
}
#else
template<> inline color8_t convCol (const color32_t& c)
{
    return c.a;
}
template<> inline color32_t convCol (const color8_t& c)
{
    color32_t rtn;
    rtn.r = 0xFF;
    rtn.g = 0xFF;
    rtn.b = 0xFF;
    rtn.a = c;
    return rtn;
}
#endif

template<> inline color8_t convCol (const color16_t& c)
{
    return (((((c & 0xF800) >> 11) * 255) / 31) +
        ((((c & 0x07E0) >> 5 ) * 255) / 63) +
        (((c & 0x001F) * 255) / 31)) / 3;
}
template<> inline color24_t convCol (const color16_t& c)
{
    color24_t rtn;
    rtn.r = (((c & 0xF800) >> 11) * 255) / 31;
    rtn.g = (((c & 0x07E0) >> 5 ) * 255) / 63;
    rtn.b = ((c & 0x001F) * 255) / 31;
    return rtn;
}
template<> inline color32_t convCol (const color16_t& c)
{
    color32_t rtn;
    rtn.r = (((c & 0xF800) >> 11) * 255) / 31;
    rtn.g = (((c & 0x07E0) >> 5 ) * 255) / 63;
    rtn.b = ((c & 0x001F) * 255) / 31;
    rtn.a = 0xFF;
    return rtn;
}

template<> inline color8_t convCol (const color24_t& c)
{
    return (c.r + c.g + c.b) / 3;
}
template<> inline color16_t convCol (const color24_t& c)
{
    return (((c.r * 31) / 255) << 11) |
        (((c.g * 63) / 255) << 5) |
        ((c.b * 31) / 255);
}
template<> inline color32_t convCol (const color24_t& c)
{
    color32_t rtn;
    rtn.r = c.r;
    rtn.g = c.g;
    rtn.b = c.b;
    rtn.a = 0xFF;
    return rtn;
}

template<> inline color16_t convCol (const color32_t& c)
{
    return (((c.r * 31) / 255) << 11) |
        (((c.g * 63) / 255) << 5) |
        ((c.b * 31) / 255);
}
template<> inline color24_t convCol (const color32_t& c)
{
    color24_t rtn;
    rtn.r = c.r;
    rtn.g = c.g;
    rtn.b = c.b;
    return rtn;
}

struct bary_t
{
    pixel_t a, b, c, p0, p1;
    position_t d00, d01, d11;
    float denom;
};

inline bary_t baryPre(const pixel_t& a, const pixel_t& b, const pixel_t& c)
{
    bary_t bary;
    bary.p0.x = b.x - a.x;
    bary.p0.y = b.y - a.x;
    bary.p1.x = c.x - a.x;
    bary.p1.y = c.y - a.y;
    bary.d00 = dot(bary.p0, bary.p0);
    bary.d01 = dot(bary.p0, bary.p1);
    bary.d11 = dot(bary.p1, bary.p1);
    bary.denom = 1.0f / ((bary.d00 * bary.d11) - (bary.d01 * bary.d01));
    bary.a = a;
    bary.b = b;
    bary.c = c;
    return bary;
}

inline void barycentricCol(pixel_t& p, const bary_t& bary)
{
    pixel_t p2; p2.x = p.x - bary.a.x; p2.y = p.y - bary.a.y;
    position_t d20 = dot(p2, bary.p0);
    position_t d21 = dot(p2, bary.p1);
    float v = (bary.d11 * d20 - bary.d01 * d21) * bary.denom;
    float w = (bary.d00 * d21 - bary.d01 * d20) * bary.denom;
    float u = 1.0f - v - w;
    p.c.r = (bary.a.c.r * u) + (bary.b.c.r * v) + (bary.c.c.r * w);
    p.c.g = (bary.a.c.g * u) + (bary.b.c.g * v) + (bary.c.c.g * w);
    p.c.b = (bary.a.c.b * u) + (bary.b.c.b * v) + (bary.c.c.b * w);
    p.c.a = (bary.a.c.a * u) + (bary.b.c.a * v) + (bary.c.c.a * w);
}

inline void barycentricUV(pixel_t& p, const bary_t& bary)
{
    pixel_t p2; p2.x = p.x - bary.a.x; p2.y = p.y - bary.a.y;
    position_t d20 = dot(p2, bary.p0);
    position_t d21 = dot(p2, bary.p1);
    float v = (bary.d11 * d20 - bary.d01 * d21) * bary.denom;
    float w = (bary.d00 * d21 - bary.d01 * d20) * bary.denom;
    float u = 1.0f - v - w;
    p.u = (bary.a.u * u) + (bary.b.u * v) + (bary.c.u * w);
    p.v = (bary.a.v * u) + (bary.b.v * v) + (bary.c.v * w);
}

inline void barycentricUVCol(pixel_t& p, const bary_t& bary)
{
    pixel_t p2; p2.x = p.x - bary.a.x; p2.y = p.y - bary.a.y;
    position_t d20 = dot(p2, bary.p0);
    position_t d21 = dot(p2, bary.p1);
    float v = (bary.d11 * d20 - bary.d01 * d21) * bary.denom;
    float w = (bary.d00 * d21 - bary.d01 * d20) * bary.denom;
    float u = 1.0f - v - w;
    p.u = (bary.a.u * u) + (bary.b.u * v) + (bary.c.u * w);
    p.v = (bary.a.v * u) + (bary.b.v * v) + (bary.c.v * w);
    p.c.r = (bary.a.c.r * u) + (bary.b.c.r * v) + (bary.c.c.r * w);
    p.c.g = (bary.a.c.g * u) + (bary.b.c.g * v) + (bary.c.c.g * w);
    p.c.b = (bary.a.c.b * u) + (bary.b.c.b * v) + (bary.c.c.b * w);
    p.c.a = (bary.a.c.a * u) + (bary.b.c.a * v) + (bary.c.c.a * w);
}

enum render_t {
    NORMAL  = 0,
    BLEND   = 1<<0,
    TEXTURE = 1<<1
};

template<typename screenColType, typename texColType>
void renderRect(renderData_t<screenColType, texColType>* renderData, rectangle_t* rect, const render_t mode)
{
    const screen_t<screenColType>* screen = renderData->screen;
    const texture_t<texColType>* tex = renderData->texture;
    const ImVec4& clipRect = renderData->clipRect;

    const position_t minclipx = (clipRect.x > 0 ? clipRect.x : 0);
    const position_t minclipy = (clipRect.y > 0 ? clipRect.y : 0);

    const position_t maxclipx = (clipRect.z < screen->w ? clipRect.z : screen->w);
    const position_t maxclipy = (clipRect.w < screen->h ? clipRect.w : screen->h);

    if ((rect->p2.x < minclipx) ||
        (rect->p2.y < minclipy) ||
        (rect->p1.x >= maxclipx) ||
        (rect->p1.y >= maxclipy))
        return;

    const position_t startx = (rect->p1.x > minclipx ? rect->p1.x : minclipx);
    const position_t starty = (rect->p1.y > minclipy ? rect->p1.y : minclipy);

    const position_t endx = (rect->p2.x < maxclipx ? rect->p2.x : maxclipx);
    const position_t endy = (rect->p2.y < maxclipy ? rect->p2.y : maxclipy);

    #ifdef CLIP_SCREEN
    screen->clip->x1 = (startx < screen->clip->x1 || screen->clip->x1 == -1 ? startx : screen->clip->x1);
    screen->clip->y1 = (starty < screen->clip->y1 || screen->clip->y1 == -1 ? starty : screen->clip->y1);
    screen->clip->x2 = (endx > screen->clip->x2 || screen->clip->x2 == -1 ? endx-1 : screen->clip->x2);
    screen->clip->y2 = (endy > screen->clip->y2 || screen->clip->y2 == -1 ? endy-1 : screen->clip->y2);
    #endif

    if (mode & render_t::TEXTURE)
    {
        #ifdef TEXTURE_MODE_CLAMP
        position_t p1u = (position_t)((rect->p1.u * tex->w) + 0.5f);
        if (p1u < 0) p1u = 0;
        else if (p1u > tex->w) p1u = tex->w - 1;

        position_t p1v = (position_t)((rect->p1.v * tex->h) + 0.5f);
        if (p1v < 0) p1v = 0;
        else if (p1v > tex->h) p1v = tex->h - 1;

        position_t p2u = (position_t)((rect->p2.u * tex->w) + 0.5f);
        if (p2u < 0) p2u = 0;
        else if (p2u > tex->w) p2u = tex->w - 1;

        position_t p2v = (position_t)((rect->p2.v * tex->h) + 0.5f);
        if (p2v < 0) p2v = 0;
        else if (p2v > tex->h) p2v = tex->h - 1;
        #else
        position_t p1u = (position_t)((rect->p1.u * tex->w) + 0.5f) % tex->w;
        position_t p1v = (position_t)((rect->p1.v * tex->h) + 0.5f) % tex->h;
        position_t p2u = (position_t)((rect->p2.u * tex->w) + 0.5f) % tex->w;
        position_t p2v = (position_t)((rect->p2.v * tex->h) + 0.5f) % tex->h;
        #endif

        const float duDx = (float)(p2u - p1u) / (float)(rect->p2.x - rect->p1.x);
        const float dvDy = (float)(p2v - p1v) / (float)(rect->p2.y - rect->p1.y);

        const float startu = p1u + (duDx * (float)(startx - rect->p1.x > 0 ? startx - rect->p1.x : 0));
        const float startv = p1v + (dvDy * (float)(starty - rect->p1.y > 0 ? starty - rect->p1.y : 0));

        bool blit = ((duDx == 1.0f) && (dvDy == 1.0f));

        if (blit)
        {
            position_t u = startu - startx, v = startv - starty;
            for (position_t x = startx; x < endx; x++)
            {
                for (position_t y = starty; y < endy; y++)
                {
                    color32_t c = convCol<color32_t>(tex->col[u+x][v+y]);
                    if (c.a == 0) continue;
                    c.r = (c.r * rect->p1.c.r) / 255;
                    c.g = (c.g * rect->p1.c.g) / 255;
                    c.b = (c.b * rect->p1.c.b) / 255;
                    if (mode & render_t::BLEND && c.a != 255)
                    {
                        color24_t prev = convCol<color24_t>(screen->buffer->col[x][y]);
                        c.r = lerp(prev.r, c.r, c.a);
                        c.g = lerp(prev.g, c.g, c.a);
                        c.b = lerp(prev.b, c.b, c.a);
                    }
                    screen->buffer->col[x][y] = convCol<screenColType>(c);
                }
            }
        }
        else
        {
            float u = startu, v = startv;
            for (position_t x = startx; x < endx; x++)
            {
                for (position_t y = starty; y < endy; y++)
                {
                    color32_t c = convCol<color32_t>(tex->col[(position_t)(u+0.5f)][(position_t)(v+0.5f)]);
                    if (c.a == 0) continue;
                    c.r = (c.r * rect->p1.c.r) / 255;
                    c.g = (c.g * rect->p1.c.g) / 255;
                    c.b = (c.b * rect->p1.c.b) / 255;
                    if (mode & render_t::BLEND && c.a != 255)
                    {
                        color24_t prev = convCol<color24_t>(screen->buffer->col[x][y]);
                        c.r = lerp(prev.r, c.r, c.a);
                        c.g = lerp(prev.g, c.g, c.a);
                        c.b = lerp(prev.b, c.b, c.a);
                    }
                    screen->buffer->col[x][y] = convCol<screenColType>(c);
                    v += dvDy;
                }
                u += duDx;
            }
        }
    }
    else if (mode & render_t::BLEND)
    {
        for (position_t x = startx; x < endx; x++)
        {
            for (position_t y = starty; y < endy; y++)
            {
                // could make a specialised lerp function for each color type
                // to reduce the number of color conversions per pixel
                color24_t prev = convCol<color24_t>(screen->buffer->col[x][y]);
                prev.r = lerp(prev.r, rect->p1.c.r, rect->p1.c.a);
                prev.g = lerp(prev.g, rect->p1.c.g, rect->p1.c.a);
                prev.b = lerp(prev.b, rect->p1.c.b, rect->p1.c.a);
                screen->buffer->col[x][y] = convCol<screenColType>(prev);
            }
        }
    }
    else
    {
        for (position_t x = startx; x < endx; x++)
        {
            for (position_t y = starty; y < endy; y++)
            {
                screen->buffer->col[x][y] = convCol<screenColType>(rect->p1.c);
            }
        }
    }
}

template<typename T>
inline void swap(T* tri1, T* tri2)
{
    T temp;
    memcpy(&temp, tri1, sizeof(T));
    memcpy(tri1, tri2, sizeof(T));
    memcpy(tri2, &temp, sizeof(T));
}

template<typename screenColType, typename texColType>
void renderTriCore(
    const renderData_t<screenColType, texColType> *const renderData,
    const position_t minclipx,  const position_t maxclipx,
    const position_t minclipy,  const position_t maxclipy,
    const position_t startx,    const position_t endx,
    const position_t starty1,   const position_t starty2,
    const position_t endy1,     const position_t endy2,
    const position_t f1,        const position_t f2,
    const bary_t bary,          const color32_t col,
    const render_t mode
)
{
    const screen_t<screenColType> *const screen = renderData->screen;
    const texture_t<texColType> *const tex = renderData->texture;

    for (position_t x = startx; x < endx; ++x)
    {
        const float f = (x - f1) / (float)(f2 - f1);
        position_t starty   = lerp(starty1, starty2, f);
        position_t endy     = lerp(endy1, endy2, f);
        starty  = (starty > minclipy ? starty : minclipy);
        endy    = (endy < maxclipy ? endy : maxclipy);
        for (position_t y = starty; y < endy; ++y)
        {
            if (mode & render_t::TEXTURE)
            {
                pixel_t p; p.x = x; p.y = y;
                barycentricCol(p, bary);
                #ifdef TEXTURE_MODE_CLAMP
                position_t u = (position_t)((p.u * tex->w) + 0.5f);
                position_t v = (position_t)((p.v * tex->h) + 0.5f);
                if (u < 0) u = 0;
                else if (u > tex->w) u = tex->w - 1;
                if (v < 0) v = 0;
                else if (v > tex->h) v = tex->h - 1;
                #else
                position_t u = (position_t)((p.u * tex->w) + 0.5f) % tex->w;
                position_t v = (position_t)((p.v * tex->h) + 0.5f) % tex->h;
                #endif
                if (mode & render_t::BLEND)
                {
                    color32_t c = convCol<color32_t>(tex->col[u][v]);
                    if (!c.a || !col.a) { c.a = 0; continue; }
                    c.a = (c.a * col.a) / 255;
                    c.r = (c.r * col.r) / 255;
                    c.g = (c.g * col.g) / 255;
                    c.b = (c.b * col.b) / 255;
                    color24_t prev;
                    if (c.a == 255)
                    {
                        prev = convCol<color24_t>(c);
                    }
                    else
                    {
                        prev = convCol<color24_t>(screen->buffer->col[x][y]);
                        prev.r = lerp(prev.r, c.r, c.a);
                        prev.g = lerp(prev.g, c.g, c.a);
                        prev.b = lerp(prev.b, c.b, c.a);
                    }
                    screen->buffer->col[x][y] = convCol<screenColType>(prev);
                }
                else
                {
                    color24_t c = convCol<color24_t>(tex->col[u][v]);
                    c.r = (c.r * col.r) / 255;
                    c.g = (c.g * col.g) / 255;
                    c.b = (c.b * col.b) / 255;
                    screen->buffer->col[x][y] = convCol<screenColType>(c);
                }
            }
            else if (mode & render_t::BLEND)
            {
                // pixel_t p; p.x = tri->p1.x; p.y = y;
                pixel_t p; p.x = x; p.y = y;
                barycentricCol(p, bary);
                color24_t prev = convCol<color24_t>(screen->buffer->col[x][y]);
                prev.r = lerp(prev.r, p.c.r, p.c.a);
                prev.g = lerp(prev.g, p.c.g, p.c.a);
                prev.b = lerp(prev.b, p.c.b, p.c.a);
                screen->buffer->col[x][y] = convCol<screenColType>(prev);
            }
            else
            {
                screen->buffer->col[x][y] = convCol<screenColType>(col);
            }
        }
    }
}

template<typename screenColType, typename texColType>
void renderTri(renderData_t<screenColType, texColType>* renderData, triangle_t* tri, const render_t mode)
{
    tri->p1.x = (position_t)(tri->p1.x + 0.5f);
    tri->p2.x = (position_t)(tri->p2.x + 0.5f);
    tri->p3.x = (position_t)(tri->p3.x + 0.5f);

    if (tri->p1.x > tri->p2.x) swap<pixel_t>(&(tri->p1), &(tri->p2));
    if (tri->p1.x > tri->p3.x) swap<pixel_t>(&(tri->p1), &(tri->p3));
    if (tri->p2.x > tri->p3.x) swap<pixel_t>(&(tri->p2), &(tri->p3));

    const screen_t<screenColType> *const screen = renderData->screen;
    const ImVec4& clipRect = renderData->clipRect;

    const position_t minclipx = (clipRect.x > 0 ? clipRect.x : 0);
    const position_t minclipy = (clipRect.y > 0 ? clipRect.y : 0);

    const position_t maxclipx = (clipRect.z < screen->w ? clipRect.z : screen->w);
    const position_t maxclipy = (clipRect.w < screen->h ? clipRect.w : screen->h);

    if ((tri->p3.x < minclipx) ||
        (tri->p1.x >= maxclipx))
        return;

    #ifdef CLIP_SCREEN
    screen->clip->x1 = (tri->p1.x < screen->clip->x1 || screen->clip->x1 == -1 ? tri->p1.x : screen->clip->x1);
    screen->clip->x2 = (tri->p3.x > screen->clip->x2 || screen->clip->x2 == -1 ? tri->p3.x : screen->clip->x2);
    position_t miny = (tri->p1.y < tri->p2.y ? (tri->p1.y < tri->p3.y ? tri->p1.y : tri->p3.y) : (tri->p2.y < tri->p3.y ? tri->p2.y : tri->p3.y));
    position_t maxy = (tri->p1.y > tri->p2.y ? (tri->p1.y > tri->p3.y ? tri->p1.y : tri->p3.y) : (tri->p2.y > tri->p3.y ? tri->p2.y : tri->p3.y));
    screen->clip->y1 = (miny < screen->clip->y1 || screen->clip->y1 == -1 ? miny : screen->clip->y1);
    screen->clip->y2 = (maxy > screen->clip->y2 || screen->clip->y2 == -1 ? maxy : screen->clip->y2);
    #endif

    position_t startx, endx;
    position_t starty1, starty2, endy1, endy2;

    if (tri->p2.x == tri->p3.x) // Flat right triangle
    {
        if (tri->p1.x == tri->p2.x) // Flat line
        {
            if (tri->p1.y > tri->p2.y) swap<pixel_t>(&(tri->p1), &(tri->p2));
            if (tri->p1.y > tri->p3.y) swap<pixel_t>(&(tri->p1), &(tri->p3));
            if (tri->p2.y > tri->p3.y) swap<pixel_t>(&(tri->p2), &(tri->p3));
            if (tri->p1.x < minclipx || tri->p1.x > maxclipx) return;
            renderTriCore(renderData,
                minclipx, maxclipx,
                minclipy, maxclipy,
                tri->p1.x, tri->p1.x + 1,           // startx, endx
                tri->p1.y, tri->p1.y,               // starty1, starty2
                tri->p3.y, tri->p3.y,               // endy1, endy2
                0, 1,                               // f1, f2
                baryPre(tri->p1, tri->p2, tri->p3), // bary
                tri->p1.c,                          // color
                mode
            );
            return;
        }
        else
        {
            if (tri->p2.y > tri->p3.y) swap<pixel_t>(&(tri->p2), &(tri->p3));
            renderTriCore(renderData,
                minclipx, maxclipx,
                minclipy, maxclipy,
                (tri->p1.x > minclipx ? tri->p1.x : minclipx),  // startx
                (tri->p3.x < maxclipx ? tri->p3.x : maxclipx),  // endx
                tri->p1.y, tri->p2.y,                           // starty1, starty2
                tri->p1.y, tri->p3.y,                           // endy1, endy2
                tri->p1.x, tri->p3.x,                           // f1, f2
                baryPre(tri->p1, tri->p2, tri->p3),             // bary
                tri->p1.c,                                      // color
                mode
            );
            return;
        }
    }
    else if (tri->p1.x == tri->p2.x) // Flat left triangle
    {
        if (tri->p1.y > tri->p2.y) swap<pixel_t>(&(tri->p1), &(tri->p2));
        renderTriCore(renderData,
            minclipx, maxclipx,
            minclipy, maxclipy,
            (tri->p1.x > minclipx ? tri->p1.x : minclipx),  // startx
            (tri->p3.x < maxclipx ? tri->p3.x : maxclipx),  // endx
            tri->p1.y, tri->p3.y,                           // starty1, starty2
            tri->p2.y, tri->p3.y,                           // endy1, endy2
            tri->p1.x, tri->p3.x,                           // f1, f2
            baryPre(tri->p1, tri->p2, tri->p3),             // bary
            tri->p1.c,                                      // color
            mode
        );
        return;
    }

    // Find 4th point to split the tri into flat top and flat bottom triangles
    pixel_t p4;
    p4.x = lerp(tri->p1.x, tri->p3.x,
        (tri->p2.x - tri->p1.x) / (float)(tri->p3.x - tri->p1.x));
    p4.y = tri->p2.y;
    p4.c = tri->p1.c;

    if (tri->p2.x > p4.x) swap<pixel_t>(&(tri->p2), &p4);

    {   // Flat right
        const pixel_t &fp1 = tri->p1;
        pixel_t fp2 = tri->p2;
        pixel_t fp3 = p4;
        if (fp2.y > fp3.y) swap<pixel_t>(&(fp2), &(fp3));
        renderTriCore(renderData,
            minclipx, maxclipx,
            minclipy, maxclipy,
            (fp1.x > minclipx ? fp1.x : minclipx),  // startx
            (fp3.x < maxclipx ? fp3.x : maxclipx),  // endx
            fp1.y, fp2.y,                           // starty1, starty2
            fp1.y, fp3.y,                           // endy1, endy2
            fp1.x, fp3.x,                           // f1, f2
            baryPre(fp1, fp2, fp3),                 // bary
            tri->p1.c,                              // color
            mode
        );
        return;
    }

    {   // Flat left
        pixel_t fp1 = tri->p2;
        pixel_t fp2 = p4;
        const pixel_t &fp3 = tri->p3;
        if (fp1.y > fp2.y) swap<pixel_t>(&(fp1), &(fp2));
        renderTriCore(renderData,
            minclipx, maxclipx,
            minclipy, maxclipy,
            (fp1.x > minclipx ? fp1.x : minclipx),  // startx
            (fp3.x < maxclipx ? fp3.x : maxclipx),  // endx
            fp1.y, fp3.y,                           // starty1, starty2
            fp2.y, fp3.y,                           // endy1, endy2
            fp1.x, fp3.x,                           // f1, f2
            baryPre(fp1, fp2, fp3),                 // bary
            tri->p1.c,                              // color
            mode
        );
        return;
    }
}

template<typename screenColType, typename texColType>
void renderCommand(const ImDrawVert* vtx_buffer, const ImDrawIdx* idx_buffer, const ImDrawCmd* pcmd, renderData_t<screenColType, texColType>* renderData)
{
    for(unsigned int i = 0; i < pcmd->ElemCount; i += 3)
    {
        const ImDrawVert* verts[] =
        {
            &vtx_buffer[idx_buffer[i]],
            &vtx_buffer[idx_buffer[i+1]],
            &vtx_buffer[idx_buffer[i+2]]
        };

        if (i < pcmd->ElemCount - 3)
        {
            ImVec2 tlpos = verts[0]->pos;
            ImVec2 brpos = verts[0]->pos;
            ImVec2 tluv = verts[0]->uv;
            ImVec2 bruv = verts[0]->uv;
            for (int v = 1; v < 3; v++)
            {
                if (verts[v]->pos.x < tlpos.x)
                {
                    tlpos.x = verts[v]->pos.x;
                    tluv.x = verts[v]->uv.x;
                }
                else if (verts[v]->pos.x > brpos.x)
                {
                    brpos.x = verts[v]->pos.x;
                    bruv.x = verts[v]->uv.x;
                }
                if (verts[v]->pos.y < tlpos.y)
                {
                    tlpos.y = verts[v]->pos.y;
                    tluv.y = verts[v]->uv.y;
                }
                else if (verts[v]->pos.y > brpos.y)
                {
                    brpos.y = verts[v]->pos.y;
                    bruv.y = verts[v]->uv.y;
                }
            }

            const ImDrawVert* nextVerts[] =
            {
                &vtx_buffer[idx_buffer[i+3]],
                &vtx_buffer[idx_buffer[i+4]],
                &vtx_buffer[idx_buffer[i+5]]
            };

            bool isRect = true;
            for (int v = 0; v < 3; v++)
            {
                if (((nextVerts[v]->pos.x != tlpos.x) && (nextVerts[v]->pos.x != brpos.x)) ||
                    ((nextVerts[v]->pos.y != tlpos.y) && (nextVerts[v]->pos.y != brpos.y)) ||
                    ((nextVerts[v]->uv.x != tluv.x) && (nextVerts[v]->uv.x != bruv.x)) ||
                    ((nextVerts[v]->uv.y != tluv.y) && (nextVerts[v]->uv.y != bruv.y)))
                {
                    isRect = false;
                    break;
                }
            }

            if (isRect)
            {
                rectangle_t rect;
                rect.p1.x = tlpos.x;
                rect.p1.y = tlpos.y;
                rect.p2.x = brpos.x;
                rect.p2.y = brpos.y;
                rect.p1.u = tluv.x;
                rect.p1.v = tluv.y;
                rect.p2.u = bruv.x;
                rect.p2.v = bruv.y;
                rect.p1.c.r = (verts[0]->col >> IM_COL32_R_SHIFT) & 0xFF;
                rect.p2.c.r = rect.p1.c.r;
                rect.p1.c.g = (verts[0]->col >> IM_COL32_G_SHIFT) & 0xFF;
                rect.p2.c.g = rect.p1.c.g;
                rect.p1.c.b = (verts[0]->col >> IM_COL32_B_SHIFT) & 0xFF;
                rect.p2.c.b = rect.p1.c.b;
                rect.p1.c.a = (verts[0]->col >> IM_COL32_A_SHIFT) & 0xFF;
                rect.p2.c.a = rect.p1.c.a;
                bool noUV = renderData->texture == NULL ||
                    ((rect.p1.u == rect.p2.u) && (rect.p1.v == rect.p2.v));
                bool flatCol = rect.p1.c == rect.p2.c;
                renderRect<screenColType, texColType>(renderData, &rect,
                    (render_t)((noUV ? render_t::NORMAL : render_t::TEXTURE) |
                    (flatCol ? render_t::NORMAL : render_t::BLEND))
                );
                i += 3;
                continue;
            }
        }

        triangle_t tri;
        tri.p1.x = verts[0]->pos.x;
        tri.p1.y = verts[0]->pos.y;
        tri.p1.u = verts[0]->uv.x;
        tri.p1.v = verts[0]->uv.y;
        tri.p1.c.r = (verts[0]->col >> IM_COL32_R_SHIFT) & 0xFF;
        tri.p1.c.g = (verts[0]->col >> IM_COL32_G_SHIFT) & 0xFF;
        tri.p1.c.b = (verts[0]->col >> IM_COL32_B_SHIFT) & 0xFF;
        tri.p1.c.a = (verts[0]->col >> IM_COL32_A_SHIFT) & 0xFF;
        tri.p2.x = verts[1]->pos.x;
        tri.p2.y = verts[1]->pos.y;
        tri.p2.u = verts[1]->uv.x;
        tri.p2.v = verts[1]->uv.y;
        tri.p2.c.r = (verts[1]->col >> IM_COL32_R_SHIFT) & 0xFF;
        tri.p2.c.g = (verts[1]->col >> IM_COL32_G_SHIFT) & 0xFF;
        tri.p2.c.b = (verts[1]->col >> IM_COL32_B_SHIFT) & 0xFF;
        tri.p2.c.a = (verts[1]->col >> IM_COL32_A_SHIFT) & 0xFF;
        tri.p3.x = verts[2]->pos.x;
        tri.p3.y = verts[2]->pos.y;
        tri.p3.u = verts[2]->uv.x;
        tri.p3.v = verts[2]->uv.y;
        tri.p3.c.r = (verts[2]->col >> IM_COL32_R_SHIFT) & 0xFF;
        tri.p3.c.g = (verts[2]->col >> IM_COL32_G_SHIFT) & 0xFF;
        tri.p3.c.b = (verts[2]->col >> IM_COL32_B_SHIFT) & 0xFF;
        tri.p3.c.a = (verts[2]->col >> IM_COL32_A_SHIFT) & 0xFF;
        bool noUV = renderData->texture == NULL || (
            (tri.p1.u == tri.p2.u) && (tri.p1.u == tri.p3.u) &&
            (tri.p1.v == tri.p2.v) && (tri.p1.v == tri.p3.v));
        bool flatCol = (tri.p1.c == tri.p2.c) && (tri.p1.c == tri.p3.c);
        renderTri<screenColType, texColType>(renderData, &tri,
            (render_t)((noUV ? render_t::NORMAL : render_t::TEXTURE) |
            (flatCol ? render_t::NORMAL : render_t::BLEND))
        );
    }
}

template<typename screenColType>
void renderDrawLists(ImDrawData* drawData, screen_t<screenColType>* screen)
{
    ImGuiIO& io = ImGui::GetIO();
    int fbWidth = (int)(io.DisplaySize.x * io.DisplayFramebufferScale.x);
    int fbHeight = (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y);
    if (fbWidth == 0 || fbHeight == 0)
        return;
    drawData->ScaleClipRects(io.DisplayFramebufferScale);

    for (int n = 0; n < drawData->CmdListsCount; n++)
    {
        const ImDrawList* cmdList = drawData->CmdLists[n];
        const ImDrawVert* vtx_buffer = cmdList->VtxBuffer.Data;
        const ImDrawIdx* idx_buffer = cmdList->IdxBuffer.Data;

        for (int cmdi = 0; cmdi < cmdList->CmdBuffer.Size; cmdi++)
        {
            const ImDrawCmd* pcmd = &cmdList->CmdBuffer[cmdi];
            if (pcmd->UserCallback)
            {
                pcmd->UserCallback(cmdList, pcmd);
            }
            else
            {
                void_texture_t* texVdPtr = (void_texture_t*)pcmd->TextureId;
                if(texVdPtr == NULL)
                {
                    renderData_t<screenColType, color8_t> renderData(
                        screen,
                        NULL,
                        pcmd->ClipRect
                    );
                    renderCommand<screenColType, color8_t>(vtx_buffer, idx_buffer, pcmd, &renderData);
                }
                else switch(texVdPtr->mode)
                {
                    case COLOR8: {
                            renderData_t<screenColType, color8_t> renderData(
                                screen,
                                (texture_t<color8_t>*)texVdPtr->texture,
                                pcmd->ClipRect
                            );
                            renderCommand<screenColType, color8_t>(vtx_buffer, idx_buffer, pcmd, &renderData);
                        }
                        break;
                    case COLOR16: {
                            renderData_t<screenColType, color16_t> renderData(
                                screen,
                                (texture_t<color16_t>*)texVdPtr->texture,
                                pcmd->ClipRect
                            );
                            renderCommand<screenColType, color16_t>(vtx_buffer, idx_buffer, pcmd, &renderData);
                        }
                        break;
                    case COLOR24: {
                            renderData_t<screenColType, color24_t> renderData(
                                screen,
                                (texture_t<color24_t>*)texVdPtr->texture,
                                pcmd->ClipRect
                            );
                            renderCommand<screenColType, color24_t>(vtx_buffer, idx_buffer, pcmd, &renderData);
                        }
                        break;
                    case COLOR32: {
                            renderData_t<screenColType, color32_t> renderData(
                                screen,
                                (texture_t<color32_t>*)texVdPtr->texture,
                                pcmd->ClipRect
                            );
                            renderCommand<screenColType, color32_t>(vtx_buffer, idx_buffer, pcmd, &renderData);
                        }
                        break;
                    default: {
                            renderData_t<screenColType, color8_t> renderData(
                                screen,
                                NULL,
                                pcmd->ClipRect
                            );
                            renderCommand<screenColType, color8_t>(vtx_buffer, idx_buffer, pcmd, &renderData);
                        }
                        break;
                }
            }
            idx_buffer += pcmd->ElemCount;
        }
    }
}

#endif
