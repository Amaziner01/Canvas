//
// Created by joand on 10/2/2021.
//

#include "canvas.hh"
#include <cmath>

#include <iostream>

#define PI 3.1416f

namespace Canvas 
{

BMPImage::BMPImage(const char* path) 
{
    BITMAPFILEHEADER header  = {};
    BITMAPINFOHEADER info    = {};
    FILE* fPtr = NULL;

#ifdef _MSC_VER
    fopen_s(&fPtr, path, "rb");
#else
    fPtr = fopen(path, "rb");
#endif

    if (!fPtr) 
    { 
        m_error = BMP_IMG_NOT_FOUND;
        return;
    }

    fread(&header, sizeof(BITMAPFILEHEADER), 1, fPtr);
    fread(&info, sizeof(BITMAPINFOHEADER), 1, fPtr);

    if (memcmp(&header.bfType, "BM", 2)) 
    {
        m_error = BMP_IMG_WRONG_IMG_FMT;
        return; 
    }

    if (info.biBitCount != 24)
    {
        m_error = BMP_IMG_WRONG_PIX_FMT;
        return;
    }

    
    const int dwImageSize = info.biWidth * info.biHeight * 3;

    m_height = info.biHeight;
    m_width  = info.biWidth;
    m_pixels = new uint8_t[dwImageSize];
    
    fseek(fPtr, header.bfOffBits, SEEK_SET);

    auto ptr = m_pixels;
    size_t read = 0;
    for (int i = 0; i < m_height; ++i ) 
    {
        read += fread(&ptr[read], 1, m_width * 3, fPtr);
        auto mod = (m_width * 3 % 4);
        if (mod)
            fseek(fPtr, 4 - mod, SEEK_CUR);
    }

    if (read != dwImageSize)
    {
        m_error = BMP_IMG_CORRUPTED;
        return;
    }

    fclose(fPtr);

    m_error = BMP_IMG_OK;
}

BMPImage::~BMPImage() {
    if (m_pixels)
        delete[] m_pixels;
}

LRESULT WINAPI Window::s_wnd_proc(HWND win, UINT msg, WPARAM wparam, LPARAM lparam)
{
    //static short mx1 = 0, my1 = 0, mx2 = 0, my2 = 0;
    Window* window = (Window*)GetWindowLongPtrA(win, GWLP_USERDATA);
    if (msg == WM_NCCREATE)
    {
        SetWindowLongPtrA(win, GWLP_USERDATA, (LONG_PTR)((CREATESTRUCT*)lparam)->lpCreateParams);
    }

    if (window)
    {
        window->wnd_proc(win, msg, wparam, lparam);
    }

    return DefWindowProcA(win, msg, wparam, lparam);
}

LRESULT WINAPI Window::wnd_proc(HWND win, UINT msg, WPARAM wparam, LPARAM lparam)
{
    (void)wparam;

    static int mx0 = 0, my0 = 0;
    switch (msg)
    {
        case WM_CREATE:
            ZeroMemory(this->keys, 256);

            POINT p;
            GetCursorPos(&p);
            ScreenToClient(win, &p);

            mx0 = p.x;
            my0 = p.y;

            break;
        case WM_DESTROY:
            running = FALSE;
            PostQuitMessage(0);
            break;

        case WM_CLOSE:
            break;

        case WM_MOUSEMOVE:
            mx = (short)(lparam & 0xFFFFF);
            my = (short)((lparam >> 16) & 0xFFFFF);

            mdx = mx - mx0;
            mdy = my - my0;

            mx0 = mx;
            my0 = my;
            break;

        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_KEYUP:
        case WM_KEYDOWN:
            GetKeyboardState(keys);
            break;

        default:
            break;
    }

    return 0;
}

Window::Window(int32_t width, int32_t height, const char *const title)
{

    WNDCLASSEXA wclassex   = {};
    wclassex.cbSize        = sizeof(wclassex);
    wclassex.hInstance     = GetModuleHandleA(NULL);
    wclassex.lpfnWndProc   = s_wnd_proc;
    wclassex.lpszClassName = "App";
    wclassex.hCursor       = LoadCursor(NULL, IDC_ARROW);

    RegisterClassExA(&wclassex);

    #define STYLE WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX

    RECT srect = { 0, 0, width, height };
    AdjustWindowRect(&srect, STYLE, FALSE);

    window = CreateWindowExA(
            0,
            wclassex.lpszClassName,
            title,
            STYLE,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            srect.right - srect.left,
            srect.bottom - srect.top,
            NULL, NULL,
            wclassex.hInstance,
            this
    );

    #undef STYLE

    ShowWindow(window, SW_SHOW);
    UpdateWindow(window);

    device_ctx = GetDC(window);
    running      = TRUE;
    this->height = height;
    this->width  = width;
}

void Window::PollEvents()
{
    while (PeekMessageA(&message, window, 0, 0, PM_REMOVE)) {
        if (message.message == WM_CLOSE) 
            running = FALSE;

        TranslateMessage(&message);
        DispatchMessageA(&message);
    }
}

void Window::SetTitle(const char* title)
{
    SetWindowTextA(window, title);
}

BOOL Window::GetKeyState(uint8_t key)
{
    return keys[key] & 0x80;
}

Window::~Window()
{
    ReleaseDC(window, device_ctx);
    DestroyWindow(window);
}


Renderer::Renderer(const Window* window)
{
    m_deviceCtx = window->GetDCHandle();
    m_width      = window->GetWidth();
    height     = window->GetHeight();
    m_pixels     = new uint32_t[m_width * height];

    m_bitmapInfo.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    m_bitmapInfo.bmiHeader.biWidth       = m_width;
    m_bitmapInfo.bmiHeader.biHeight      = height;
    m_bitmapInfo.bmiHeader.biPlanes      = 1;
    m_bitmapInfo.bmiHeader.biCompression = BI_RGB;
    m_bitmapInfo.bmiHeader.biBitCount    = sizeof(*m_pixels)*8;
}

void Renderer::Present()
{
    if (!m_pixels) { puts("No buffer [pixels p]"); return ;}
    SetDIBitsToDevice(
            m_deviceCtx,
            0, 0,
            m_width,
            height,
            0, 0, 0,
            height,
            (void*)m_pixels,
            &m_bitmapInfo,
            DIB_RGB_COLORS
            );
}

void Renderer::ClearColour(uint32_t colour)
{
    if (!m_pixels) { puts("No buffer [pixels]"); return; }
    for (int i = 0; i < m_width * height; ++i)
        m_pixels[i] = colour;
}

void Renderer::DrawPixel(int32_t x, int32_t y, uint32_t colour)
{
    if ((x | y) < 0) return;
    if (x >= m_width || y >= height) return;

    m_pixels[x + y * m_width] = colour;
}

void Renderer::DrawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1,  uint32_t colour)
{
    int dy = abs(y1 - y0);
    int dx = abs(x1 - x0);

    int inc, norm;
    float slope;
    float carry;

    if (dx > dy)
    {
        slope = (float)dy / (float)dx;
        slope = y0 < y1 ? slope : -slope;
        norm = x0 < x1 ? 1 : -1;

        inc = x0; carry = (float)y0;

        for (int i = 0; i < dx; i++)
        {
            DrawPixel(inc, (int)carry, colour);
            carry += slope;
            inc += norm;
        }
    }
    else
    {
        slope = (float)dx / (float)dy;
        slope = x0 < x1 ? slope : -slope;
        norm = y0 < y1 ? 1 : -1;

        inc = y0; carry = (float)x0;

        for (int i = 0; i < dy; i++)
        {
            DrawPixel((int)carry, inc, colour);
            carry += slope;
            inc += norm;
        }
    }
}

void Renderer::DrawRect(int32_t x, int32_t y, int32_t w, int32_t h,  uint32_t colour)
{
    for (int i = x; i < x + w; ++i)
    {
        DrawPixel(i, y, colour);    
        DrawPixel(i, y + h, colour);    
    }

    for (int j = y; j < y + h; ++j)
    {
        DrawPixel(x, j, colour);
        DrawPixel(x + w, j, colour);
    }   
}

void Renderer::DrawCircle(int32_t x, int32_t y, int32_t r,  uint32_t colour)
{
    for (float i = PI/2; i < 3*PI/2; i += PI/(4*r))
    {
        float icos  = (float)r*cos(i);
        int32_t px1 = (int32_t)icos + x;
        int32_t px2 = (int32_t)-icos + x;
        int32_t py  = (int32_t)((float)r*sin(i)) + y;

        DrawPixel(px1, py, colour);
        DrawPixel(px2, py, colour);
    }
}

void Renderer::DrawRectFill(int32_t x, int32_t y, int32_t w, int32_t h,  uint32_t colour)
{
    for (int i = x; i < x + w; ++i)
        for (int j = y; j < y + h; ++j)
            DrawPixel(i, j, colour);
}

void Renderer::DrawCircleFill(int32_t x, int32_t y, int32_t r,  uint32_t colour)
{
    for (float i = PI/2; i < 3*PI/2; i += PI/(4*r))
    {
        float icos  = (float)r*cos(i);
        int32_t px1 = (int32_t)icos + x;
        int32_t px2 = (int32_t)-icos + x;
        int32_t py  = (int32_t)((float)r*sin(i)) + y;

        DrawLine(px1, py,  px2, py, colour);
    }
}

void Renderer::DrawRGBBitmap(int32_t x, int32_t y, int32_t w, int32_t h, const uint8_t *pixels)
{
    auto ptr = pixels;

    if (alphaKey.has_value()) 
    {
        auto alpha = alphaKey.value();
        for (int j = 0; j < h; ++j) 
        {
            for (int i = 0; i < w; ++i) 
            {
                auto colour = RGB(ptr[0], ptr[1], ptr[2]);
                if (colour != alpha)
                    DrawPixel(x + i, y + j, colour);
                ptr += 3;
            }
        }
    }
    else 
    {
        for (int j = 0; j < h; ++j) 
        {
            for (int i = 0; i < w; ++i) 
            {
                auto colour = RGB(ptr[0], ptr[1], ptr[2]);
                DrawPixel(x + i, y + j, colour);
                ptr += 3;
            }
        }
    }
}

void Renderer::SetBitmapAlphaKey(uint32_t colour) 
{
    alphaKey = std::make_optional<uint32_t>(colour);
}

void Renderer::ClearAlphaKey()
{
    alphaKey.reset();
    alphaKey = std::nullopt;
}

Renderer::~Renderer()
{
    delete[] m_pixels;
}

}

