//
// Created by joand on 10/2/2021.
//

#ifndef RAYCASTER_CANVAS_H
#define RAYCASTER_CANVAS_H

#undef UNICODE

#ifndef NOMINMAX
    #define NOMINMAX
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <cstdint>
#include <memory>
#include <optional>

namespace Canvas 
{

class BMPImage 
{

public:
    enum Error : uint8_t
    {
        BMP_IMG_OK = 0,
        BMP_IMG_WRONG_PIX_FMT, // Pixel Format is not 24-bit RGB. 
        BMP_IMG_WRONG_IMG_FMT, // Image is not a BMP image.
        BMP_IMG_NOT_FOUND,     // Image not found.
        BMP_IMG_CORRUPTED,     // Image data is probably corrupted.
    };

    BMPImage(const char* path);
    ~BMPImage();

    bool Failed() const { return m_error != 0; }
    Error GetError() const { return m_error; }

    [[nodiscard]] int32_t GetWidth() const { return m_width; }
    [[nodiscard]] int32_t GetHeight() const { return m_height; }
    [[nodiscard]] const uint8_t* GetPixelPtr() { return m_pixels; }

private:

    uint8_t* m_pixels = nullptr;
    int32_t  m_width, m_height;
    Error    m_error;

};

class Window
{
private:
    HWND     window;
    MSG      message;
    HDC      device_ctx;

    int32_t  mx, my;
    int32_t  mdx, mdy;
    uint8_t  keys[256];

    BOOL     running;
    int32_t  width, height;


    static LRESULT WINAPI s_wnd_proc(HWND win, UINT msg, WPARAM wparam, LPARAM lparam);
    
    LRESULT WINAPI wnd_proc(HWND win, UINT msg, WPARAM wparam, LPARAM lparam);

public:
    Window(int32_t width, int32_t height, const char* title);

    ~Window();

    void PollEvents();

    void SetTitle(const char* title);

    [[nodiscard]] BOOL GetKeyState(uint8_t key);

    [[nodiscard]] int32_t GetMouseDeltaX() const { return mdx; }

    [[nodiscard]] int32_t GetMouseDeltaY() const { return mdy; }

    [[nodiscard]] int32_t GetMousePositionX() const { return mx; }

    [[nodiscard]] int32_t GetMousePositionY() const { return height - my; }

    [[nodiscard]] BOOL IsRunning() const { return running; };
    
    [[nodiscard]] HDC GetDCHandle() const { return device_ctx; };

    [[nodiscard]] int32_t GetWidth() const { return width; };

    [[nodiscard]] int32_t GetHeight() const { return height; };
};


class Renderer
{
private:
    HDC                     m_deviceCtx;
    uint32_t*               m_pixels;
    Window*                 window;
    int32_t                 m_width, height;
    BITMAPINFO              m_bitmapInfo;

    std::optional<uint32_t> alphaKey = std::nullopt;

public:
    Renderer(const Window* window);

    ~Renderer();

    void Present();

    void ClearColour(uint32_t colour);

    void DrawPixel(int32_t x, int32_t y, uint32_t colour);

    void DrawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1,  uint32_t colour);

    void DrawRect(int32_t x, int32_t y, int32_t w, int32_t h,  uint32_t colour);

    void DrawCircle(int32_t x, int32_t y, int32_t r,  uint32_t colour);
    
    void DrawRectFill(int32_t x, int32_t y, int32_t w, int32_t h,  uint32_t colour);

    void DrawCircleFill(int32_t x, int32_t y, int32_t r,  uint32_t colour);

    void DrawRGBBitmap(int32_t x, int32_t y, int32_t w, int32_t h, const uint8_t *pixels);
    
    void SetBitmapAlphaKey(uint32_t colour);

    void ClearAlphaKey();
};

}

#endif //RAYCASTER_CANVAS_H