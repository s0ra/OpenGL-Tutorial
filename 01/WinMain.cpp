#include <windows.h>
#include <GL/GL.h>
#include <windowsx.h>
//#include <dxgi.h>
//#include <d2d1.h>
//#include <d3dcommon.h>
//#include <d3d11.h>
//#include <d3dcompiler.h>
//#include <DirectXMath.h>
//#include <dwrite.h>
#include <wingdi.h>
//#include "glext.h"
#include <stdio.h>

// See https://www.opengl.org/registry/specs/ARB/wgl_create_context.txt for all values
#define WGL_CONTEXT_MAJOR_VERSION_ARB             0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB             0x2092
#define WGL_CONTEXT_PROFILE_MASK_ARB              0x9126

#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB          0x00000001

typedef HGLRC WINAPI wglCreateContextAttribsARB_type(HDC hdc, HGLRC hShareContext,
                                                     const int *attribList);
wglCreateContextAttribsARB_type *wglCreateContextAttribsARB;

typedef bool WINAPI wglChoosePixelFormatARB_type(HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats);
wglChoosePixelFormatARB_type *wglChoosePixelFormatARB;

// See https://www.opengl.org/registry/specs/ARB/wgl_pixel_format.txt for all values
#define WGL_DRAW_TO_WINDOW_ARB                    0x2001
#define WGL_ACCELERATION_ARB                      0x2003
#define WGL_SUPPORT_OPENGL_ARB                    0x2010
#define WGL_DOUBLE_BUFFER_ARB                     0x2011
#define WGL_PIXEL_TYPE_ARB                        0x2013
#define WGL_COLOR_BITS_ARB                        0x2014
#define WGL_DEPTH_BITS_ARB                        0x2022
#define WGL_STENCIL_BITS_ARB                      0x2023

#define WGL_FULL_ACCELERATION_ARB                 0x2027
#define WGL_TYPE_RGBA_ARB                         0x202B


bool up = false;
bool down = false;
bool left = false;
bool right = false;

int dir_count = 0;

int choosen_level = -1;
int level = 1;

bool undo = false;
bool restart = false;
bool failed = false;

float mouse_x = 0.0f;
float mouse_y = 0.0f;

bool clicking = false;

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);


void* GetAnyGLFuncAddress(const char *name);
void ASSERT(bool, char*, HWND);


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASSA window_class = {0};
    
    window_class.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    window_class.lpfnWndProc = DefWindowProcA;
    window_class.hInstance = GetModuleHandle(0);
    window_class.lpszClassName = "Dummy_WGL_djuasiodwa";
    
    RegisterClassA(&window_class);
    
    HWND dummy_window = CreateWindowExA(0, window_class.lpszClassName, "Dummy OpenGL Window", 0, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, window_class.hInstance, 0);
    PIXELFORMATDESCRIPTOR pfd = {0};
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.cColorBits = 32;
    pfd.cAlphaBits = 8;
    pfd.iLayerType = PFD_MAIN_PLANE;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 8;
    
    
    HDC dummy_dc = GetDC(dummy_window);
    
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.cColorBits = 32;
    pfd.cAlphaBits = 8;
    pfd.iLayerType = PFD_MAIN_PLANE;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 8;
    
    int _pixel_format = ChoosePixelFormat(dummy_dc, &pfd);
    
    SetPixelFormat(dummy_dc,_pixel_format, &pfd);
    HGLRC dummy_context = wglCreateContext(dummy_dc);
    wglMakeCurrent(dummy_dc, dummy_context);
    
    //wglGetProcAddress("wglCreateContextAttribsARB");
    wglChoosePixelFormatARB = (wglChoosePixelFormatARB_type*)GetAnyGLFuncAddress("wglChoosePixelFormatARB");
    wglCreateContextAttribsARB = (wglCreateContextAttribsARB_type*)GetAnyGLFuncAddress("wglCreateContextAttribsARB");
    //wglGetProcAddress("wglChoosePixelFormatARB");
    
    wglMakeCurrent(dummy_dc, 0);
    wglDeleteContext(dummy_context);
    ReleaseDC(dummy_window, dummy_dc);
    DestroyWindow(dummy_window);
    
    const auto pClassName = "hw3dgame";
    WNDCLASSEX wc = { 0 };
    wc.cbSize = sizeof( wc );
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = WindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = nullptr;
    wc.hCursor = nullptr;
    wc.hbrBackground = nullptr;
    wc.lpszMenuName = nullptr;
    wc.lpszClassName = pClassName;
    wc.hIconSm = nullptr;
	RegisterClassEx( &wc );
    HWND hWnd = CreateWindowEx(0,pClassName, "Game Project", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, 200,200,1920, 1080, nullptr, nullptr, hInstance, nullptr);
    
    
    
    
    FILE* fp;
    
    //freopen_s(&fp, "CONOUT$", "w", stdout);
    
    // Obtain the size of the drawing area.
    //RECT rc;
    //GetClientRect(hWnd, &rc);
    
    HDC    hdc;
    HGLRC  hglrc;
    
    
    // Now we can choose a pixel format the modern way, using wglChoosePixelFormatARB.
    int pixel_format_attribs[] =
    {
        WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
        WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
        WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB, 
        WGL_COLOR_BITS_ARB, 32, 
        WGL_DEPTH_BITS_ARB, 24, 
        WGL_STENCIL_BITS_ARB, 8, 0
    };
    
	ShowWindow(hWnd,SW_SHOW);
    
    HDC real_dc = GetDC(hWnd);
    
    int pixel_format;
    UINT num_formats;
    
    
    wglChoosePixelFormatARB(real_dc, pixel_format_attribs, 0, 1, &pixel_format, &num_formats);
    
    
    PIXELFORMATDESCRIPTOR pfd1;
    DescribePixelFormat(real_dc, pixel_format, sizeof(pfd1), &pfd1);
    SetPixelFormat(real_dc, pixel_format, &pfd1);
    
    // Specify that we want to create an OpenGL 3.3 core profile context
    int gl33_attribs[] =
    {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
        WGL_CONTEXT_MINOR_VERSION_ARB, 3,
        WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0,
    };
    
    HGLRC gl33_context = wglCreateContextAttribsARB(real_dc, 0, gl33_attribs);
    
    wglMakeCurrent(real_dc, gl33_context);
    
    
    
    LARGE_INTEGER StartingTime, EndingTime, ticks;
    LARGE_INTEGER Frequency;
    
    ticks.QuadPart = 0;
    
    // message pump
    MSG msg = {0};
    while(WM_QUIT != msg.message)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
        else
        {
            QueryPerformanceFrequency(&Frequency);
            QueryPerformanceCounter(&StartingTime);
            
            
            
            QueryPerformanceCounter(&EndingTime);
            ticks.QuadPart += ((EndingTime.QuadPart - StartingTime.QuadPart) * 1000000) / Frequency.QuadPart;
        }
        
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        SwapBuffers(real_dc);
    }
    
    wglMakeCurrent (NULL, NULL) ; 
    wglDeleteContext (gl33_context);
    
    //FreeConsole(); // Console related
    return 0;
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;
    
    int temp_dir_count = dir_count;
    
    switch (uMsg)
    {
        
        /*
        case WM_SIZE:
        {
            int width = LOWORD(lParam);  // Macro to get the low-order word.
            int height = HIWORD(lParam); // Macro to get the high-order word.
            // Respond to the message:
            //OnSize(hwnd, (UINT)wParam, width, height);
            return 0;
        }
        */
        case WM_CREATE:
        {
            //glGetString = GetAnyGLFuncAddress("glGetString");
            //MessageBoxA(0, (char*)glGetIntegerv(GL_MAJOR_VERSION), "OPENGL VERSION", 0);
        }
        
        case WM_MOUSEMOVE:
        {
            mouse_x = GET_X_LPARAM(lParam);
            mouse_y = GET_Y_LPARAM(lParam);
            break;
        }
        
        case WM_LBUTTONUP:
        {
            mouse_x = GET_X_LPARAM(lParam);
            mouse_y = GET_Y_LPARAM(lParam);
            clicking = false;
            break;
        }
        
        case WM_LBUTTONDOWN:
        {
            mouse_x = GET_X_LPARAM(lParam);
            mouse_y = GET_Y_LPARAM(lParam);
            clicking = true;
            break;
        }
        
        case WM_KEYDOWN:
        {
            if (wParam == VK_ESCAPE)
            {
                SendMessage(hWnd, WM_CLOSE, 0, 0);
            }
            if (wParam == VK_SHIFT)
            {
                undo = true;
            }
            
            if (wParam == 0x57 || wParam == VK_UP && up == false)
            {
                up = true;
            }
            if (wParam == 0x53 || wParam == VK_DOWN && down == false)
            {
                down = true;
            }
            if (wParam == 0x41 || wParam == VK_LEFT && left == false)
            {
                left = true;
            }
            if (wParam == 0x44 || wParam == VK_RIGHT && right == false)
            {
                right = true;
            }
            if (wParam == 0x52 && restart == false)
            {
                restart = true;
            }
        }
        
        /* case WM_KEYUP:
        {
            
            if (wParam == VK_ESCAPE)
            {
                restart = false;
            }
            if (wParam == VK_SHIFT)
            {
                undo = false;
            }
            if (wParam == 0x57 || wParam == VK_UP && up)
            {
                up = false;
            }
            if (wParam == 0x53 || wParam == VK_DOWN && down)
            {
                down = false;
            }
            if (wParam == 0x41 || wParam == VK_LEFT && left)
            {
                left = false;
            }
            if (wParam == 0x44 || wParam == VK_RIGHT && right)
            {
                right = false;
            }
        } */
        
        case WM_PAINT:
        {
            BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
            break;
        }
        
        case WM_CLOSE:
        {
            DestroyWindow(hWnd);
            return 0;
        }
        case WM_DESTROY:
        {
            PostQuitMessage(0);
            return 0;
        }
    }
    
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void* GetAnyGLFuncAddress(const char *name)
{
    void *p = (void *)wglGetProcAddress(name);
    if(p == 0 ||
       (p == (void*)0x1) || (p == (void*)0x2) || (p == (void*)0x3) ||
       (p == (void*)-1) )
    {
        HMODULE module = LoadLibraryA("opengl32.dll");
        p =  (void *)GetProcAddress(module, name);
    }
    return p;
}

void ASSERT(bool must_be_true, char* caption, HWND hWnd)
{
    if (!must_be_true)
    {
        MessageBox(hWnd, "Assertion Failed", caption, 0);
    }
}
