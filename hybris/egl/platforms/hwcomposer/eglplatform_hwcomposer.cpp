#include <android-config.h>
#include <ws.h>
#include "hwcomposer_window.h"
#include <malloc.h>
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>
extern "C" {
#include <eglplatformcommon.h>
};

#include "logging.h"

#include <hybris/gralloc/gralloc.h>

static HWComposerNativeWindow *_nativewindowPrimary = NULL;
static HWComposerNativeWindow *_nativewindowExternal = NULL;

extern "C" void hwcomposerws_init_module(struct ws_egl_interface *egl_iface)
{
    hybris_gralloc_initialize(0);
	eglplatformcommon_init(egl_iface);
}

extern "C" _EGLDisplay *hwcomposerws_GetDisplay(EGLNativeDisplayType display)
{
	_EGLDisplay *dpy = 0;
	if (display == EGL_DEFAULT_DISPLAY) {
		dpy = new _EGLDisplay;
	}
	return dpy;
}

extern "C" void hwcomposerws_Terminate(_EGLDisplay *dpy)
{
	delete dpy;
}

extern "C" EGLNativeWindowType hwcomposerws_CreateWindow(EGLNativeWindowType win, _EGLDisplay *display)
{
	HWComposerNativeWindow *window = static_cast<HWComposerNativeWindow *>((ANativeWindow *) win);

	if (_nativewindowPrimary == NULL) {
		_nativewindowPrimary = window;
		_nativewindowPrimary->common.incRef(&_nativewindowPrimary->common);
		return (EGLNativeWindowType) static_cast<struct ANativeWindow *>(_nativewindowPrimary);
	} else if (_nativewindowExternal == NULL) {
		_nativewindowExternal = window;
		_nativewindowExternal->common.incRef(&_nativewindowExternal->common);
		return (EGLNativeWindowType) static_cast<struct ANativeWindow *>(_nativewindowExternal);
	} else {
		return NULL;
	}
}

extern "C" void hwcomposerws_DestroyWindow(EGLNativeWindowType win)
{
	if (static_cast<HWComposerNativeWindow *>((struct ANativeWindow *)win) == _nativewindowPrimary) {
		_nativewindowPrimary->common.decRef(&_nativewindowPrimary->common);
		/* We are done with it, refcounting will delete the window when appropriate */
		_nativewindowPrimary = NULL;
	} else if (static_cast<HWComposerNativeWindow *>((struct ANativeWindow *)win) == _nativewindowExternal) {
		_nativewindowExternal->common.decRef(&_nativewindowExternal->common);
		/* We are done with it, refcounting will delete the window when appropriate */
		_nativewindowExternal = NULL;
	} else {
		assert(false);
	}
}

extern "C" __eglMustCastToProperFunctionPointerType hwcomposerws_eglGetProcAddress(const char *procname) 
{
	return eglplatformcommon_eglGetProcAddress(procname);
}

extern "C" void hwcomposerws_passthroughImageKHR(EGLContext *ctx, EGLenum *target, EGLClientBuffer *buffer, const EGLint **attrib_list)
{
	eglplatformcommon_passthroughImageKHR(ctx, target, buffer, attrib_list);
}

struct ws_module ws_module_info = {
	hwcomposerws_init_module,
	hwcomposerws_GetDisplay,
	hwcomposerws_Terminate,
	hwcomposerws_CreateWindow,
	hwcomposerws_DestroyWindow,
	hwcomposerws_eglGetProcAddress,
	hwcomposerws_passthroughImageKHR,
	eglplatformcommon_eglQueryString
};

// vim:ts=4:sw=4:noexpandtab
