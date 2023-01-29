#include "coreWatch.h"
#include "reloj.h"
#include "kbhit.h"
//#include "lcd.h"

//VARIABLES GLOBALES
TipoCoreWatch g_corewatch;
static int g_flagsCoreWatch;

//Version 3
int arrayFilas[NUM_FILAS_TECLADO] = {GPIO_KEYBOARD_ROW_1,GPIO_KEYBOARD_ROW_2,GPIO_KEYBOARD_ROW_3, GPIO_KEYBOARD_ROW_4};
int arrayColumnas[NUM_COLUMNAS_TECLADO]= {GPIO_KEYBOARD_COL_1,GPIO_KEYBOARD_COL_2,GPIO_KEYBOARD_COL_3, GPIO_KEYBOARD_COL_4};
void (*rutinas_ISR[NUM_FILAS_TECLADO])= {teclado_fila_1_isr,teclado_fila_2_isr,teclado_fila_3_isr,teclado_fila_4_isr};

#if VERSION >= 2
//tabla de transición
static fsm_trans_t fsmTransCoreWatch[] = {   //7 estados
		//{EstadoIni, FuncCompruebaCondicion, EstadoSig, FuncAccionesSiTransicion}
		{START, CompruebaSetupDone, STAND_BY,Start},
		{STAND_BY, CompruebaTimeActualizado, STAND_BY, ShowTime},
		{STAND_BY, CompruebaReset, STAND_BY, Reset},
		{STAND_BY, CompruebaSetCancelNewTime, SET_TIME, PrepareSetNewTime},
		{SET_TIME, CompruebaDigitoPulsado, SET_TIME, ProcesaDigitoTime},
		{SET_TIME, CompruebaSetCancelNewTime, STAND_BY, CancelSetNewTime},
		{SET_TIME, CompruebaNewTimeIsReady, STAND_BY, SetNewTime},
		{-1, NULL, -1, NULL}
};
#endif

fsm_trans_t fsmTransDeteccionComandos[] = {
		{ WAIT_COMMAND,CompruebaTeclaPulsada, WAIT_COMMAND, ProcesaTeclaPulsada},
		{-1, NULL, -1, NULL },
};
//------------------------------------------------------
// FUNCIONES PROPIAS
//------------------------------------------------------
// Wait until next_activation (absolute time)
// Necesita de la función "delay" de WiringPi.
//espera hasta la próxima activación del reloj (funcion de retardo)
void DelayUntil(unsigned int next) {
	unsigned int now = millis();
	if (next > now) {
		delay(next - now);
	}
}

int EsNumero(char value){
	int x = value - 0x30;
	if ((x>= 0) && (x<= 9)){
		return 1;
	}else{
		return 0;
	}
}
#if VERSION == 2
PI_THREAD(ThreadExploraTecladoPC){
	int teclaPulsada;
	while(1){
		delay(10); //WiringPi function: pauses program execution for at least 10 ms
		if(kbhit()){
			teclaPulsada = kbread();
			//Logica (diagrama de flujo)
			piLock(SYSTEM_KEY);
			if(teclaPulsada == TECLA_RESET){
				g_flagsCoreWatch |= FLAG_RESET;
			}else{
				if(teclaPulsada == TECLA_SET_CANCEL_TIME){
					g_flagsCoreWatch |= FLAG_SET_CANCEL_NEW_TIME;
				}else{
					if(EsNumero(teclaPulsada)){
						g_corewatch.digitoPulsado= teclaPulsada - 0x30;
						g_flagsCoreWatch |= FLAG_DIGITO_PULSADO;
					}else{
						if(teclaPulsada == TECLA_EXIT){
							printf("Vamos a salir del sistema\n");
							fflush(stdout);
							exit(0);
						}else if((teclaPulsada != '\n') && (teclaPulsada != 'r') && (teclaPulsada != 0xA)){
							printf("Tecla Desconocida\n");
							fflush(stdout);
						}
					}
				}
			}
			piUnlock(SYSTEM_KEY);
		}
	}
}
#endif

//Funcion que se encarga de inicializar los elementos y variables del sistema, segun la version
int ConfiguraInicializaSistema(TipoCoreWatch *p_sistema){

#if VERSION  == 2
	int resultadoDeInicializarThread;
	int resultadoDeInicializarReloj;
	g_flagsCoreWatch = 0;
	p_sistema->tempTime = 0;
	p_sistema->digitosGuardados = 0;
	//Recoge resultado de la inicializacion
	resultadoDeInicializarReloj = ConfiguraInicializaReloj(&(p_sistema->reloj));
	//Comprobacion del resultado de inicialización (resultadoDeInicializarReloj) creo
	if (resultadoDeInicializarReloj != 0){
		return CODIGOERROR;
	}


	//Recoge resultado de la inicialización
	resultadoDeInicializarThread = piThreadCreate(ThreadExploraTecladoPC);
	//Comprobacion del resultado de inicialización (resultadoDeInicializarThread) creo
	if(resultadoDeInicializarThread != 0){
		return CODIGOERROR;
	}
	else{

		piLock(STD_IO_LCD_BUFFER_KEY);
		printf("Thread creado correctamente! :)");
		fflush(stdout);
		piUnlock(STD_IO_LCD_BUFFER_KEY);


		piLock(SYSTEM_KEY);
		g_flagsCoreWatch |= FLAG_SETUP_DONE;
		piUnlock(SYSTEM_KEY);
		return 0;
	}
#endif

#if VERSION == 3

	int result=0;
	int prueba = wiringPiSetupGpio();
	if(prueba <0){
		piLock(STD_IO_LCD_BUFFER_KEY);
		printf("Unable to setup wiringPi/n");
		fflush(stdout);
		piUnlock(STD_IO_LCD_BUFFER_KEY);
		return -1;
	}
	//Copia en memoria N bytes de SRC a DEST:
	//memcpy(DEST,SRC,N)
	memcpy(p_sistema->teclado.filas, arrayFilas, sizeof(arrayFilas));
	memcpy(p_sistema->teclado.columnas, arrayColumnas, sizeof(arrayColumnas));
	memcpy(p_sistema->teclado.rutinas_ISR, rutinas_ISR, sizeof(rutinas_ISR));
	ConfiguraInicializaTeclado(&(p_sistema->teclado));
	piLock(SYSTEM_KEY);
	g_flagsCoreWatch |= FLAG_SETUP_DONE;
	piUnlock(SYSTEM_KEY);
	return result;

#endif

#if VERSION == 4
		int result = 0;
		wiringPiSetupGpio();

		//Copia en memoria N bytes de SRC a DEST:
		//memcpy(DEST,SRC,N)
		memcpy(p_sistema->teclado.filas, arrayFilas, sizeof(arrayFilas));
		memcpy(p_sistema->teclado.columnas, arrayColumnas, sizeof(arrayColumnas));
		memcpy(p_sistema->teclado.rutinas_ISR, rutinas_ISR, sizeof(rutinas_ISR));
		ConfiguraInicializaTeclado(&(p_sistema->teclado));
		//}
		//Inicializacion del display
		p_sistema->lcdId = lcdInit(NUM_FILAS_DISPLAY, NUM_COLUMNAS_DISPLAY, NUM_BITS_DISPLAY, GPIO_LCD_RS, GPIO_LCD_EN, GPIO_LCD_D0, GPIO_LCD_D1, GPIO_LCD_D2, GPIO_LCD_D3, GPIO_LCD_D4, GPIO_LCD_D5,GPIO_LCD_D6, GPIO_LCD_D7);
		//Comprueba que handle no es -1 y se ha inicializado correctamente
		if(p_sistema->lcdId == -1){
			result = -1;
			exit(1);
		}
		piLock(SYSTEM_KEY);
				g_flagsCoreWatch |= FLAG_SETUP_DONE;
				piUnlock(SYSTEM_KEY);
		return result;

#endif
}

int CompruebaDigitoPulsado(fsm_t* p_this){
	piLock(SYSTEM_KEY);
	if(g_flagsCoreWatch & FLAG_DIGITO_PULSADO){
		piUnlock(SYSTEM_KEY);
		return 1;
	}
	piUnlock(SYSTEM_KEY);
	return 0;
}

int CompruebaNewTimeIsReady(fsm_t* p_this){
	piLock(SYSTEM_KEY);
	if(g_flagsCoreWatch & FLAG_NEW_TIME_IS_READY){
		piUnlock(SYSTEM_KEY);
		return 1;
	}
	piUnlock(SYSTEM_KEY);
	return 0;

}

int CompruebaReset(fsm_t* p_this){
	piLock(SYSTEM_KEY);
	if(g_flagsCoreWatch & FLAG_RESET){
		piUnlock(SYSTEM_KEY);
		return 1;
	}
	piUnlock(SYSTEM_KEY);
	return 0;

	piLock(STD_IO_LCD_BUFFER_KEY);
	printf("[RESET] Hora reiniciada");
	fflush(stdout);
	piUnlock(STD_IO_LCD_BUFFER_KEY);

}

int CompruebaSetCancelNewTime(fsm_t* p_this){
	piLock(SYSTEM_KEY);
	if(g_flagsCoreWatch & FLAG_SET_CANCEL_NEW_TIME){
		piUnlock(SYSTEM_KEY);
		return 1;
	}
	piUnlock(SYSTEM_KEY);
	return 0;

	piLock(STD_IO_LCD_BUFFER_KEY);
	printf("[SET_TIME] Introduzca nueva hora u operación cancelada");
	fflush(stdout);
	piUnlock(STD_IO_LCD_BUFFER_KEY);
}

int CompruebaSetupDone(fsm_t* p_this){
	piLock(SYSTEM_KEY);
	if(g_flagsCoreWatch & FLAG_SETUP_DONE){
		piUnlock(SYSTEM_KEY);
		return 1;
	}
	piUnlock(SYSTEM_KEY);
	return 0;
}

int CompruebaTimeActualizado(fsm_t* p_this){
	TipoRelojShared r = GetRelojSharedVar();
	int flag  = r.flags & FLAG_TIME_ACTUALIZADO;
	return flag;
}

void Start(fsm_t* p_this){
	piLock(SYSTEM_KEY);
	(g_flagsCoreWatch &= (~FLAG_SETUP_DONE));
	piUnlock(SYSTEM_KEY);
}

void ShowTime(fsm_t* p_this){
	TipoRelojShared r = GetRelojSharedVar();
	(r.flags &= (~FLAG_TIME_ACTUALIZADO));
	SetRelojSharedVar(r);
	TipoCoreWatch *p_sistema = (TipoCoreWatch*)(p_this -> user_data);
#if VERSION <4
	piLock(STD_IO_LCD_BUFFER_KEY);
	printf("Son las %d:%d:%d del %d/%d/%d\n",p_sistema->reloj.hora.hh, p_sistema->reloj.hora.mm, p_sistema->reloj.hora.ss,
			p_sistema->reloj.calendario.dd, p_sistema->reloj.calendario.MM, p_sistema->reloj.calendario.yyyy);
	fflush(stdout);
	piUnlock(STD_IO_LCD_BUFFER_KEY);

#endif

#if VERSION >= 4
	int reloj = p_sistema->lcdId;
	piLock(STD_IO_LCD_BUFFER_KEY);
	lcdClear(p_sistema->lcdId);
	lcdPrintf(reloj, "%d:%d:%d \n", p_sistema->reloj.hora.hh, p_sistema->reloj.hora.mm, p_sistema->reloj.hora.ss);
	lcdPosition(reloj,COLUMNA_1,FILA_2);
	lcdPrintf(reloj, "%d:%d:%d \n", p_sistema->reloj.calendario.dd, p_sistema->reloj.calendario.MM, p_sistema->reloj.calendario.yyyy);
	lcdPosition(reloj,COLUMNA_1,FILA_1);
	fflush(stdout);
	piUnlock(STD_IO_LCD_BUFFER_KEY);

#endif

}

void Reset(fsm_t* p_this){
	TipoCoreWatch *p_sistema = (TipoCoreWatch*)(p_this->user_data);
	ResetReloj(&(p_sistema->reloj));
	piLock(SYSTEM_KEY);
	g_flagsCoreWatch &= (~FLAG_RESET);
	piUnlock(SYSTEM_KEY);
#if VERSION < 4
	piLock(STD_IO_LCD_BUFFER_KEY);
	printf("[RESET] Hora reiniciada \n");
	fflush(stdout);
	piUnlock(STD_IO_LCD_BUFFER_KEY);

#endif

#if VERSION >= 4
	//piLock(STD_IO_LCD_BUFFER_KEY);
	lcdClear(p_sistema->lcdId);
	lcdPosition(p_sistema->lcdId,COLUMNA_1,FILA_2);
	lcdPrintf(p_sistema->lcdId, "RESET \n");
	delay(ESPERA_MENSAJE_MS);
	lcdClear(p_sistema->lcdId);
	//piUnlock(STD_IO_LCD_BUFFER_KEY);

#endif
}

void ProcesaTeclaPulsada(fsm_t* p_this){

		TipoCoreWatch *p_sistema = (TipoCoreWatch*)(p_this->user_data);
		TipoTecladoShared r = GetTecladoSharedVar();
		piLock(KEYBOARD_KEY);
		r.flags &= (~FLAG_TECLA_PULSADA);
		piUnlock(KEYBOARD_KEY);
		SetTecladoSharedVar(r);
		char teclaPulsada = r.teclaDetectada;

				//Logica (diagrama de flujo)
				piLock(STD_IO_LCD_BUFFER_KEY);
				if(teclaPulsada == TECLA_RESET){
					g_flagsCoreWatch|= FLAG_RESET;
				}else{
					if(teclaPulsada == TECLA_SET_CANCEL_TIME){
						g_flagsCoreWatch |= FLAG_SET_CANCEL_NEW_TIME;
					}else{
						if(EsNumero(teclaPulsada)){
							p_sistema->digitoPulsado= teclaPulsada - 0x30;
							g_flagsCoreWatch |= FLAG_DIGITO_PULSADO;

						}else{
							if(teclaPulsada == TECLA_EXIT){
								//cambiar por lcdPrint para v4
								printf(p_sistema->lcdId,"Salimos del sistema \n");
								fflush(stdout);
								exit(0);
							}else if((teclaPulsada != '\n') && (teclaPulsada != 'r') && (teclaPulsada != 0xA)){
								printf(p_sistema->lcdId, "Tecla Desconocida \n");//cambiar por lcdPrint para v4
								delay(ESPERA_MENSAJE_MS);
								fflush(stdout);
							}
						}
					}
				}
				piUnlock(STD_IO_LCD_BUFFER_KEY);
			}

void ProcesaDigitoTime(fsm_t* p_this){

	TipoCoreWatch *p_sistema = (TipoCoreWatch*)(p_this->user_data);
	int tT = p_sistema->tempTime;
	int DG = p_sistema->digitosGuardados;
	int ultimoDigito = g_corewatch.digitoPulsado;
	piLock(SYSTEM_KEY);
	g_flagsCoreWatch &= (~FLAG_DIGITO_PULSADO);
	piUnlock(SYSTEM_KEY);
	//Diagrama de flujo de la funcion ProcesaDigitoTime
	if(DG == 0){
		if(p_sistema->reloj.hora.formato == TIME_FORMAT_12_H){
			ultimoDigito = MIN(1, ultimoDigito);
		}else{
			ultimoDigito = MIN(2, ultimoDigito);
		}
		tT = tT*10 + ultimoDigito;
		DG++;
	}else{
		if(DG ==1){
			if(p_sistema->reloj.hora.formato == TIME_FORMAT_12_H){
				if(tT == 0){
					ultimoDigito = MAX(1, ultimoDigito);
				}else{
					ultimoDigito = MIN(2, ultimoDigito);
				}
			}else{
				if(tT == 2){
					ultimoDigito = MIN(3, ultimoDigito);
				}
			}
			tT = tT*10 + ultimoDigito;
			DG++;
		}else{
			if(DG == 2){
				tT = tT*10 + MIN(5, ultimoDigito);
				DG++;
			}else{
				tT = tT*10 + ultimoDigito;
				piLock(SYSTEM_KEY);
				g_flagsCoreWatch &= (~FLAG_DIGITO_PULSADO);
				piUnlock(SYSTEM_KEY);
				piLock(SYSTEM_KEY);
				g_flagsCoreWatch |= FLAG_NEW_TIME_IS_READY;
				piUnlock(SYSTEM_KEY);
			}
		}
	}



#if VERSION < 4
	piLock(STD_IO_LCD_BUFFER_KEY);
		printf("[SET_TIME] Nueva hora temporal %d \n", p_sistema->tempTime);
		piUnlock(STD_IO_LCD_BUFFER_KEY);
#endif

#if VERSION >= 4
		//limpia primera linea del LCD
		lcdClear(p_sistema->lcdId);
		//imprimir en la primera linea tiempo acumulado
		piLock(STD_IO_LCD_BUFFER_KEY);
		lcdPrintf(p_sistema->lcdId, "SET: %d \n", p_sistema->tempTime);
		piUnlock(STD_IO_LCD_BUFFER_KEY);
#endif
		p_sistema->tempTime = tT;
		p_sistema->digitosGuardados = DG;
}


void SetNewTime(fsm_t* p_this){
	TipoCoreWatch *p_sistema = (TipoCoreWatch*)(p_this->user_data);
	piLock(SYSTEM_KEY);
	g_flagsCoreWatch &= (~FLAG_NEW_TIME_IS_READY);
	piUnlock(SYSTEM_KEY);
	SetHora(p_sistema->tempTime, &(p_sistema->reloj.hora));
	p_sistema->tempTime=0;
	p_sistema->digitosGuardados=0;

}


void PrepareSetNewTime(fsm_t* p_this){
	TipoCoreWatch *p_sistema = (TipoCoreWatch*)(p_this->user_data);
	int form = p_sistema->reloj.hora.formato;
	piLock(SYSTEM_KEY);
	g_flagsCoreWatch &= (~FLAG_DIGITO_PULSADO);
	g_flagsCoreWatch &= (~FLAG_SET_CANCEL_NEW_TIME);
	piUnlock(SYSTEM_KEY);
	//Informar al usuario
#if VERSION < 4
	piLock(STD_IO_LCD_BUFFER_KEY);
	printf("[SET_TIME] Introduzca la nueva hora en formato 0-%d \n", form);
	fflush(stdout);
	piUnlock(STD_IO_LCD_BUFFER_KEY);
#endif
#if VERSION >= 4
	//limpia LCD
	lcdClear(p_sistema->lcdId);
	//coloca cursor en columna 1 fila 2
	lcdPosition(p_sistema->lcdId, COLUMNA_1,FILA_2);
	//imprime cancelado y espera ESPERA_MENSAJE_MS
	piLock(STD_IO_LCD_BUFFER_KEY);
	lcdPrintf(p_sistema->lcdId,"FORMAT 0-%d \n", form);
	piUnlock(STD_IO_LCD_BUFFER_KEY);
	delay(ESPERA_MENSAJE_MS);

#endif
}

void CancelSetNewTime(fsm_t* p_this){
	TipoCoreWatch *p_sistema = (TipoCoreWatch*)(p_this->user_data);
	p_sistema->tempTime = 0;
	p_sistema->digitosGuardados = 0;
	piLock(SYSTEM_KEY);
	g_flagsCoreWatch &= (~FLAG_SET_CANCEL_NEW_TIME);
	piUnlock(SYSTEM_KEY);

#if VERSION < 4
	piLock(STD_IO_LCD_BUFFER_KEY);
	printf("[SET_TIME] Operación cancelada\n");
	fflush(stdout);
	piUnlock(STD_IO_LCD_BUFFER_KEY);
#endif

#if VERSION >= 4
	//limpia LCD
	lcdClear(p_sistema->lcdId);
	//coloca cursor en columna 1 fila 2
	lcdPosition(p_sistema->lcdId, COLUMNA_1, FILA_2);
	//imprime cancelado y espera ESPERA_MENSAJE_MS
	piLock(STD_IO_LCD_BUFFER_KEY);
	lcdPrintf(p_sistema->lcdId,"Cancelado");
	delay(ESPERA_MENSAJE_MS);
	piUnlock(STD_IO_LCD_BUFFER_KEY);
#endif
}

//------------------------------------------------------
// MAIN
//------------------------------------------------------
int main() {
	unsigned int next;
#if VERSION == 1

	TipoReloj relojPrueba;
	fsm_t* fsmReloj=fsm_new(WAIT_TIC,g_fsmTransReloj,&(relojPrueba));
	ConfiguraInicializaReloj(&relojPrueba); //Después va a ConfiguraInicializaSistema

#endif


#if VERSION >= 1
	ConfiguraInicializaReloj(&g_corewatch.reloj); //Después va a ConfiguraInicializaSistema
	fsm_t* fsmReloj=fsm_new(WAIT_TIC,g_fsmTransReloj,&(g_corewatch.reloj));
	fsm_t* fsmReloj2=fsm_new(START,fsmTransCoreWatch,&(g_corewatch));
	fsm_t* deteccionComandosFSM= fsm_new(WAIT_COMMAND,fsmTransDeteccionComandos, &(g_corewatch));
	fsm_t* tecladoFSM = fsm_new(TECLADO_ESPERA_COLUMNA,g_fsmTransExcitacionColumnas, &(g_corewatch.teclado));
	int resultSystem = ConfiguraInicializaSistema(&(g_corewatch));
	if(resultSystem != 0){
		piLock(STD_IO_LCD_BUFFER_KEY);
		printf(" Error: va a salir del sistema");
		fflush(stdout);
		piUnlock(STD_IO_LCD_BUFFER_KEY);
		exit(0);
	}

#endif

	next = millis();
	while (1) {
#if VERSION == 1
		fsm_fire(fsmReloj);
#endif
#if VERSION >= 1
		fsm_fire(fsmReloj);
		fsm_fire(fsmReloj2);
		fsm_fire(deteccionComandosFSM);
		fsm_fire(tecladoFSM);

#endif
		next += CLK_MS;
		DelayUntil(next);
	}
};
