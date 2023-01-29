/*
 * reloj.c
 *
 *  Created on: 11 de feb. de 2022
 *      Author: Sandra y Sof�a
 */
#include "reloj.h"
#include "kbhit.h" //Para poder detectar teclas pulsadas sin bloqueo y leer las teclas pulsadas
#include "fsm.h"
#include "util.h"
#include "tmr.h"

//#include "pseudoWiringPi.h"
//Para poder crear y ejecutar la m�quina de estados
//#define CLK_MS 10 //Periodo de actualizaci�n de la m�quina de estados (100ms)

const int DIAS_MESES[2][12]={
		{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
		{31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
};

fsm_trans_t g_fsmTransReloj[] = {
		//{EstadoIni, FuncCompruebaCondicion, EstadoSig, FuncAccionesSiTransicion}
		//{EstadoIni, in , EstadoSig, on}
		{WAIT_TIC, CompruebaTic, WAIT_TIC, ActualizaReloj},
		{-1, NULL, -1, NULL}//Sirve a la librer�a fsm.c para detectar fin tabla y empezar de nuevo a comprobar transiciones
};

static TipoRelojShared g_relojSharedVars;

void ResetReloj(TipoReloj *p_reloj){
	TipoCalendario calendario;
	calendario.dd = DEFAULT_DAY;
	calendario.MM = DEFAULT_MONTH;
	calendario.yyyy = DEFAULT_YEAR;
	p_reloj->calendario = calendario;

	TipoHora hora;
	hora.hh = DEFAULT_HOUR;
	hora.ss = DEFAULT_SEC;
	hora.mm = DEFAULT_MIN;
	hora.formato = DEFAULT_TIME_FORMAT;
	p_reloj->hora = hora;

	p_reloj->timestamp = 0;

	piLock(RELOJ_KEY);
	g_relojSharedVars.flags = 0;
	piUnlock(RELOJ_KEY);
}

int ConfiguraInicializaReloj(TipoReloj *p_reloj){
	//temporizador estructura: tmr_t* tmr_new (notify_func_t isr);
	ResetReloj(p_reloj);
	tmr_t* temporizador = tmr_new(tmr_actualiza_reloj_isr);
	p_reloj->tmrTic = temporizador;
	tmr_startms_periodic(temporizador, PRECISION_RELOJ_MS);
	return 0;
}

void ActualizaReloj(fsm_t *p_this){
	TipoReloj *p_miReloj = (TipoReloj*)(p_this -> user_data);
	//*p_miReloj es el reloj recibido
	p_miReloj->timestamp++;//Aumenta en 1 el timestamp de p_miReloj
	ActualizaHora(&(p_miReloj->hora));//Llama a la funcion pasando direcci�n de memoria de la hora del reloj recibido
	if(p_miReloj->hora.hh == 0 && p_miReloj->hora.mm == 0 && p_miReloj->hora.ss == 0){
		ActualizaFecha(&(p_miReloj->calendario));
	}
	piLock(RELOJ_KEY);
	g_relojSharedVars.flags &= (~FLAG_ACTUALIZA_RELOJ); //Limpia el flag
	g_relojSharedVars.flags |= FLAG_TIME_ACTUALIZADO;//activa flag avisar sistema se ha actualizado la hora
	piUnlock(RELOJ_KEY);

//#if VERSION >= 1
//	//Imprime la hora y la fecha por el terminal
//	piLock(STD_IO_LCD_BUFFER_KEY);
//	printf("Son las %d:%d:%d del %d/%d/%d\n", p_miReloj->hora.hh, p_miReloj->hora.mm,p_miReloj->hora.ss,
//			p_miReloj->calendario.dd, p_miReloj->calendario.MM,  p_miReloj->calendario.yyyy);
//	fflush(stdout);
//	piUnlock(STD_IO_LCD_BUFFER_KEY);
//
//#endif


}

int CompruebaTic(fsm_t *p_this){
	int resultado;
	piLock(RELOJ_KEY);
	resultado = (g_relojSharedVars.flags & FLAG_ACTUALIZA_RELOJ);
	piUnlock(RELOJ_KEY);
	return resultado;
	//se activa si se pone a 1
}

void tmr_actualiza_reloj_isr(union sigval value){
	piLock(RELOJ_KEY);
	g_relojSharedVars.flags |= FLAG_ACTUALIZA_RELOJ; //Activa flag FLAG_ACTUALIZA_RELOJ
	piUnlock(RELOJ_KEY);
}

//Actualiza la fecha (dia (dd), mes (MM), y año (yyyy) de la estructura recibida p_fecha
void ActualizaFecha(TipoCalendario *p_fecha){
	int dmaximo;
	int drecibida;
	int mmaximo;

	int DiasMes= CalculaDiasMes(p_fecha->MM,p_fecha->yyyy);
	drecibida = (p_fecha->dd +1) % (DiasMes +1);
	dmaximo = MAX(drecibida,1);
	p_fecha->dd = dmaximo;

	if(p_fecha->dd == 1){
		int m2 = (p_fecha->MM +1) % (MAX_MONTH +1);
		mmaximo = MIN(MAX (m2,1), MAX_MONTH);
		p_fecha->MM = mmaximo;
	}
	if(p_fecha->dd == 1 && p_fecha->MM == 1){
		p_fecha->yyyy = p_fecha->yyyy + 1;
	}
}

//Actualiza la hora (hora(hh)), minutos (mm) y segundos (ss) de la estructura recibida p_hora
void ActualizaHora(TipoHora *p_hora){
	p_hora->ss = (p_hora->ss+1)%60;
	if(p_hora->ss==0){
		p_hora->mm=(p_hora->mm+1)%60;
		if(p_hora->mm==0){
			p_hora->hh = p_hora->hh+1;
			if(p_hora->formato == TIME_FORMAT_12_H && p_hora->hh > MAX_HOUR_12){
				if(p_hora->ampm == 1){
					p_hora->hh = 1;
					p_hora->ampm = 0;
				}
				if(p_hora->ampm == 0){
					p_hora->hh = 1;
					p_hora->ampm = 1;
				}

			}
			if(p_hora->formato == TIME_FORMAT_24_H && p_hora->hh > MAX_HOUR){
				p_hora->hh = 0;
			}
		}
	}
}


int CalculaDiasMes(int month, int year){
	//comprueba si el a�o year es bisiesto para devolver el num dias del mes indicado
	if (EsBisiesto(year)){
		return DIAS_MESES[1][month-1];
	}else{
		return DIAS_MESES[0][month-1];
	}
}

int EsBisiesto(int year){
	int i=0;
	if(year%4 == 0){
		if(year%100 == 0){
			if(year%400 == 0){
				i=1;
			}else{
				i=0;
			}
		}else{
			i=1;
		}
	}
	return i;
};

int SetHora(int horaInt, TipoHora *p_hora){
	if(!(horaInt >= 0)){
		return 1;
	}
	int NumeroDigitos = 0;
	int auxiliar = horaInt;
	do {
		auxiliar = auxiliar/10;
		NumeroDigitos++;
	}while(auxiliar != 0);

	if (horaInt == 1){
		p_hora->mm = horaInt;
		p_hora->hh = 0;
	}
	if (NumeroDigitos == 2){
		p_hora->mm = horaInt;
		p_hora->hh = 0;
	}
	if(NumeroDigitos == 3 || NumeroDigitos == 4 ){
		p_hora->mm = horaInt%100;
		p_hora->hh = horaInt/100;
	}
	if(p_hora->hh > MAX_HOUR) {
		p_hora->hh = MAX_HOUR;
	}
	if(p_hora->mm > MAX_MIN) {
		p_hora->mm = MAX_MIN;
	}
	p_hora->ss = 0;
	return 0;
}

TipoRelojShared GetRelojSharedVar(){
	piLock(RELOJ_KEY);
	TipoRelojShared g_local = g_relojSharedVars;
	piUnlock(RELOJ_KEY);
	return g_local;
}

void SetRelojSharedVar (TipoRelojShared value){
	piLock(RELOJ_KEY);
	g_relojSharedVars = value;
	piUnlock(RELOJ_KEY);
	return;
}
