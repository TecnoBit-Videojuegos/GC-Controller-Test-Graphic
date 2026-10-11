#include <stdio.h>
#include <stdlib.h>
#include <gccore.h>

#include "../common/common_utils.h"
#include "gfx.h"
#include "ppc_utils.h"

// Combinación para salir: Start + A a la vez
#define EXIT_COMBO (PAD_BUTTON_START | PAD_BUTTON_A)
// Cuántos cuadros hay que mantenerla (unos 2 segundos)
#define EXIT_FRAMES 120

// Volver a Swiss (o al cargador que nos abrió)
static void salir(void) {
  volatile u32 *stub = (volatile u32 *)0x80001800;
  // Swiss deja la marca "STUBHAXX" en 0x80001804
  if (stub[1] == 0x53545542 && stub[2] == 0x48415858) {
    SYS_ResetSystem(SYS_SHUTDOWN, 0, 0);
    ((void (*)(void))0x80001800)();
  }
  // Si no hay Swiss, salida normal
  exit(0);
}

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
        salir();
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
