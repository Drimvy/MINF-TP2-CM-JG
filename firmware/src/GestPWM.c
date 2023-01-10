/*--------------------------------------------------------*/
// GestPWM.c
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



#include "GestPWM.h"
#include "Mc32DriverAdc.h"
#include "Mc32DriverLcd.h"
#include "app.h"
#include "peripheral/oc/plib_oc.h"

//S_pwmSettings PWMData;      // pour les settings
//S_ADCResults ReadAdc;

S_pwmSettings PwmData = {.cntAdc=0};
        
void GPWM_Initialize(S_pwmSettings *pData)
{
   // Init les data 
   
   // Init �tat du pont en H
    
   // lance les timers et OC
    
}

// Obtention vitesse et angle (mise a jour des 4 champs de la structure)
void GPWM_GetSettings(S_pwmSettings *pData)	
{
    /* Gestion de la lecture d'ADC avec filtrage */
    GPWM_ReadAdcFiltered(pData);
    
    /* Calcul de la consigne de vitesse */
    pData->absSpeed = pData->adcResFilt_Can0 * SPEED_RATIO;
    pData->SpeedSetting = pData->absSpeed - 99;
    pData->absSpeed = abs(pData->SpeedSetting);
    
    
    /* Calcul consigne de l'angle */
    pData->absAngle = pData->adcResFilt_Can1 * (180 / ADC_RES);
    pData->AngleSetting = pData->absAngle - 90;
    pData->absAngle = abs(pData->AngleSetting);
    
    
}


// Affichage des information en exploitant la structure
void GPWM_DispSettings(S_pwmSettings *pData)
{   
            lcd_gotoxy(1,1);
            printf_lcd("TP1 PWM 2022-2023");
            lcd_gotoxy(1,2);
            /* Affichage vitesse */
            printf_lcd("SpeedSetting");
            lcd_gotoxy(17,2);
            printf_lcd("%4d", (int)pData->SpeedSetting);
            /* Affichage vitesse absolue */
            lcd_gotoxy(1,3);
            printf_lcd("absSpeed");
            lcd_gotoxy(17,3);
            printf_lcd("%4d", (int)pData->absSpeed);
            /* Afficahge de l'angle */
            lcd_gotoxy(1,4);
            printf_lcd("Angle");
            lcd_gotoxy(17,4);
            printf_lcd("%4d", (int)pData->AngleSetting);
            
            
}

// Execution PWM et gestion moteur � partir des info dans structure
void GPWM_ExecPWM(S_pwmSettings *pData)
{
 float old_Data;
 static int i_speeed;
 static int i_angle; 
    
    //tourner le moteur de gauche � droite.............................
    if(pData->SpeedSetting <= 1)
    {
        AIN1_HBRIDGE_W = 1; //AIN1 High
        AIN2_HBRIDGE_W = 0; //AIN2 LOW
        STBY_HBRIDGE_W = 1; // STBY High
    }
    //tourner le moteur de droite � gauche.........................
    if (pData->SpeedSetting >= 1)
    {
        AIN1_HBRIDGE_W = 0; //AIN1 LOW
        AIN2_HBRIDGE_W = 1; //AIN2 High
        STBY_HBRIDGE_W = 1; // STBY High
    }
    else 
    {
        //moteur ne tourne plus, il passe en stanby
        STBY_HBRIDGE_W = 0; // STBY LOW       
    }
    //d�terminer le nombre d'impulsion pour OC2 � partir de absSpeed
    old_Data = pData->absSpeed; 
    if ( old_Data != pData->absSpeed)
    {
        i_speeed ++;
    }
    //D�terminer la valeur cyclique du PWM
    PLIB_OC_PulseWidth16BitSet(i_speeed,pData->absSpeed);
    //d�terminer le nombre d'impulsion pour OC3 � partir de absAngle
      old_Data = pData->AngleSetting; 
    if ( old_Data != pData->AngleSetting)
    {
        i_angle ++;
    }
    
    //g�n�ration d'une impulsion dont la largeur est proportionnelle � l'angle
    PLIB_OC_PulseWidth16BitSet(i_angle,pData->AngleSetting);
    
 }
    
   
  
    



void GPWM_ReadAdcFiltered(S_pwmSettings *pData)
{
    uint8_t i = 0; 
//    uint8_t flagZero = 0; 
    
    //lecture nouvelles valeurs ch0 et ch1
    pData->AdcResBuff[pData->cntAdc] = BSP_ReadAllADC();
    
    //incr�mentation compteur
    if(pData->cntAdc < FILTER_SIZE-1)
    {
        /* Incremente le timer de filtrage */
        pData->cntAdc = pData->cntAdc +1;
    }
    else
    {
        /* Relance le filtrage */
        pData->cntAdc = 0;
    }
    
    
//    
//    /* Si le compteur de nombre d'�chantillons pour filtrage atteint */
//    if(pData->cntAdc < FILTER_SIZE)
//    {
//        /* Sauvegarde les cannaux dans le buffer de mesure */
//        pData->AdcResBuff[pData->cntAdc] = BSP_ReadAllADC();
//        /* Incremente le timer de filtrage */
//        pData->cntAdc = pData->cntAdc +1;
//    }
//    else
//    {
//        /* Relance le filtrage */
//        pData->cntAdc = 0;
//    }
    
    /* Somme du buffer pour le filtrage */
    pData->adcResFilt_Can0 = 0;
    pData->adcResFilt_Can1 = 0;
    for(i = 0; i<FILTER_SIZE; i++)
    {
        pData->adcResFilt_Can0 += pData->AdcResBuff[i].Chan0;
        pData->adcResFilt_Can1 += pData->AdcResBuff[i].Chan1;
        
//        if((pData->AdcResBuff[i].Chan0 == 0) || (pData->AdcResBuff[i].Chan1 == 0))
//            flagZero = 1;
    }  
//    /* Moyenne du buffer avec protection de division avec 0 */
//    if((pData->adcResFilt_Can0 != 0) && (pData->adcResFilt_Can1 != 0))
//    {
    pData->adcResFilt_Can0 /= (float)FILTER_SIZE;
    pData->adcResFilt_Can1 /= (float)FILTER_SIZE;
//    }
//    return flagZero;
}
