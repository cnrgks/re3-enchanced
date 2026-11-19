#define WITHWINDOWS
#include "common.h"
#include "GLDXGIPresenter.h"

#if defined(_WIN32) && defined(RW_GL3) && !defined(LIBRW_SDL2)
#include <d3d11.h>
#include <dxgi1_2.h>
#include <cstdarg>
#include <cstdio>
#include "Enhanced.h"
#include "Frontend.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

namespace Enhanced {
bool DXGIComposedFullscreen=true;

namespace {
typedef HANDLE (WINAPI *OpenInterop)(void *);
typedef BOOL (WINAPI *CloseInterop)(HANDLE);
typedef HANDLE (WINAPI *RegisterInterop)(HANDLE,void *,GLuint,GLenum,GLenum);
typedef BOOL (WINAPI *UnregisterInterop)(HANDLE,HANDLE);
typedef BOOL (WINAPI *LockInterop)(HANDLE,GLint,HANDLE *);
typedef HRESULT (WINAPI *CreateD3D)(IDXGIAdapter *,D3D_DRIVER_TYPE,HMODULE,UINT,
    const D3D_FEATURE_LEVEL *,UINT,UINT,const DXGI_SWAP_CHAIN_DESC *,IDXGISwapChain **,
    ID3D11Device **,D3D_FEATURE_LEVEL *,ID3D11DeviceContext **);
const GLenum AccessWriteDiscard=0x0002; // WGL_ACCESS_WRITE_DISCARD_NV

struct Presenter {
    GLFWwindow *window;
    bool failed, fallbackPending, suspended, locked, busy, latencyLimited, oldErrorLogged;
    HMODULE library;
    ID3D11Device *device;
    ID3D11DeviceContext *context;
    IDXGISwapChain *chain;
    IDXGIOutput *output;
    ID3D11Texture2D *backBuffer, *interopTexture;
    DXGI_MODE_DESC mode;
    UINT swapFlags;
    HANDLE interopDevice, interopObject;
    GLuint sharedBuffer, sharedFbo, resolveTexture, resolveFbo;
    int width,height;
    OpenInterop open;
    CloseInterop close;
    RegisterInterop registerObject;
    UnregisterInterop unregisterObject;
    LockInterop lock,unlock;
} state={};

void Log(const char *format,...)
{
    FILE *file=fopen("re3-display.log","a");
    if (!file) return;
    fputs("v1.9 dxgi: ",file);
    va_list args;
    va_start(args,format);
    vfprintf(file,format,args);
    va_end(args);
    fputc('\n',file);
    fclose(file);
}
template<class T> void Release(T *&value)
{
    if (value) { value->Release(); value=nil; }
}
struct ResizeGuard {
    bool previous;
    ResizeGuard() : previous(SetPresentationResizeGuard(true)) {}
    ~ResizeGuard() { SetPresentationResizeGuard(previous); }
};
struct PresentGuard {
    PresentGuard() { state.busy=true; }
    ~PresentGuard() { state.busy=false; }
};
// librw caches GL bindings. Restore the exact bindings after our GPU copy so
// its cache remains valid; no game rendering is redirected or patched.
struct GLBindings {
    GLint read,draw,renderbuffer,texture,defaultRead;
    GLboolean scissor,srgb;
    GLBindings() {
        glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING,&read);
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING,&draw);
        glGetIntegerv(GL_RENDERBUFFER_BINDING,&renderbuffer);
        glGetIntegerv(GL_TEXTURE_BINDING_2D,&texture);
        scissor=glIsEnabled(GL_SCISSOR_TEST);
        srgb=glIsEnabled(GL_FRAMEBUFFER_SRGB);
        glBindFramebuffer(GL_READ_FRAMEBUFFER,0);
        glGetIntegerv(GL_READ_BUFFER,&defaultRead);
        glDisable(GL_SCISSOR_TEST);
        glDisable(GL_FRAMEBUFFER_SRGB);
    }
    ~GLBindings() {
        glBindFramebuffer(GL_READ_FRAMEBUFFER,0);
        glReadBuffer(defaultRead);
        glBindFramebuffer(GL_READ_FRAMEBUFFER,read);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER,draw);
        glBindRenderbuffer(GL_RENDERBUFFER,renderbuffer);
        glBindTexture(GL_TEXTURE_2D,texture);
        if (scissor) glEnable(GL_SCISSOR_TEST);
        if (srgb) glEnable(GL_FRAMEBUFFER_SRGB);
    }
};

void ReleaseBuffers()
{
    if (state.locked && state.unlock) {
        state.unlock(state.interopDevice,1,&state.interopObject);
        state.locked=false;
    }
    if (state.interopObject && state.unregisterObject)
        state.unregisterObject(state.interopDevice,state.interopObject);
    state.interopObject=nil;
    if (state.sharedFbo) glDeleteFramebuffers(1,&state.sharedFbo);
    if (state.resolveFbo) glDeleteFramebuffers(1,&state.resolveFbo);
    if (state.sharedBuffer) glDeleteRenderbuffers(1,&state.sharedBuffer);
    if (state.resolveTexture) glDeleteTextures(1,&state.resolveTexture);
    state.sharedFbo=state.resolveFbo=state.sharedBuffer=state.resolveTexture=0;
    Release(state.interopTexture);
    Release(state.backBuffer);
    state.width=state.height=0;
}
void ReleaseDevice()
{
    ReleaseBuffers();
    if (state.interopDevice && state.close) state.close(state.interopDevice);
    state.interopDevice=nil;
    if (state.chain && !DXGIComposedFullscreen) state.chain->SetFullscreenState(FALSE,nil);
    if (state.context) { state.context->ClearState(); state.context->Flush(); }
    Release(state.chain);
    Release(state.output);
    Release(state.context);
    Release(state.device);
    if (state.library) { FreeLibrary(state.library); state.library=nil; }
}
bool Fail(const char *where, HRESULT result=E_FAIL, bool loading=false)
{
    Log("%s failed hr=0x%08lX; returning to native OpenGL %s",
        where,(unsigned long)result,DXGIComposedFullscreen ? "borderless" : "fullscreen");
    ResizeGuard guard;
    ReleaseDevice();
    state.failed=true;
    if (!DXGIComposedFullscreen) {
        if (loading)
            state.fallbackPending=true;
        else
            FallbackNativeFullscreen(state.window);
    }
    return false;
}

bool CreateBuffers(int width,int height)
{
    ReleaseBuffers();
    HRESULT hr=state.chain->ResizeBuffers(1,width,height,DXGI_FORMAT_R8G8B8A8_UNORM,
        state.swapFlags);
    if (FAILED(hr)) { Log("ResizeBuffers hr=0x%08lX",(unsigned long)hr); return false; }
    hr=state.chain->GetBuffer(0,__uuidof(ID3D11Texture2D),(void**)&state.backBuffer);
    if (FAILED(hr)) return false;
    // Keep WGL ownership transfers off the swap-chain surface.
    D3D11_TEXTURE2D_DESC textureDesc;
    state.backBuffer->GetDesc(&textureDesc);
    textureDesc.Usage=D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags=D3D11_BIND_RENDER_TARGET;
    textureDesc.CPUAccessFlags=0;
    textureDesc.MiscFlags=0;
    hr=state.device->CreateTexture2D(&textureDesc,nil,&state.interopTexture);
    if (FAILED(hr)) { Log("CreateTexture2D hr=0x%08lX",(unsigned long)hr); return false; }
    GLBindings restore;
    glGenRenderbuffers(1,&state.sharedBuffer);
    state.interopObject=state.registerObject(state.interopDevice,state.interopTexture,
        state.sharedBuffer,GL_RENDERBUFFER,AccessWriteDiscard);
    if (!state.interopObject) { Log("wglDXRegisterObjectNV error=%lu",GetLastError()); return false; }
    if (!state.lock(state.interopDevice,1,&state.interopObject)) return false;
    state.locked=true;
    glGenFramebuffers(1,&state.sharedFbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER,state.sharedFbo);
    glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_RENDERBUFFER,state.sharedBuffer);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    const bool complete=glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE;
    const bool unlocked=state.unlock(state.interopDevice,1,&state.interopObject)!=FALSE;
    state.locked=!unlocked;
    if (!complete || !unlocked) return false;

    // A separate GL target resolves WGL MSAA before the optional Y flip into
    // the shared DXGI buffer. There is no glReadPixels or CPU frame upload.
    glGenTextures(1,&state.resolveTexture);
    glBindTexture(GL_TEXTURE_2D,state.resolveTexture);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,width,height,0,GL_RGBA,GL_UNSIGNED_BYTE,nil);
    glGenFramebuffers(1,&state.resolveFbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER,state.resolveFbo);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,state.resolveTexture,0);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    if (glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE) return false;
    state.width=width; state.height=height;
    return true;
}

bool EnterFullscreen()
{
    ResizeGuard guard;
    const double started=glfwGetTime();
    if (state.swapFlags & DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH) {
        ReleaseBuffers();
        const HRESULT hr=state.chain->ResizeTarget(&state.mode);
        if (hr!=S_OK) { Log("ResizeTarget hr=0x%08lX",(unsigned long)hr); return false; }
    }
    HRESULT hr=state.chain->SetFullscreenState(TRUE,state.output);
    if (hr!=S_OK) { Log("SetFullscreenState hr=0x%08lX",(unsigned long)hr); return false; }
    BOOL fullscreen=FALSE;
    state.chain->GetFullscreenState(&fullscreen,nil);
    if (!fullscreen) return false; // Never label a windowed fallback as exclusive.
    int width,height;
    glfwGetFramebufferSize(state.window,&width,&height);
    if (width<=0 || height<=0) return false;
    // This discard-model chain can keep its buffers across a same-size transition.
    const bool reuse=state.backBuffer && state.interopObject &&
        width==state.width && height==state.height;
    if (!reuse && !CreateBuffers(width,height)) return false;
    state.suspended=false;
    Log("active fullscreen=1 transfer=texture-copy framebuffer=%dx%d modeSwitch=%d flipY=%d latencyLimitApplied=%d reusedBuffers=%d enterMs=%.2f",
        width,height,int(state.swapFlags!=0),int(FullscreenDXGIFlipY),int(state.latencyLimited),
        int(reuse),1000.0*(glfwGetTime()-started));
    return true;
}

bool Initialise(GLFWwindow *window, bool loading)
{
    if (!GLAD_GL_VERSION_3_0 || !glBlitFramebuffer ||
        !glfwExtensionSupported("WGL_NV_DX_interop2")) return false;
    state.open=(OpenInterop)glfwGetProcAddress("wglDXOpenDeviceNV");
    state.close=(CloseInterop)glfwGetProcAddress("wglDXCloseDeviceNV");
    state.registerObject=(RegisterInterop)glfwGetProcAddress("wglDXRegisterObjectNV");
    state.unregisterObject=(UnregisterInterop)glfwGetProcAddress("wglDXUnregisterObjectNV");
    state.lock=(LockInterop)glfwGetProcAddress("wglDXLockObjectsNV");
    state.unlock=(LockInterop)glfwGetProcAddress("wglDXUnlockObjectsNV");
    if (!state.open || !state.close || !state.registerObject || !state.unregisterObject || !state.lock || !state.unlock)
        return false;
    int width,height;
    glfwGetFramebufferSize(window,&width,&height);
    if (width<=0 || height<=0) return false;
    DXGI_SWAP_CHAIN_DESC desc={};
    desc.BufferDesc.Width=width; desc.BufferDesc.Height=height;
    desc.BufferDesc.Format=DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count=1;
    desc.BufferUsage=DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.BufferCount=1;
    desc.OutputWindow=glfwGetWin32Window(window);
    desc.Windowed=TRUE; // Microsoft recommends an explicit transition after creation.
    desc.SwapEffect=DXGI_SWAP_EFFECT_DISCARD;
    MONITORINFOEXW monitor={};
    monitor.cbSize=sizeof(monitor);
    DEVMODEW desktop={};
    desktop.dmSize=sizeof(desktop);
    const HMONITOR target=MonitorFromWindow(desc.OutputWindow,MONITOR_DEFAULTTONEAREST);
    const bool desktopKnown=!DXGIComposedFullscreen &&
        GetMonitorInfoW(target,reinterpret_cast<MONITORINFO*>(&monitor)) &&
        EnumDisplaySettingsW(monitor.szDevice,ENUM_CURRENT_SETTINGS,&desktop);
    if (desktopKnown &&
        (desktop.dmPelsWidth!=UINT(FrontEndMenuManager.m_nPrefsWidth) ||
         desktop.dmPelsHeight!=UINT(FrontEndMenuManager.m_nPrefsHeight)))
        state.swapFlags=DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    desc.Flags=state.swapFlags;
    D3D_FEATURE_LEVEL level;
    ResizeGuard guard;
    // Optional system component, not a new mandatory DLL import. Machines
    // without it keep the native OpenGL path, including older Windows.
    WCHAR systemPath[MAX_PATH];
    const UINT length=GetSystemDirectoryW(systemPath,MAX_PATH);
    const WCHAR suffix[]=L"\\d3d11.dll";
    if (!length || length+ARRAY_SIZE(suffix)>=MAX_PATH) return false;
    memcpy(systemPath+length,suffix,sizeof(suffix));
    state.library=LoadLibraryW(systemPath);
    if (!state.library) return false;
    CreateD3D create=(CreateD3D)GetProcAddress(state.library,"D3D11CreateDeviceAndSwapChain");
    if (!create) return false;
    HRESULT hr=create(nil,D3D_DRIVER_TYPE_HARDWARE,nil,
        D3D11_CREATE_DEVICE_BGRA_SUPPORT,nil,0,D3D11_SDK_VERSION,&desc,&state.chain,
        &state.device,&level,&state.context);
    if (FAILED(hr)) { Log("CreateDeviceAndSwapChain hr=0x%08lX",(unsigned long)hr); return false; }
    // The DX device must match the active OpenGL GPU; opening interop checks it.
    state.interopDevice=state.open(state.device);
    if (!state.interopDevice) { Log("wglDXOpenDeviceNV error=%lu",GetLastError()); return false; }
    IDXGIDevice1 *latency=nil;
    if (SUCCEEDED(state.device->QueryInterface(__uuidof(IDXGIDevice1),(void**)&latency))) {
        state.latencyLimited=SUCCEEDED(latency->SetMaximumFrameLatency(1));
        latency->Release();
    }
    if (DXGIComposedFullscreen) {
        IDXGIFactory *factory=nil;
        hr=state.chain->GetParent(__uuidof(IDXGIFactory),(void**)&factory);
        if (FAILED(hr)) return false;
        hr=factory->MakeWindowAssociation(desc.OutputWindow,DXGI_MWA_NO_ALT_ENTER);
        factory->Release();
        if (FAILED(hr)) return false;
        if (!CreateBuffers(width,height)) return false;
        Log("active fullscreen=0 composed=1 transfer=texture-copy framebuffer=%dx%d",width,height);
        return true;
    }
    hr=state.chain->GetContainingOutput(&state.output);
    if (FAILED(hr)) return false;
    if (state.swapFlags) {
        DXGI_MODE_DESC desired={};
        desired.Width=FrontEndMenuManager.m_nPrefsWidth;
        desired.Height=FrontEndMenuManager.m_nPrefsHeight;
        desired.Format=DXGI_FORMAT_R8G8B8A8_UNORM;
        hr=state.output->FindClosestMatchingMode(&desired,&state.mode,state.device);
        if (FAILED(hr)) return false;
        state.mode.RefreshRate=DXGI_RATIONAL{};
    }
    if (!loading)
        return EnterFullscreen();
    if (!CreateBuffers(width,height))
        return false;
    state.suspended=true;
    Log("loading presentation windowed framebuffer=%dx%d",width,height);
    return true;
}

bool CopyFrame()
{
    // Do not misattribute an engine GL error to the interop copy.
    for (int i=0;i<8;++i) {
        const GLenum error=glGetError();
        if (error==GL_NO_ERROR) break;
        if (!state.oldErrorLogged) { Log("pre-existing engine GL error=0x%X",error); state.oldErrorLogged=true; }
    }
    GLBindings restore;
    if (!state.lock(state.interopDevice,1,&state.interopObject)) return false;
    state.locked=true;
    glBindFramebuffer(GL_READ_FRAMEBUFFER,0);
    glReadBuffer(GL_BACK);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER,state.resolveFbo);
    glBlitFramebuffer(0,0,state.width,state.height,0,0,state.width,state.height,GL_COLOR_BUFFER_BIT,GL_NEAREST);
    glBindFramebuffer(GL_READ_FRAMEBUFFER,state.resolveFbo);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER,state.sharedFbo);
    const int bottom=FullscreenDXGIFlipY ? state.height : 0;
    const int top=FullscreenDXGIFlipY ? 0 : state.height;
    glBlitFramebuffer(0,0,state.width,state.height,0,bottom,state.width,top,GL_COLOR_BUFFER_BIT,GL_NEAREST);
    const GLenum error=glGetError();
    glFlush();
    const bool unlocked=state.unlock(state.interopDevice,1,&state.interopObject)!=FALSE;
    state.locked=!unlocked;
    if (error!=GL_NO_ERROR) Log("GPU blit GL error=0x%X",error);
    return error==GL_NO_ERROR && unlocked;
}
}

bool DXGIFullscreenRequested() { return ScreenFormat==0; }
bool DXGIComposedFullscreenRequested() { return DXGIComposedFullscreen && DXGIFullscreenRequested(); }

void SuspendFullscreenForLoading(void *handle)
{
    if (DXGIComposedFullscreen || !handle || state.window!=handle ||
        !state.chain || state.suspended || state.busy)
        return;

    ResizeGuard guard;
    const double started=glfwGetTime();
    if (state.swapFlags)
        ReleaseBuffers();
    const HRESULT hr=state.chain->SetFullscreenState(FALSE,nil);
    if (hr!=S_OK) {
        Log("exclusive exit deferred hr=0x%08lX",(unsigned long)hr);
        return;
    }
    state.suspended=true;
    Log("suspended fullscreen iconified=%d exitMs=%.2f",
        glfwGetWindowAttrib((GLFWwindow*)handle,GLFW_ICONIFIED),1000.0*(glfwGetTime()-started));
}

void UpdateFullscreenFocus(void *handle)
{
    GLFWwindow *window=(GLFWwindow*)handle;
    if (!window)
        return;
    if (GetForegroundWindow() == glfwGetWin32Window(window) &&
        glfwGetWindowAttrib(window,GLFW_FOCUSED) && !glfwGetWindowAttrib(window,GLFW_ICONIFIED))
        return;
    SuspendFullscreenForLoading(window);
}

void ShutdownFullscreenDXGI()
{
    if (state.device || state.interopDevice) {
        ResizeGuard guard;
        ReleaseDevice();
        Log("shutdown");
    }
    state=Presenter{};
}

bool PresentFullscreenDXGI(void *handle, bool vsync, bool loading, bool trace)
{
    if (!DXGIFullscreenRequested()) return false;
    if (state.busy) return true;
    GLFWwindow *window=(GLFWwindow*)handle;
    if (state.window!=window) { ShutdownFullscreenDXGI(); state.window=window; }
    if (state.failed) {
        if (state.fallbackPending && !loading) {
            state.fallbackPending=false;
            FallbackNativeFullscreen(window);
            return true; // The mode switch invalidates the current frame dimensions.
        }
        return false;
    }
    if (GetForegroundWindow() != glfwGetWin32Window(window) || !glfwGetWindowAttrib(window,GLFW_FOCUSED) ||
        glfwGetWindowAttrib(window,GLFW_ICONIFIED)) {
        UpdateFullscreenFocus(window);
        return true;
    }
    if (loading && !DXGIComposedFullscreen && state.chain && !state.suspended) {
        SuspendFullscreenForLoading(window);
        if (!state.suspended)
            return true;
    }
    PresentGuard presenting;
    int renderedWidth,renderedHeight;
    glfwGetFramebufferSize(window,&renderedWidth,&renderedHeight);
    if (!state.device && !Initialise(window,loading)) return Fail("initialise/interop/fullscreen",E_FAIL,loading);
    if (!DXGIComposedFullscreen && !loading) {
        BOOL fullscreen=FALSE;
        const HRESULT hr=state.chain->GetFullscreenState(&fullscreen,nil);
        if (FAILED(hr)) return Fail("GetFullscreenState",hr);
        if (state.suspended || !fullscreen) {
            if (trace) Log("focus t=%.3f phase=exclusive_enter_begin",glfwGetTime());
            if (!EnterFullscreen()) return Fail("resume fullscreen");
            if (trace) Log("focus t=%.3f phase=exclusive_enter_end",glfwGetTime());
        }
    }
    int width,height;
    glfwGetFramebufferSize(window,&width,&height);
    // Skip a frame drawn against the pre-transition framebuffer size.
    if (width<=0 || height<=0 || width!=renderedWidth || height!=renderedHeight)
        return true;
    if (width!=state.width || height!=state.height) {
        ResizeGuard guard;
        if (!CreateBuffers(width,height)) return Fail("resize buffers",E_FAIL,loading);
    }
    if (trace) Log("focus t=%.3f phase=interop_copy_begin",glfwGetTime());
    if (!CopyFrame()) return Fail("GPU copy/interop",E_FAIL,loading);
    state.context->CopyResource(state.backBuffer,state.interopTexture);
    if (trace) Log("focus t=%.3f phase=dxgi_present_begin",glfwGetTime());
    const HRESULT hr=state.chain->Present(vsync ? 1 : 0,0);
    if (trace) Log("focus t=%.3f phase=dxgi_present_end hr=0x%08lX",glfwGetTime(),(unsigned long)hr);
    if (FAILED(hr)) return Fail("Present",hr,loading);
    return true;
}
}
#endif
