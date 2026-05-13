#ifndef ZENITH_OUTPUT_H
#define ZENITH_OUTPUT_H

#include <gbm.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <xf86drmMode.h>

#define ZENITH_MAX_OUTPUTS 8

struct zenith_server;

struct zenith_output {
    struct zenith_server *server;
    int drm_fd;

    drmModeConnector *connector;
    drmModeEncoder *encoder;
    drmModeCrtc *crtc;
    drmModeModeInfo mode;

    uint32_t connector_id;
    uint32_t crtc_id;

    struct gbm_device *gbm;
    struct gbm_surface *gbm_surface;

    EGLDisplay egl_display;
    EGLSurface egl_surface;
    EGLContext egl_context;
    EGLConfig egl_config;

    struct gbm_bo *front_bo;
    uint32_t front_fb;

    int width, height;
    int x, y;
    float scale;
    int refresh;
    int enabled;
    char name[64];

    GLuint framebuffer;
    GLuint renderbuffer;
    GLuint texture;

    int needs_redraw;
};

int  zenith_output_init(struct zenith_server *server);
void zenith_output_destroy(struct zenith_server *server);
void zenith_output_render(struct zenith_output *output);
int  zenith_output_apply_config(struct zenith_output *output);
void zenith_output_enumerate(struct zenith_server *server);

#endif
