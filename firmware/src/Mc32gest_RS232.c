// Mc32Gest_RS232.C
// Canevas manipulatio TP2 RS232 SLO2 2017-18
// Fonctions d'émission et de réception des message
// CHR 20.12.2016 ajout traitement int error
// CHR 22.12.2016 evolution des marquers observation int Usart
// SCA 03.01.2018 nettoyé réponse interrupt pour ne laisser que les 3 ifs

#include <xc.h>
#include <sys/attribs.h>
#include "system_definitions.h"
#include <GenericTypeDefs.h>
#include "app.h"
#include "GesFifoTh32.h"
#include "Mc32gest_RS232.h"
#include "gestPWM.h"
#include "Mc32CalCrc16.h"


typedef union {
        uint16_t val;
        struct {uint8_t lsb;
                uint8_t msb;} shl;
} U_manip16;


// Definition pour les messages
#define MESS_SIZE  5
// avec int8_t besoin -86 au lieu de 0xAA
#define STX_code  (-86)

// Structure décrivant le message
typedef struct {
    int8_t Start;
    int8_t  Speed;
    int8_t  Angle;
    int8_t MsbCrc;
    int8_t LsbCrc;
} StruMess;


// Struct pour émission des messages
StruMess TxMess;
// Struct pour réception des messages
StruMess RxMess;

// Declaration des FIFO pour réception et émission
#define FIFO_RX_SIZE ( (4*MESS_SIZE) + 1)  // 4 messages
#define FIFO_TX_SIZE ( (4*MESS_SIZE) + 1)  // 4 messages

int8_t fifoRX[FIFO_RX_SIZE];
// Declaration du descripteur du FIFO de réception
S_fifo descrFifoRX;


int8_t fifoTX[FIFO_TX_SIZE];
// Declaration du descripteur du FIFO d'émission
S_fifo descrFifoTX;


// Initialisation de la communication sérielle
void InitFifoComm(void)
{    
    // Initialisation du fifo de réception
    InitFifo ( &descrFifoRX, FIFO_RX_SIZE, fifoRX, 0 );
    // Initialisation du fifo d'émission
    InitFifo ( &descrFifoTX, FIFO_TX_SIZE, fifoTX, 0 );
    
    // Init RTS 
    RS232_RTS = 1;   // interdit émission par l'autre
   
} // InitComm

 
// Valeur de retour 0  = pas de message reçu donc local (data non modifié)
// Valeur de retour 1  = message reçu donc en remote (data mis à jour)
int GetMessage(S_pwmSettings *pData)
{
    bool commStatus = 0;
    int32_t NbCharToRead;
    uint8_t TailleChar= 0;
    
    uint16_t OLD_CRC = 0XFFFF;
    uint16_t NEW_CRC;
    uint8_t controle_LCB;
    uint8_t controle_MCB;
    
    // Retourne le nombre de caractères à lire
    NbCharToRead = GetReadSize(&descrFifoRX);
    if(NbCharToRead >= MESS_SIZE)
    {
        
       // Traitement de réception à introduire ICI 
        
        TailleChar = GetCharFromFifo(&descrFifoRX,&RxMess.Start );
        if(TailleChar != 0)
        {
           commStatus = 0; 
        }
        TailleChar = GetCharFromFifo(&descrFifoRX,&RxMess.Speed );
        if(TailleChar != 0)
        {
           commStatus = 0; 
        }
        TailleChar = GetCharFromFifo(&descrFifoRX,&RxMess.Angle);
        if(TailleChar != 0)
        {
           commStatus = 0; 
        }
        TailleChar = GetCharFromFifo(&descrFifoRX,&RxMess.MsbCrc);
        if(TailleChar != 0)
        {
           commStatus = 0; 
        }
        TailleChar = GetCharFromFifo(&descrFifoRX,&RxMess.LsbCrc);
        if(TailleChar != 0)
        {
           commStatus = 0; 
        }
    }
    else
    {
        commStatus = 0;
    }
    
    
    // Lecture et décodage fifo réception
    if(RxMess.Start == 0xAA)
    {
        
        //Calcul de CRC à 0xFFFF et soustraire le message start
        NEW_CRC = updateCRC16(OLD_CRC, RxMess.Start );
        OLD_CRC = NEW_CRC;
        //Soustraire le message speed au CRC
        NEW_CRC = updateCRC16(OLD_CRC, RxMess.Speed );
        OLD_CRC = NEW_CRC;
        //Soustraire le message angle au CRC
        NEW_CRC = updateCRC16(OLD_CRC, RxMess.Angle );
        //extraire du CRC le MSB 
        controle_MCB = (NEW_CRC>>8)& 0xFF;
        //extraire du CRC le LSB 
        controle_LCB = NEW_CRC & 0xFF;
        
        if((RxMess.MsbCrc == controle_MCB) && (RxMess.LsbCrc == controle_LCB))
        {
            pData->AngleSetting = RxMess.Angle;
            pData->SpeedSetting = RxMess.Speed;
            commStatus = 1;
        }
        else
        {
            commStatus =0;
        }
    }
    else
    {
        commStatus = 0;
    }   

    // Gestion controle de flux de la réception
    if(GetWriteSpace ( &descrFifoRX) >= (2*MESS_SIZE)) 
    {
        // autorise émission par l'autre
        RS232_RTS = 0;
        
    }
    return commStatus;
} // GetMessage


// Fonction d'envoi des messages, appel cyclique
void SendMessage(S_pwmSettings *pData)
{
    int32_t freeSize;
    int8_t Fifo_Full;
    //initailiser le CRC
    uint16_t OLD_CRC = 0XFFFF;
    uint16_t NEW_CRC;
    int16_t test;
    
    

    //verifier la place 
    freeSize = GetWriteSpace ( &descrFifoRX);
    
    // Traitement émission à introduire ICI
    if(freeSize >= MESS_SIZE)
    {       
        //compose le message
        //Initalise le start du message
        TxMess.Start = 0XAA;
        //Initalise la vitesse du message
        TxMess.Speed = pData->SpeedSetting ;
        //Initalise la angle du message
        TxMess.Angle = pData->AngleSetting;
        
        //Calcule du CRC pour l'envoie
        //Initialiser le CRC à 0xFFFF et soustraire le message start
        NEW_CRC = updateCRC16(OLD_CRC, TxMess.Start );
        OLD_CRC = NEW_CRC;
        //Soustraire le message speed au CRC
        NEW_CRC = updateCRC16(OLD_CRC, TxMess.Speed );
        OLD_CRC = NEW_CRC;
        //Soustraire le message angle au CRC
        NEW_CRC = updateCRC16(OLD_CRC, TxMess.Angle );
        //extraire du CRC le MSB 
        TxMess.MsbCrc = (NEW_CRC>>8)& 0xFF;
        //extraire du CRC le LSB 
        TxMess.LsbCrc = NEW_CRC & 0xFF;
        
        // remplissage fifo émission
        Fifo_Full = PutCharInFifo (&descrFifoTX, TxMess.Start);
        if(Fifo_Full == 1) //Est-ce que la FIFO est pleine?
        {
           InitFifoComm(); //réinitaliser la fifo pour la remplire de nouveau
        }
        
        Fifo_Full = PutCharInFifo (&descrFifoTX, TxMess.Speed);
        if(Fifo_Full == 1)//Est-ce que la FIFO est pleine?
        {
           InitFifoComm(); //réinitaliser la fifo pour la remplire de nouveau
        }
        Fifo_Full = PutCharInFifo (&descrFifoTX, TxMess.Angle);
        if(Fifo_Full == 1)//Est-ce que la FIFO est pleine?
        {
           InitFifoComm(); //réinitaliser la fifo pour la remplire de nouveau
        }
        Fifo_Full = PutCharInFifo (&descrFifoTX, TxMess.MsbCrc);
        if(Fifo_Full == 1)//Est-ce que la FIFO est pleine?
        {
           InitFifoComm(); //réinitaliser la fifo pour la remplire de nouveau
        }
        Fifo_Full = PutCharInFifo (&descrFifoTX, TxMess.LsbCrc);
    }
    
    
    test = RS232_CTS;
    // Gestion du controle de flux
    // si on a un caractère à envoyer et que CTS = 0
    freeSize = GetReadSize(&descrFifoTX);
    if (RS232_CTS == 0)//((RS232_CTS == 0) && (freeSize < 16))
    {
        // Autorise int émission    
        PLIB_INT_SourceEnable(INT_ID_0, INT_SOURCE_USART_1_TRANSMIT);                
    }
}


// Interruption USART1
// !!!!!!!!
// Attention ne pas oublier de supprimer la réponse générée dans system_interrupt
// !!!!!!!!
 void __ISR(_UART_1_VECTOR, ipl5AUTO) _IntHandlerDrvUsartInstance0(void)
{
    USART_ERROR UsartStatus;    
    bool TxBuffFull;
    int32_t Txsize;
    bool i_cts;
    int32_t freeSize;
    bool ErrFiFoFull;
    int8_t Car;
    int8_t ptcarLu;
       

    // Marque début interruption avec Led3
    LED3_W = 1;
    
    // Is this an Error interrupt ?
    if ( PLIB_INT_SourceFlagGet(INT_ID_0, INT_SOURCE_USART_1_ERROR) &&
                 PLIB_INT_SourceIsEnabled(INT_ID_0, INT_SOURCE_USART_1_ERROR) ) 
    {
        /* Clear pending interrupt */
        PLIB_INT_SourceFlagClear(INT_ID_0, INT_SOURCE_USART_1_ERROR);
        // Traitement de l'erreur à la réception.
    }
   

    // Is this an RX interrupt ?
    if ( PLIB_INT_SourceFlagGet(INT_ID_0, INT_SOURCE_USART_1_RECEIVE) &&
                 PLIB_INT_SourceIsEnabled(INT_ID_0, INT_SOURCE_USART_1_RECEIVE) ) 
    {

        // Oui Test si erreur parité ou overrun
        UsartStatus = PLIB_USART_ErrorsGet(USART_ID_1);

        if ( (UsartStatus & (USART_ERROR_PARITY | USART_ERROR_FRAMING | USART_ERROR_RECEIVER_OVERRUN)) == 0) 
        {
             // Transfert dans le FIFO de tous les chars reçus
            // Traitement RX à faire ICI
            // Lecture des caractères depuis le buffer HW -> fifo SW
			//  (pour savoir s'il y a une data dans le buffer HW RX : 
            while (PLIB_USART_ReceiverDataIsAvailable(USART_ID_1))
            {
                Car = PLIB_USART_ReceiverByteReceive(USART_ID_1);
                PutCharInFifo ( &descrFifoRX, Car);
                LED4_W = !LED4_R; // Toggle Led4
            }
  
            // buffer is empty, clear interrupt flag
            PLIB_INT_SourceFlagClear(INT_ID_0, INT_SOURCE_USART_1_RECEIVE);
        } 
        else 
        {
            // Suppression des erreurs
            // La lecture des erreurs les efface sauf pour overrun
            if ( (UsartStatus & USART_ERROR_RECEIVER_OVERRUN) == USART_ERROR_RECEIVER_OVERRUN) 
            {
                   PLIB_USART_ReceiverOverrunErrorClear(USART_ID_1);
            }
        }

        
        // Traitement controle de flux reception à faire ICI
        // Gerer sortie RS232_RTS en fonction de place dispo dans fifo reception
        // ...
        // Traitement du contrôle de flux
            freeSize = GetWriteSpace ( &descrFifoRX);
            if (freeSize <= 6) // a cause d'un int pour 6 char
            {
            // Demande de ne plus émettre
                RS232_RTS = 1;
                if (freeSize == 0) 
                {
                    ErrFiFoFull = 1; // pour debugging
                }
            }

        
    } // end if RX

    
    // Is this an TX interrupt ?
    if ( PLIB_INT_SourceFlagGet(INT_ID_0, INT_SOURCE_USART_1_TRANSMIT) &&
                 PLIB_INT_SourceIsEnabled(INT_ID_0, INT_SOURCE_USART_1_TRANSMIT) ) 
    {
        
        // Avant d'émettre, on vérifie 3 conditions :
        
        //  Si CTS = 0 autorisation d'émettre (entrée RS232_CTS)
        //  S'il y a un caratères à émettre dans le fifo
        //  S'il y a de la place dans le buffer d'émission
        i_cts = RS232_CTS;
        Txsize = GetReadSize (&descrFifoTX);
        TxBuffFull = PLIB_USART_TransmitterBufferIsFull(USART_ID_1);
        if ( (i_cts == 0) && ( Txsize > 0 ) && (TxBuffFull == false )) 
        {
            PutCharInFifo(&descrFifoTX, &ptcarLu); 
            PLIB_USART_TransmitterByteSend(USART_ID_1,ptcarLu);
        //   (envoi avec 
            
            /*do {
                    GetCharFromFifo(&descrFifoTX, &ptcarLu);
                    PLIB_USART_TransmitterByteSend(USART_ID_1,ptcarLu);
                    LED5_W = !LED5_R; // Toggle Led5
                    i_cts = RS232_CTS;
                    Txsize = GetReadSize (&descrFifoTX);
                    TxBuffFull = PLIB_USART_TransmitterBufferIsFull(USART_ID_1);
                } while ( (i_cts == 0) && ( Txsize > 0 ) && TxBuffFull == false );*/
                

        }
         LED5_W = !LED5_R; // Toggle Led5
        
		
        // disable TX interrupt (pour éviter une interrupt. inutile si plus rien à transmettre)
        PLIB_INT_SourceDisable(INT_ID_0, INT_SOURCE_USART_1_TRANSMIT);
        
        // Clear the TX interrupt Flag (Seulement apres TX) 
        PLIB_INT_SourceFlagClear(INT_ID_0, INT_SOURCE_USART_1_TRANSMIT);
    }
    // Marque fin interruption avec Led3
    LED3_W = 0;
 }




