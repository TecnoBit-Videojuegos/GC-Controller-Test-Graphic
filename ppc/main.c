#include <stdio.h>
#include <stdlib.h>
#include <gccore.h>

#include "../common/common_utils.h"
#include "gfx.h"
#include "ppc_utils.h"

// Combinación para salir: Start + Z + L + R a la vez
#define EXIT_COMBO                                                             \
  (PAD_BUTTON_START | PAD_TRIGGER_Z | PAD_TRIGGER_L | PAD_TRIGGER_R)
// Cuántos cuadros hay que mantenerla (unos 2 segundos)
#define EXIT_FRAMES 120

int main(int argc, char **argv) {
  gfx_init();
  PAD_Init();

  int activePad = 0;
  int exitCounter = 0;
  u16 keysHeld[4] = {0, 0, 0, 0};
  u16 keysHeldOld[4] = {0, 0, 0, 0};

  while (1) {
    PAD_ScanPads();

    for (int i = 0; i < 4; i++) {
      PAD_ControlMotor(i, 0); // stop rumble
      keysHeld[i] = PAD_ButtonsHeld(i);
    }

    PadState st = {keysHeld[activePad],        PAD_StickX(activePad),
                   PAD_StickY(activePad),      PAD_SubStickX(activePad),
                   PAD_SubStickY(activePad),   PAD_TriggerL(activePad),
                   PAD_TriggerR(activePad)};

    gfx_begin();
    gfx_draw_controller(&st, activePad);
    gfx_end(); // also waits for vsync

    // Special actions
    if (keysHeld[activePad] & PAD_BUTTON_A &&
        keysHeldOld[activePad] & PAD_BUTTON_B) {
      PAD_ControlMotor(activePad, 1);
    }

    // Salir: mantener la combinación durante unos 2 segundos
    if ((keysHeld[activePad] & EXIT_COMBO) == EXIT_COMBO) {
      exitCounter++;
      if (exitCounter >= EXIT_FRAMES) {
#ifdef __WII__
        exit(0);
#else
        SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
#endif
      }
    } else {
      exitCounter = 0;
    }

    // Active controller modifier
    for (int i = 0; i < 4; i++) {
      if (keysHeld[i] != 0) {
        activePad = i;
      }
    }

    for (int i = 0; i < 4; i++) {
      keysHeldOld[i] = keysHeld[i];
    }
  }

  exit(0);
}
