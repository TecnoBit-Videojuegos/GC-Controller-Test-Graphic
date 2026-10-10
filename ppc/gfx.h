#ifndef GFX_H
#define GFX_H

#include <gccore.h>
#include "textures_gen.h"

#define SCREEN_W 640.0f
#define SCREEN_H 480.0f

#define RGBA(r, g, b, a)                                                       \
  (((u32)(r) << 24) | ((u32)(g) << 16) | ((u32)(b) << 8) | (u32)(a))

typedef struct {
  u16 buttons;
  s8 stickX, stickY;
  s8 cX, cY;
  u8 trigL, trigR;
} PadState;

void gfx_init(void);
void gfx_begin(void);
void gfx_end(void);

void gfx_rect(f32 x, f32 y, f32 w, f32 h, u32 color);
void gfx_ellipse(f32 cx, f32 cy, f32 rx, f32 ry, u32 color);
void gfx_ring(f32 cx, f32 cy, f32 rInner, f32 rOuter, u32 color);
void gfx_tex_draw(const gx_texdesc *t, f32 x, f32 y);
void gfx_tex_draw_rot(const gx_texdesc *t, f32 x, f32 y, int rot);

void gfx_draw_controller(const PadState *p, int activePad);

#endif
