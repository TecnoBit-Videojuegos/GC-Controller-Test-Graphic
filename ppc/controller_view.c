#include "gfx.h"
#include <gccore.h>

// Posición del mando en pantalla (centrado en 640x480)
#define BODY_X 144.0f
#define BODY_Y 107.0f

// Posiciones de cada pieza, relativas a la esquina del cuerpo
#define START_X 165
#define START_Y 80
#define A_X 259
#define A_Y 66
#define B_X 228
#define B_Y 95
#define X_X 308
#define X_Y 61
#define Y_X 251
#define Y_Y 39
#define DPAD_X 92
#define DPAD_Y 142
#define L_X 34
#define L_Y 1
#define R_X 259
#define R_Y 1
#define Z_X 264
#define Z_Y 9
#define STICK_MAIN_X 44
#define STICK_MAIN_Y 67
#define STICK_C_X 217
#define STICK_C_Y 153

// Cuánto se mueven los sticks (en píxeles) al llegar al tope
#define STICK_RANGE 20

// Dibuja una pieza en una posición relativa al cuerpo
static void piece(const gx_texdesc *t, f32 x, f32 y) {
  gfx_tex_draw(t, BODY_X + x, BODY_Y + y);
}

// Botón: versión "pulsado" o versión "en reposo"
static void button(const gx_texdesc *idle, const gx_texdesc *down, int pressed,
                   f32 x, f32 y) {
  piece(pressed ? down : idle, x, y);
}

// Gatillo: pulsado a fondo, o relleno si hay valor analógico, o reposo
static void trigger(const gx_texdesc *idle, const gx_texdesc *down,
                    const gx_texdesc *fill, int digital, u8 analog, f32 x,
                    f32 y) {
  if (digital)
    piece(down, x, y);
  else if (analog > 0)
    piece(fill, x, y);
  else
    piece(idle, x, y);
}

// Stick: una sola pieza que se desplaza según el valor
static void stick(const gx_texdesc *cap, s8 sx, s8 sy, f32 x, f32 y) {
  f32 dx = (f32)sx * STICK_RANGE / 128.0f;
  f32 dy = -(f32)sy * STICK_RANGE / 128.0f; // en pantalla Y va hacia abajo
  piece(cap, x + dx, y + dy);
}

void gfx_draw_controller(const PadState *p, int activePad) {
  u16 b = p->buttons;

  // Luces del mando activo (1-4), arriba del mando
  for (int i = 0; i < 4; i++) {
    f32 x = 269.0f + i * 34.0f;
    gfx_ellipse(x, 60, 8, 8,
                (i == activePad) ? RGBA(0x2E, 0xE0, 0x7A, 0xFF)
                                 : RGBA(0x1B, 0x17, 0x36, 0xFF));
  }

  // Cuerpo del mando (primero, para que lo demás quede encima)
  piece(&tex_pad_body, 0, 0);

  // Gatillos y Z
  trigger(&tex_trig_l_idle, &tex_trig_l_down, &tex_trig_l_fill,
          b & PAD_TRIGGER_L, p->trigL, L_X, L_Y);
  trigger(&tex_trig_r_idle, &tex_trig_r_down, &tex_trig_r_fill,
          b & PAD_TRIGGER_R, p->trigR, R_X, R_Y);
  button(&tex_btn_z_idle, &tex_btn_z_down, b & PAD_TRIGGER_Z, Z_X, Z_Y);

  // Botones
  button(&tex_btn_start_idle, &tex_btn_start_down, b & PAD_BUTTON_START,
         START_X, START_Y);
  button(&tex_btn_a_idle, &tex_btn_a_down, b & PAD_BUTTON_A, A_X, A_Y);
  button(&tex_btn_b_idle, &tex_btn_b_down, b & PAD_BUTTON_B, B_X, B_Y);
  button(&tex_btn_x_idle, &tex_btn_x_down, b & PAD_BUTTON_X, X_X, X_Y);
  button(&tex_btn_y_idle, &tex_btn_y_down, b & PAD_BUTTON_Y, Y_X, Y_Y);

  // Cruceta: siempre en reposo, y encima la flecha pulsada girada
  piece(&tex_dpad_idle, DPAD_X, DPAD_Y);
  if (b & PAD_BUTTON_UP)
    gfx_tex_draw_rot(&tex_dpad_down, BODY_X + DPAD_X, BODY_Y + DPAD_Y, 0);
  if (b & PAD_BUTTON_RIGHT)
    gfx_tex_draw_rot(&tex_dpad_down, BODY_X + DPAD_X, BODY_Y + DPAD_Y, 1);
  if (b & PAD_BUTTON_DOWN)
    gfx_tex_draw_rot(&tex_dpad_down, BODY_X + DPAD_X, BODY_Y + DPAD_Y, 2);
  if (b & PAD_BUTTON_LEFT)
    gfx_tex_draw_rot(&tex_dpad_down, BODY_X + DPAD_X, BODY_Y + DPAD_Y, 3);

  // Sticks (los últimos, para que queden por encima)
  stick(&tex_stick_main_cap, p->stickX, p->stickY, STICK_MAIN_X, STICK_MAIN_Y);
  stick(&tex_stick_c_cap, p->cX, p->cY, STICK_C_X, STICK_C_Y);
}

  // Mensaje de salida, debajo del mando
  gfx_tex_draw(&tex_msg_exit, (SCREEN_W - tex_msg_exit.w) / 2, 395);
