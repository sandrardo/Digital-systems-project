#ifndef COREWATCH_H_
#define COREWATCH_H_
#define CODIGOERROR 666

// INCLUDES
// Propios:
#include "systemConfig.h"     // Sistema: includes, entrenadora (GPIOs, MUTEXes y entorno), setup de perifericos y otros otros.
#include "reloj.h"
#include "teclado_TL04.h"

// DEFINES Y ENUMS
#define TECLA_RESET 0x46
#define TECLA_SET_CANCEL_TIME 0x45
#define TECLA_EXIT 0x42
#define NUM_COLUMNAS_TECLADO 4
#define NUM_FILAS_TECLADO 4
//Version 4
#define NUM_FILAS_DISPLAY 2
#define NUM_COLUMNAS_DISPLAY 12
#define NUM_BITS_DISPLAY 8


enum FSM_ESTADOS_SISTEMA {
	START,
	STAND_BY,
	SET_TIME
};

enum FSM_DETECCION_COMANDOS {
	WAIT_COMMAND,
};

// DECLARACIÓN ESTRUCTURAS
//La version 2 no debe incluir ni el teclado ni el lcdId
typedef struct{
	TipoReloj reloj;
	TipoTeclado teclado;
	int lcdId;
	int tempTime;
	int digitosGuardados;
	int digitoPulsado;
} TipoCoreWatch;



// DECLARACIÓN VARIABLES



// DEFINICIÓN VARIABLES


//------------------------------------------------------
// FUNCIONES DE INICIALIZACION DE LAS VARIABLES
//------------------------------------------------------

//------------------------------------------------------
// FUNCIONES PROPIAS
//------------------------------------------------------
void DelayUntil(unsigned int next);
int ConfiguraInicializaSistema(TipoCoreWatch *p_sistema);
int EsNumero(char value);

//------------------------------------------------------
// FUNCIONES DE ENTRADA O DE TRANSICION DE LA MAQUINA DE ESTADOS
//------------------------------------------------------
int CompruebaDigitoPulsado(fsm_t* p_this);
int CompruebaNewTimeIsReady(fsm_t* p_this);
int CompruebaReset(fsm_t* p_this);
int CompruebaSetCancelNewTime(fsm_t* p_this);
int CompruebaSetupDone(fsm_t* p_this);
int CompruebaTeclaPulsada(fsm_t* p_this); //Se codifica en la versión 3
int CompruebaTimeActualizado(fsm_t* p_this);



//------------------------------------------------------
// FUNCIONES DE SALIDA O DE ACCION DE LA MAQUINA DE ESTADOS
//------------------------------------------------------

void CancelSetNewTime(fsm_t* p_this);
void PrepareSetNewTime(fsm_t* p_this);
void ProcesaTeclaPulsada(fsm_t* p_this);
void ShowTime(fsm_t* p_this);
void Start(fsm_t* p_this);
void ProcesaDigitoTime(fsm_t* p_this);
void Reset(fsm_t* p_this);
void SetNewTime(fsm_t* p_this);


//------------------------------------------------------
// SUBRUTINAS DE ATENCION A LAS INTERRUPCIONES
//------------------------------------------------------
//vacio

//------------------------------------------------------
// FUNCIONES LIGADAS A THREADS ADICIONALES
//------------------------------------------------------
#if VERSION==2
PI_THREAD(ThreadExploraTecladoPC);


#endif /* VERSION */
#endif /* COREWATCH_H */
