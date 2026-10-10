#include "gfx.h"
#include <malloc.h>
#include <math.h>
#include <string.h>
#include "textures_gen.h"

#define FIFO_SIZE (256 * 1024)
#define SEGMENTS 28
#define PI 3.14159265f

static GXRModeObj *rmode = NULL;
static void *frameBuffer[2] = {NULL, NULL};
static u32 fbIndex = 0;
static int firstFrame = 1;

static inline void vertex(f32 x, f32 y, u32 color) {
  GX_Position3f32(x, y, 0.0f);
  GX_Color1u32(color);
}

void gfx_init(void) {
  GXColor background = {16, 16, 24, 255};
  Mtx44 proj;
  Mtx mv;

  VIDEO_Init();
  rmode = VIDEO_GetPreferredMode(NULL);
  frameBuffer[0] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
  frameBuffer[1] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));

  VIDEO_Configure(rmode);
  VIDEO_SetNextFramebuffer(frameBuffer[fbIndex]);
  VIDEO_SetBlack(TRUE);
  VIDEO_Flush();
  VIDEO_WaitVSync();
  if (rmode->viTVMode & VI_NON_INTERLACE)
    VIDEO_WaitVSync();

  void *fifo = memalign(32, FIFO_SIZE);
  memset(fifo, 0, FIFO_SIZE);
  GX_Init(fifo, FIFO_SIZE);

  GX_SetCopyClear(background, GX_MAX_Z24);
  GX_SetViewport(0, 0, rmode->fbWidth, rmode->efbHeight, 0, 1);
  f32 yscale = GX_GetYScaleFactor(rmode->efbHeight, rmode->xfbHeight);
  u32 xfbHeight = GX_SetDispCopyYScale(yscale);
  GX_SetScissor(0, 0, rmode->fbWidth, rmode->efbHeight);
  GX_SetDispCopySrc(0, 0, rmode->fbWidth, rmode->efbHeight);
  GX_SetDispCopyDst(rmode->fbWidth, xfbHeight);
  GX_SetCopyFilter(rmode->aa, rmode->sample_pattern, GX_TRUE, rmode->vfilter);
  GX_SetFieldMode(rmode->field_rendering,
                  ((rmode->viHeight == 2 * rmode->xfbHeight) ? GX_ENABLE
                                                             : GX_DISABLE));
  GX_SetCullMode(GX_CULL_NONE);
  GX_SetDispCopyGamma(GX_GM_1_0);

  // Vertex format: position + color
  GX_ClearVtxDesc();
  GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
  GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
  GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
  GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
  GX_SetNumChans(1);
  GX_SetNumTexGens(0);
  GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORDNULL, GX_TEXMAP_NULL, GX_COLOR0A0);
  GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
  GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);

  // 2D projection with a virtual 640x480 screen
  guOrtho(proj, 0, SCREEN_H, 0, SCREEN_W, 0, 300);
  GX_LoadProjectionMtx(proj, GX_ORTHOGRAPHIC);
  guMtxIdentity(mv);
  GX_LoadPosMtxImm(mv, GX_PNMTX0);
}

void gfx_begin(void) {
  GX_SetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
  GX_SetColorUpdate(GX_TRUE);
}

void gfx_end(void) {
  GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
  GX_SetColorUpdate(GX_TRUE);
  GX_CopyDisp(frameBuffer[fbIndex], GX_TRUE);
  GX_DrawDone();
  VIDEO_SetNextFramebuffer(frameBuffer[fbIndex]);
  if (firstFrame) {
    VIDEO_SetBlack(FALSE);
    firstFrame = 0;
  }
  VIDEO_Flush();
  VIDEO_WaitVSync();
  fbIndex ^= 1;
}

void gfx_rect(f32 x, f32 y, f32 w, f32 h, u32 color) {
  GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
  vertex(x, y, color);
  vertex(x + w, y, color);
  vertex(x + w, y + h, color);
  vertex(x, y + h, color);
  GX_End();
}

void gfx_ellipse(f32 cx, f32 cy, f32 rx, f32 ry, u32 color) {
  GX_Begin(GX_TRIANGLEFAN, GX_VTXFMT0, SEGMENTS + 2);
  vertex(cx, cy, color);
  for (int i = 0; i <= SEGMENTS; i++) {
    f32 a = (2.0f * PI * i) / SEGMENTS;
    vertex(cx + cosf(a) * rx, cy + sinf(a) * ry, color);
  }
  GX_End();
}

void gfx_ring(f32 cx, f32 cy, f32 rInner, f32 rOuter, u32 color) {
  GX_Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, (SEGMENTS + 1) * 2);
  for (int i = 0; i <= SEGMENTS; i++) {
    f32 a = (2.0f * PI * i) / SEGMENTS;
    vertex(cx + cosf(a) * rOuter, cy + sinf(a) * rOuter, color);
    vertex(cx + cosf(a) * rInner, cy + sinf(a) * rInner, color);
  }
  GX_End();
}

static void tex_begin(const gx_texdesc *t) {
  GXTexObj obj;
  GX_InitTexObj(&obj, (void *)t->data, t->tw, t->th, GX_TF_RGBA8, GX_CLAMP,
                GX_CLAMP, GX_FALSE);
  GX_InitTexObjFilterMode(&obj, GX_NEAR, GX_NEAR);
  GX_LoadTexObj(&obj, GX_TEXMAP0);

  GX_ClearVtxDesc();
  GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
  GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);
  GX_SetVtxAttrFmt(GX_VTXFMT1, GX_VA_POS, GX_POS_XY, GX_F32, 0);
  GX_SetVtxAttrFmt(GX_VTXFMT1, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
  GX_SetNumTexGens(1);
  GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
  GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLORNULL);
  GX_SetTevOp(GX_TEVSTAGE0, GX_REPLACE);
}

static void tex_end(void) {
  // Dejar todo como estaba para que las formas normales sigan funcionando
  GX_ClearVtxDesc();
  GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
  GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
  GX_SetNumTexGens(0);
  GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORDNULL, GX_TEXMAP_NULL, GX_COLOR0A0);
  GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
}

// rot: 0 = normal, 1 = girada a la derecha, 2 = abajo, 3 = izquierda
void gfx_tex_draw_rot(const gx_texdesc *t, f32 x, f32 y, int rot) {
  f32 u = (f32)t->w / t->tw;
  f32 v = (f32)t->h / t->th;
  f32 U[4] = {0, u, u, 0};
  f32 V[4] = {0, 0, v, v};
  f32 px[4] = {x, x + t->w, x + t->w, x};
  f32 py[4] = {y, y, y + t->h, y + t->h};

  tex_begin(t);
  GX_Begin(GX_QUADS, GX_VTXFMT1, 4);
  for (int i = 0; i < 4; i++) {
    int k = (i - rot + 4) & 3;
    GX_Position2f32(px[i], py[i]);
    GX_TexCoord2f32(U[k], V[k]);
  }
  GX_End();
  tex_end();
}

void gfx_tex_draw(const gx_texdesc *t, f32 x, f32 y) {
  gfx_tex_draw_rot(t, x, y, 0);
}
