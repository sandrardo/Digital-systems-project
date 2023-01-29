/*
 * reloj.h
 *
 *  Created on: 11 de feb. de 2022
 *      Author: Sof�a y Sandra
 */

#ifndef RELOJ_H_
#define RELOJ_H_

//INCLUDES cabeceras externas
#include "systemConfig.h"
#include "util.h"



//DEFINES Y ENUMS incluimos enum -> definici�n de estados
enum fsm_state {
	WAIT_TIC
};

//FLAGS FSM
#define FLAG_ACTUALIZA_RELOJ	0x10
//#define FLAG_TIME_ACTUALIZADO	0x02


//DECLARACION DE ESTRUCTURAS
//Definici�n tipo calendario
typedef struct {
	int dd;
	int MM;
	int yyyy;
	//int pmam;
}
TipoCalendario;

//Definici�n TipoHora
typedef struct {
	int hh;
	int mm;
	int ss;
	int formato;
	int ampm;
} TipoHora;

//Definici�n TipoReloj
typedef struct {
	int timestamp; //segundos que han pasado desde que se arranc� el sistema
	TipoHora hora; //almacena hora-min-s
	TipoCalendario calendario; //almacena dia-mes-a�o
	tmr_t* tmrTic; //puntero a temporizador que avisa que es el momento de actualizar la hora
} TipoReloj;

//Definici�n TipoRelojShared
typedef struct {
	int flags; //flags: variable entera de la que se usar�n sus bits para notificar eventos del reloj
} TipoRelojShared;

//Definici�n de etiquetas necesarias
#define PRECISION_RELOJ_MS 1000
#define DEFAULT_DAY 28
#define DEFAULT_MONTH 02
#define DEFAULT_YEAR 2020
#define DEFAULT_HOUR 23
#define DEFAULT_MIN 59
#define DEFAULT_SEC 57
#define DEFAULT_TIME_FORMAT 24
#define MAX_HOUR 23
#define MAX_HOUR_12 12
#define MAX_MIN 59
#define MAX_MONTH 12
#define TIME_FORMAT_24_H 24
#define TIME_FORMAT_12_H 12

//DECLARACI�N DE ARRAY
extern fsm_trans_t g_fsmTransReloj[];
//Numero de dias de cada mes para años bisiestos [2] y no bisiestos [1]
extern const int DIAS_MESES[2][12];



//FUNCIONES DE INICIALIZACI�N DE LAS VARIABLES
int ConfiguraInicializaReloj (TipoReloj *p_reloj);
void ResetReloj(TipoReloj *p_reloj);

//FUNCIONES PROPIAS
void ActualizaFecha(TipoCalendario *p_fecha);
void ActualizaHora(TipoHora *p_hora);
int CalculaDiasMes(int month, int year);
int EsBisiesto(int year);
TipoRelojShared GetRelojSharedVar();
int SetFecha(int nuevaFecha, TipoCalendario *p_fecha);
int SetFormato(int nuevaFormato, TipoHora *p_hora);
int SetHora(int nuevaHora, TipoHora *p_hora);
void SetRelojSharedVar(TipoRelojShared value);


//FUNCIONES DE ENTRADA O DE TRANSICION DE LA MAQUINA DE ESTADOS
int CompruebaTic(fsm_t *p_this);

//FUNCIONES DE SALIDA O DE ACCION DE LA MAQUINA DE ESTADOS
void ActualizaReloj(fsm_t *p_this);

//SUBRUTINAS DE ATENCION A LAS INTERRUPCIONES
void tmr_actualiza_reloj_isr(union sigval value);

#endif /* RELOJ_H_ */
