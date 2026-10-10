#include "gfx.h"
#include <gccore.h>
#include <stdlib.h>

#define C_PANEL RGBA(0x2A, 0x24, 0x52, 0xFF)
#define C_DARK RGBA(0x1B, 0x17, 0x36, 0xFF)
#define C_GATE RGBA(0x4B, 0x43, 0x90, 0xFF)
#define C_GREY_ON RGBA(0xE0, 0xE0, 0xE0, 0xFF)
#define C_GREY_OFF RGBA(0x6A, 0x6A, 0x7A, 0xFF)
#define C_A_ON RGBA(0x2E, 0xE0, 0x7A, 0xFF)
#define C_A_OFF RGBA(0x1E, 0x7A, 0x45, 0xFF)
#define C_B_ON RGBA(0xFF, 0x4D, 0x4D, 0xFF)
#define C_B_OFF RGBA(0x8A, 0x2A, 0x2A, 0xFF)
#define C_C_ON RGBA(0xFF, 0xE0, 0x30, 0xFF)
#define C_C_OFF RGBA(0x9A, 0x84, 0x1A, 0xFF)
#define C_Z_ON RGBA(0x9B, 0x7B, 0xFF, 0xFF)
#define C_Z_OFF RGBA(0x4A, 0x3A, 0x8A, 0xFF)
#define C_STICK_ON RGBA(0xF0, 0xF0, 0xF0, 0xFF)
#define C_STICK_OFF RGBA(0x90, 0x90, 0xA0, 0xFF)
#define C_TRIG_FILL RGBA(0x9A, 0xA0, 0xFF, 0xFF)

#define GLOW(c) (((c) & 0xFFFFFF00u) | 0x66u)

// Round/oval button with a "pressed" effect: shrinks slightly and glows
static void draw_button(f32 cx, f32 cy, f32 rx, f32 ry, u32 on, u32 off,
                        int pressed) {
  gfx_ellipse(cx, cy, rx * 1.12f, ry * 1.12f, C_DARK);
  if (pressed) {
    gfx_ellipse(cx, cy, rx * 1.4f, ry * 1.4f, GLOW(on));
    gfx_ellipse(cx, cy, rx * 0.92f, ry * 0.92f, on);
  } else {
    gfx_ellipse(cx, cy, rx, ry, off);
  }
}

static void draw_stick(f32 cx, f32 cy, f32 gateR, f32 knobR, s8 sx, s8 sy,
                       f32 scale, u32 on, u32 off) {
  f32 ox = sx * scale;
  f32 oy = -sy * scale; // screen Y goes down
  int moved = (abs(sx) > 12 || abs(sy) > 12);

  gfx_ellipse(cx, cy, gateR, gateR, C_DARK);
  gfx_ring(cx, cy, gateR - 4.0f, gateR, C_GATE);
  if (moved)
    gfx_ellipse(cx + ox, cy + oy, knobR * 1.3f, knobR * 1.3f, GLOW(on));
  gfx_ellipse(cx + ox, cy + oy, knobR, knobR, moved ? on : off);
}

static void draw_dpad(f32 cx, f32 cy, u16 b) {
  const f32 t = 24.0f; // arm thickness
  const f32 a = 28.0f; // arm length
  u32 off = C_GREY_OFF;
  u32 on = C_GREY_ON;

  gfx_rect(cx - t / 2 - 3, cy - t / 2 - a - 3, t + 6, t + 2 * a + 6, C_DARK);
  gfx_rect(cx - t / 2 - a - 3, cy - t / 2 - 3, t + 2 * a + 6, t + 6, C_DARK);

  gfx_rect(cx - t / 2, cy - t / 2 - a, t, a,
           (b & PAD_BUTTON_UP) ? on : off);
  gfx_rect(cx - t / 2, cy + t / 2, t, a, (b & PAD_BUTTON_DOWN) ? on : off);
  gfx_rect(cx - t / 2 - a, cy - t / 2, a, t,
           (b & PAD_BUTTON_LEFT) ? on : off);
  gfx_rect(cx + t / 2, cy - t / 2, a, t, (b & PAD_BUTTON_RIGHT) ? on : off);
  gfx_rect(cx - t / 2, cy - t / 2, t, t, off);
}

// Analog trigger bar, fills with the analog value; glows when fully pressed
static void draw_trigger(f32 x, f32 y, f32 w, f32 h, u8 value, int digital,
                         int fromRight) {
  f32 fill = w * (f32)value / 255.0f;

  if (digital)
    gfx_rect(x - 5, y - 5, w + 10, h + 10, GLOW(0xFFFFFFFFu));
  gfx_rect(x, y, w, h, C_DARK);
  if (fromRight)
    gfx_rect(x + w - fill, y, fill, h, C_TRIG_FILL);
  else
    gfx_rect(x, y, fill, h, C_TRIG_FILL);
}

void gfx_draw_controller(const PadState *p, int activePad) {
  u16 b = p->buttons;

  // Body
  gfx_rect(30, 70, 580, 370, C_PANEL);

  // Triggers
  draw_trigger(50, 20, 200, 28, p->trigL, b & PAD_TRIGGER_L, 0);
  draw_trigger(390, 20, 200, 28, p->trigR, b & PAD_TRIGGER_R, 1);

  // Z
  gfx_rect(520, 52, 70, 14, (b & PAD_TRIGGER_Z) ? C_Z_ON : C_Z_OFF);

  // Active controller lamps (1-4)
  for (int i = 0; i < 4; i++) {
    f32 x = 269.0f + i * 34.0f;
    draw_button(x, 110, 8, 8, C_A_ON, C_DARK, i == activePad);
  }

  // Main stick, D-pad
  draw_stick(150, 200, 62, 30, p->stickX, p->stickY, 0.45f, C_STICK_ON,
             C_STICK_OFF);
  draw_dpad(240, 350, b);

  // Start
  draw_button(320, 215, 14, 14, C_GREY_ON, C_GREY_OFF, b & PAD_BUTTON_START);

  // Face buttons
  draw_button(500, 220, 40, 40, C_A_ON, C_A_OFF, b & PAD_BUTTON_A);
  draw_button(440, 262, 24, 24, C_B_ON, C_B_OFF, b & PAD_BUTTON_B);
  draw_button(568, 205, 15, 28, C_GREY_ON, C_GREY_OFF, b & PAD_BUTTON_X);
  draw_button(505, 150, 28, 15, C_GREY_ON, C_GREY_OFF, b & PAD_BUTTON_Y);

  // C-stick
  draw_stick(410, 345, 42, 20, p->cX, p->cY, 0.32f, C_C_ON, C_C_OFF);
}
