#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <strsafe.h>
#include <d3d9.h>
#include <d3dx9.h>

typedef HRESULT (WINAPI *PFN_D3DXCreateFontW)(
  LPDIRECT3DDEVICE9 pDevice,
  INT height,
  UINT width,
  UINT weight,
  UINT miplevels,
  BOOL italic,
  DWORD charset,
  DWORD outputPrecision,
  DWORD quality,
  DWORD pitchAndFamily,
  LPCWSTR pFacename,
  LPD3DXFONT *ppFont);

// Global variables
static LPDIRECT3D9 g_d3d = nullptr;
static LPDIRECT3DDEVICE9 g_d3dDevice = nullptr;
static LPD3DXFONT g_font = nullptr;
static HMODULE g_d3dxModule = nullptr;
static PFN_D3DXCreateFontW g_pfnCreateFontW = nullptr;
static WCHAR g_d3dxModulePath[MAX_PATH] = L"";

// Function declarations
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
bool InitD3D(HWND hwnd);
void CleanD3D();
void RenderFrame();
bool EnsureD3DXModule(void);
void UpdateWindowTitle(HWND hwnd);

// Entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
  // Window class
  WNDCLASSEX wc = {sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, L"DX9WindowClass", NULL};
  RegisterClassEx(&wc);

  // Create the window
  HWND hwnd = CreateWindow(
    L"DX9WindowClass", L"DirectX9 Hello World", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, 100, 100, 800, 600, NULL, NULL, wc.hInstance, NULL
  );

  ShowWindow(hwnd, nCmdShow);
  UpdateWindow(hwnd);

  // Initialize Direct3D
  if (!InitD3D(hwnd))
  {
    DestroyWindow(hwnd);
    UnregisterClass(L"DX9WindowClass", wc.hInstance);
    return 0;
  }

  // Main message loop
  MSG msg = {0};
  while (msg.message != WM_QUIT) {
    if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    } else {
      RenderFrame();
    }
  }

  CleanD3D();
  UnregisterClass(L"DX9WindowClass", wc.hInstance);

  return 0;
}

// Initialize Direct3D
bool InitD3D(HWND hwnd)
{
  g_d3d = Direct3DCreate9(D3D_SDK_VERSION);
  if (!g_d3d)
  {
    MessageBoxW(hwnd, L"Direct3DCreate9 failed", L"Error", MB_ICONERROR);
    return false;
  }

  D3DPRESENT_PARAMETERS d3dpp = {};
  d3dpp.Windowed = TRUE;
  d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
  d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;

  HRESULT hr = g_d3d->CreateDevice(
    D3DADAPTER_DEFAULT,
    D3DDEVTYPE_HAL,
    hwnd,
    D3DCREATE_SOFTWARE_VERTEXPROCESSING,
    &d3dpp,
    &g_d3dDevice);
  if (FAILED(hr))
  {
    MessageBoxW(hwnd, L"CreateDevice failed", L"Error", MB_ICONERROR);
    return false;
  }

  if (!EnsureD3DXModule())
  {
    MessageBoxW(hwnd, L"D3DX9_42.dll is required for this repro.", L"D3DX missing", MB_ICONERROR);
    return false;
  }

  hr = g_pfnCreateFontW(
    g_d3dDevice,
    20,
    0,
    FW_BOLD,
    1,
    FALSE,
    DEFAULT_CHARSET,
    OUT_DEFAULT_PRECIS,
    DEFAULT_QUALITY,
    DEFAULT_PITCH | FF_DONTCARE,
    L"Tahoma", // Or Arial
    &g_font);
  if (FAILED(hr))
  {
    MessageBoxW(hwnd, L"D3DXCreateFontW failed", L"Error", MB_ICONERROR);
    return false;
  }

  UpdateWindowTitle(hwnd);
  return true;
}

// Render frame
void RenderFrame()
{
  if (!g_d3dDevice)
    return;

  g_d3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(40, 40, 60), 1.0f, 0);
  if (FAILED(g_d3dDevice->BeginScene()))
    return;

  RECT rect;
  SetRect(&rect, 10, 10, 760, 560);

  if (g_font)
  {
    g_font->DrawTextW(
      nullptr,
      L"* Connected! [MTA:SA Server 1.6 [Windows]]\n"
      L"* AfraidStation80 has joined the game\n"
      L"Welcome to Freeroam\n"
      L"Press F1 to show/hide controls\n"
      L"Say: wwwwTTTT",
      -1,
      &rect,
      DT_LEFT | DT_TOP | DT_NOCLIP,
      D3DCOLOR_XRGB(255, 255, 255));
  }

  g_d3dDevice->EndScene();
  g_d3dDevice->Present(NULL, NULL, NULL, NULL);
}

// Clean up
void CleanD3D()
{
  if (g_font)
  {
    g_font->Release();
    g_font = nullptr;
  }
  if (g_d3dDevice)
  {
    g_d3dDevice->Release();
    g_d3dDevice = nullptr;
  }
  if (g_d3d)
  {
    g_d3d->Release();
    g_d3d = nullptr;
  }
  if (g_d3dxModule)
  {
    FreeLibrary(g_d3dxModule);
    g_d3dxModule = nullptr;
    g_pfnCreateFontW = nullptr;
    g_d3dxModulePath[0] = L'\0';
  }
}

// Window procedure
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch (msg) {
    case WM_DESTROY:
      PostQuitMessage(0);
      return 0;
    case WM_KEYDOWN:
      if (wParam == VK_ESCAPE)
        DestroyWindow(hwnd);
      return 0;
  }
  return DefWindowProc(hwnd, msg, wParam, lParam);
}

bool EnsureD3DXModule(void)
{
  if (g_pfnCreateFontW)
    return true;

  if (!g_d3dxModule)
    g_d3dxModule = LoadLibraryW(L"D3DX9_42.dll");

  if (!g_d3dxModule)
    return false;

  g_pfnCreateFontW = (PFN_D3DXCreateFontW)GetProcAddress(g_d3dxModule, "D3DXCreateFontW");
  if (!g_pfnCreateFontW)
  {
    FreeLibrary(g_d3dxModule);
    g_d3dxModule = nullptr;
    return false;
  }

  if (!GetModuleFileNameW(g_d3dxModule, g_d3dxModulePath, ARRAYSIZE(g_d3dxModulePath)))
    lstrcpynW(g_d3dxModulePath, L"D3DX9_42.dll", ARRAYSIZE(g_d3dxModulePath));

  return true;
}

void UpdateWindowTitle(HWND hwnd)
{
  if (!IsWindow(hwnd))
    return;

  WCHAR caption[512];
  if (g_d3dxModulePath[0])
  {
    StringCchPrintfW(
      caption,
      ARRAYSIZE(caption),
      L"DirectX9 font repro (D3DX source: %s)",
      g_d3dxModulePath);
  }
  else
  {
    lstrcpynW(caption, L"DirectX9 font repro (D3DX not loaded)", ARRAYSIZE(caption));
  }
  SetWindowTextW(hwnd, caption);
}
