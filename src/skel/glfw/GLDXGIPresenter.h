#pragma once

namespace Enhanced {
#if defined(_WIN32) && defined(RW_GL3) && !defined(LIBRW_SDL2)
bool DXGIFullscreenRequested();
bool PresentFullscreenDXGI(void *window, bool vsync);
void ShutdownFullscreenDXGI();
// Implemented by glfw.cpp to prevent a resize event re-entering rsIDLE.
bool SetPresentationResizeGuard(bool active);
void FallbackNativeFullscreen(void *window);
#else
inline bool DXGIFullscreenRequested() { return false; }
inline bool PresentFullscreenDXGI(void *, bool) { return false; }
inline void ShutdownFullscreenDXGI() {}
#endif
}
