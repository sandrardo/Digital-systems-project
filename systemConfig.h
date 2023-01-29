#ifndef _SYSTEMCONFIG_H_
#define _SYSTEMCONFIG_H_
//------------------------------------------------------
// INCLUDES
// Del sistema
#include <time.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>

// Propios
#include "ent2004cfConfig.h"  // Entorno emulado o de laboratorio. GPIOs.
#include "fsm.h"
#include "tmr.h"

#if ENTORNO == ENTORNO_LABORATORIO
#include "kbhit.h" // Para poder detectar teclas pulsadas sin bloqueo y leer las teclas pulsadas
#include <wiringPi.h>
#include <lcd.h>
#elif ENTORNO == ENTORNO_EMULADO
#include "pseudoWiringPi.h"
#include "pseudoWiringPiDev.h"
#endif

// DEFINES Y ENUMS
#define VERSION 3 // Version del sistema (1, 2, 3, 4...)
#define CLK_MS	10  // Actualizacion de las maquinas de estado (ms)
#define ESPERA_MENSAJE_MS 1000 //V4, tiempo antes de limpiar de nuevo flag

// FLAGS FSM DEL SISTEMA CORE WATCH
#define FLAG_SETUP_DONE 0x01
#define FLAG_RESET 0x04
#define FLAG_TIME_ACTUALIZADO 0x02
#define FLAG_SET_CANCEL_NEW_TIME 0x08
#define FLAG_NEW_TIME_IS_READY 0x20
#define FLAG_DIGITO_PULSADO 0x40

// DECLARACIÓN DE VARIABLES


// DEFINICIÓN DE VARIABLES


#endif /* SYSTEMCONFIG_H_ */
