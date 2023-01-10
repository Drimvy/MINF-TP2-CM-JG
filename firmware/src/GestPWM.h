#ifndef GestPWM_H
#define GestPWM_H
/*--------------------------------------------------------*/
// GestPWM.h
/*--------------------------------------------------------*/
//	Description :	Gestion des PWM 
//			        pour TP1 2016-2017
//
//	Auteur 		: 	C. HUBER
//
//	Version		:	V1.1
//	Compilateur	:	XC32 V1.42 + Harmony 1.08
//
/*--------------------------------------------------------*/

#include <stdint.h>
#include "Mc32DriverAdc.h"


/*--------------------------------------------------------*/
// Définition des fonctions prototypes
/*--------------------------------------------------------*/
#define FILTER_SIZE 10
#define ADC_RES 1023
#define ABS_MAX_SPEED 198
#define SPEED_RATIO (ABS_MAX_SPEED / ADC_RES)



typedef struct {
    
    /* Variables consignes */
    uint8_t absSpeed;    // vitesse 0 à 99
    uint8_t absAngle;    // Angle  0 à 180
    int8_t SpeedSetting; // consigne vitesse -99 à +99
    int8_t AngleSetting; // consigne angle  -90 à +90
    
    /* Variables ADC */
    S_ADCResults AdcResBuff[FILTER_SIZE]; // Données ADC
    float adcResFilt_Can0;
    float adcResFilt_Can1;
    uint8_t cntAdc;  
} S_pwmSettings;

extern S_pwmSettings PWMData; 

void GPWM_Initialize(S_pwmSettings *pData);

// Ces 3 fonctions ont pour paramètre un pointeur sur la structure S_pwmSettings.
uint8_t GPWM_ReadAdcFiltered(S_pwmSettings *pData);
void GPWM_GetSettings(S_pwmSettings *pData);	// Obtention vitesse et angle
void GPWM_DispSettings(S_pwmSettings *pData);	// Affichage
void GPWM_ExecPWM(S_pwmSettings *pData);		// Execution PWM et gestion moteur.
void GPWM_ExecPWMSoft(S_pwmSettings *pData);		// Execution PWM software.


#endif
