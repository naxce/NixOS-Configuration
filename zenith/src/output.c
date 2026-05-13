#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <limits.h>

#include <gbm.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

#include "zenith.h"
#include "output.h"
#include "config.h"

static PFNEGLGETPLATFORMDISPLAYEXTPROC egl_get_platform_display = NULL;
static PFNEGLCREATEPLATFORMWINDOWSURFACEEXTPROC egl_create_platform_window = NULL;

static int find_drm_device(void) {
    DIR *d = opendir("/dev/dri");
    if (!d) return -1;
    struct dirent *ent;
    int fd = -1;
    while ((ent = readdir(d)) != NULL) {
        if (strncmp(ent->d_name, "card", 4) != 0) continue;
        char path[128];
        snprintf(path, sizeof(path), "/dev/dri/%s", ent->d_name);
        fd = open(path, O_RDWR | O_CLOEXEC | O_NONBLOCK);
        if (fd < 0) continue;
        drmModeRes *res = drmModeGetResources(fd);
        if (!res) { close(fd); fd = -1; continue; }
        if (res->count_connectors > 0) {
            drmModeFreeResources(res);
            break;
        }
        drmModeFreeResources(res);
        close(fd);
        fd = -1;
    }
    closedir(d);
    return fd;
}

static int init_egl(struct zenith_output *out) {
    egl_get_platform_display = (PFNEGLGETPLATFORMDISPLAYEXTPROC)
        eglGetProcAddress("eglGetPlatformDisplayEXT");
    egl_create_platform_window = (PFNEGLCREATEPLATFORMWINDOWSURFACEEXTPROC)
        eglGetProcAddress("eglCreatePlatformWindowSurfaceEXT");

    if (egl_get_platform_display) {
        out->egl_display = egl_get_platform_display(EGL_PLATFORM_GBM_KHR, out->gbm, NULL);
    } else {
        out->egl_display = eglGetDisplay((EGLNativeDisplayType)out->gbm);
    }
    if (out->egl_display == EGL_NO_DISPLAY) {
        fprintf(stderr, "zenith: eglGetDisplay failed\n");
        return 0;
    }

    EGLint major, minor;
    if (!eglInitialize(out->egl_display, &major, &minor)) {
        fprintf(stderr, "zenith: eglInitialize failed\n");
        return 0;
    }

    eglBindAPI(EGL_OPENGL_ES_API);

    static const EGLint cfg_attribs[] = {
        EGL_SURFACE_TYPE,    EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_RED_SIZE,   8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE,  8,
        EGL_ALPHA_SIZE, 0,
        EGL_NONE
    };
    EGLint n;
    if (!eglChooseConfig(out->egl_display, cfg_attribs, &out->egl_config, 1, &n) || n < 1) {
        fprintf(stderr, "zenith: eglChooseConfig failed\n");
        return 0;
    }

    out->gbm_surface = gbm_surface_create(out->gbm, out->width, out->height,
        GBM_FORMAT_XRGB8888, GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING);
    if (!out->gbm_surface) {
        fprintf(stderr, "zenith: gbm_surface_create failed\n");
        return 0;
    }

    if (egl_create_platform_window) {
        out->egl_surface = egl_create_platform_window(out->egl_display,
            out->egl_config, out->gbm_surface, NULL);
    } else {
        out->egl_surface = eglCreateWindowSurface(out->egl_display,
            out->egl_config, (EGLNativeWindowType)out->gbm_surface, NULL);
    }
    if (out->egl_surface == EGL_NO_SURFACE) {
        fprintf(stderr, "zenith: eglCreateWindowSurface failed\n");
        return 0;
    }

    static const EGLint ctx_attribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };
    out->egl_context = eglCreateContext(out->egl_display, out->egl_config,
        EGL_NO_CONTEXT, ctx_attribs);
    if (out->egl_context == EGL_NO_CONTEXT) {
        fprintf(stderr, "zenith: eglCreateContext failed\n");
        return 0;
    }

    if (!eglMakeCurrent(out->egl_display, out->egl_surface, out->egl_surface, out->egl_context)) {
        fprintf(stderr, "zenith: eglMakeCurrent failed\n");
        return 0;
    }

    return 1;
}

static int setup_output(struct zenith_server *server, struct zenith_output *out,
                        int drm_fd, drmModeConnector *conn, drmModeRes *res) {
    out->server = server;
    out->drm_fd = drm_fd;
    out->connector = conn;
    out->connector_id = conn->connector_id;

    snprintf(out->name, sizeof(out->name), "connector-%u", conn->connector_id);
    out->scale = 1.0f;
    out->enabled = 1;
    out->needs_redraw = 1;

    for (int i = 0; i < conn->count_modes; i++) {
        if (conn->modes[i].type & DRM_MODE_TYPE_PREFERRED) {
            out->mode = conn->modes[i];
            break;
        }
    }
    if (out->mode.hdisplay == 0 && conn->count_modes > 0)
        out->mode = conn->modes[0];

    out->width  = out->mode.hdisplay;
    out->height = out->mode.vdisplay;
    out->refresh = out->mode.vrefresh;

    for (int i = 0; i < conn->count_encoders; i++) {
        drmModeEncoder *enc = drmModeGetEncoder(drm_fd, conn->encoders[i]);
        if (!enc) continue;
        for (int j = 0; j < res->count_crtcs; j++) {
            if (!(enc->possible_crtcs & (1 << j))) continue;
            out->crtc = drmModeGetCrtc(drm_fd, res->crtcs[j]);
            out->crtc_id = res->crtcs[j];
            drmModeFreeEncoder(enc);
            goto found_crtc;
        }
        drmModeFreeEncoder(enc);
    }
    fprintf(stderr, "zenith: no usable crtc for connector %u\n", conn->connector_id);
    return 0;
found_crtc:

    out->gbm = gbm_create_device(drm_fd);
    if (!out->gbm) {
        fprintf(stderr, "zenith: gbm_create_device failed\n");
        return 0;
    }

    if (!init_egl(out)) {
        fprintf(stderr, "zenith: EGL init failed\n");
        return 0;
    }

    glViewport(0, 0, out->width, out->height);
    glClearColor(0.07f, 0.07f, 0.07f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    eglSwapBuffers(out->egl_display, out->egl_surface);

    struct gbm_bo *bo = gbm_surface_lock_front_buffer(out->gbm_surface);
    if (!bo) {
        fprintf(stderr, "zenith: failed to lock front buffer\n");
        return 0;
    }
    uint32_t stride = gbm_bo_get_stride(bo);
    uint32_t handle = gbm_bo_get_handle(bo).u32;
    uint32_t fb = 0;
    if (drmModeAddFB(drm_fd, out->width, out->height, 24, 32, stride, handle, &fb)) {
        fprintf(stderr, "zenith: drmModeAddFB failed\n");
        return 0;
    }
    if (drmModeSetCrtc(drm_fd, out->crtc_id, fb, 0, 0, &out->connector_id, 1, &out->mode)) {
        fprintf(stderr, "zenith: drmModeSetCrtc failed\n");
        return 0;
    }
    out->front_bo = bo;
    out->front_fb = fb;

    return 1;
}

int zenith_output_init(struct zenith_server *server) {
    server->drm_fd = find_drm_device();
    if (server->drm_fd < 0) {
        fprintf(stderr, "zenith: no DRM device found\n");
        return 0;
    }

    drmSetMaster(server->drm_fd);

    drmModeRes *res = drmModeGetResources(server->drm_fd);
    if (!res) {
        fprintf(stderr, "zenith: drmModeGetResources failed\n");
        return 0;
    }

    int x_offset = 0;
    for (int i = 0; i < res->count_connectors && server->output_count < ZENITH_MAX_OUTPUTS; i++) {
        drmModeConnector *conn = drmModeGetConnector(server->drm_fd, res->connectors[i]);
        if (!conn) continue;
        if (conn->connection != DRM_MODE_CONNECTED || conn->count_modes == 0) {
            drmModeFreeConnector(conn);
            continue;
        }
        struct zenith_output *out = calloc(1, sizeof(*out));
        if (!out) { drmModeFreeConnector(conn); continue; }
        if (!setup_output(server, out, server->drm_fd, conn, res)) {
            free(out);
            drmModeFreeConnector(conn);
            continue;
        }
        out->x = x_offset;
        out->y = 0;
        x_offset += out->width;
        server->outputs[server->output_count++] = out;
    }

    drmModeFreeResources(res);
    return server->output_count > 0;
}

void zenith_output_render(struct zenith_output *out) {
    if (!out->enabled) return;
    eglMakeCurrent(out->egl_display, out->egl_surface, out->egl_surface, out->egl_context);
    glViewport(0, 0, out->width, out->height);
    glClearColor(0.07f, 0.07f, 0.07f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    eglSwapBuffers(out->egl_display, out->egl_surface);

    if (out->front_bo) {
        gbm_surface_release_buffer(out->gbm_surface, out->front_bo);
    }
    out->front_bo = gbm_surface_lock_front_buffer(out->gbm_surface);
    if (!out->front_bo) return;
    uint32_t stride = gbm_bo_get_stride(out->front_bo);
    uint32_t handle = gbm_bo_get_handle(out->front_bo).u32;
    uint32_t fb = 0;
    drmModeAddFB(out->drm_fd, out->width, out->height, 24, 32, stride, handle, &fb);
    drmModePageFlip(out->drm_fd, out->crtc_id, fb, DRM_MODE_PAGE_FLIP_EVENT, NULL);
    if (out->front_fb) drmModeRmFB(out->drm_fd, out->front_fb);
    out->front_fb = fb;
    out->needs_redraw = 0;
}

void zenith_output_destroy(struct zenith_server *server) {
    for (int i = 0; i < server->output_count; i++) {
        struct zenith_output *out = server->outputs[i];
        if (!out) continue;
        if (out->egl_display != EGL_NO_DISPLAY) {
            eglMakeCurrent(out->egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
            if (out->egl_context != EGL_NO_CONTEXT)
                eglDestroyContext(out->egl_display, out->egl_context);
            if (out->egl_surface != EGL_NO_SURFACE)
                eglDestroySurface(out->egl_display, out->egl_surface);
            eglTerminate(out->egl_display);
        }
        if (out->gbm_surface) gbm_surface_destroy(out->gbm_surface);
        if (out->gbm) gbm_device_destroy(out->gbm);
        if (out->crtc) drmModeFreeCrtc(out->crtc);
        if (out->connector) drmModeFreeConnector(out->connector);
        free(out);
        server->outputs[i] = NULL;
    }
    server->output_count = 0;
    if (server->drm_fd >= 0) {
        drmDropMaster(server->drm_fd);
        close(server->drm_fd);
        server->drm_fd = -1;
    }
}
