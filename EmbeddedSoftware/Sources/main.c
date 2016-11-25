/*!
 ** @file
 ** @version 1.0
 ** @brief  Main module.
 **
 **   This file contains the high-level code for the project.
 **   It initialises appropriate hardware subsystems,
 **   creates application threads, and then starts the OS.
 **
 **   An example of two threads communicating via a semaphore
 **   is given that flashes the orange LED. These should be removed
 **   when the use of threads and the RTOS is understood.
 */
/*!
 **  @addtogroup main_module main module documentation
 **  @{
 */
/* MODULE main */

// CPU module - contains low level hardware initialization routines
#include "Cpu.h"
#include "Events.h"
#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"
#include "FIFO.h"
#include "UART.h"
#include "packet.h"
#include "LEDs.h"
#include "Flash.h"
#include "RTC.h"
#include "PIT.h"
#include "FTM.h"
#include "SPI.h"
#include "analog.h"
#include "median.h"
#include "types.h"
#include "OS.h"

#define CMD_STARTUP 0x04
#define CMD_TOWERVERSION 0x09
#define CMD_TOWERMODE 0x0D
#define CMD_TOWERNUMBER 0x0B
#define CMD_FLASHREAD 0x08
#define CMD_FLASHPROGRAM 0x07
#define CMD_TOWERTIME 0x0C
#define CMD_PROTOCOL 0x0A
#define CMD_ANALOGINPUT 0x50
#define THREAD_STACK_SIZE 100

/*!< The packets' command */
static const uint8_t CMDSTARTUP = 0x04;
static const uint8_t CMDTOWERVERSION = 0x09;
static const uint8_t CMDTOWERNUMBER = 0x0B;
static const uint8_t CMDTOWERMODE = 0x0D;
static const uint8_t CMDFLASHREAD = 0x08;
static const uint8_t CMDTOWERTIME = 0x0C;
static const uint8_t CMDPROTOCOL = 0x0A;
static const uint8_t CMDANALOGINPUT = 0x50;
const uint8_t PACKET_ACK_MASK = 0x80;

static const uint32_t BAUDRATE = 115200;  /*!< The baud rate */
static const uint32_t MODULE_CLK = CPU_BUS_CLK_HZ;  /*!< The module clock */
static const uint16_t TIMEDELAY1SECOND = 24414;  /*!< a timer delay count equals to 1 second */
static uint8_t ProtocolMode = 1;
volatile uint16union_t *NvTowerNb, *NvTowerMd;  /*!< non-volatile variable for storing tower number and mode */
static uint16union_t towerNumber, towerMode;
static TFTMChannel FTMChannel0, FTMChannel1;

// semaphores
static OS_ECB *TowerInitSemaphore;

//create threads stacks
static uint32_t TxThreadStack[THREAD_STACK_SIZE] __attribute__ ((aligned(0x08)));
static uint32_t RxThreadStack[THREAD_STACK_SIZE] __attribute__ ((aligned(0x08)));
static uint32_t HandlePacketThreadStack[THREAD_STACK_SIZE] __attribute__ ((aligned(0x08)));
static uint32_t TowerInitThreadStack[THREAD_STACK_SIZE] __attribute__ ((aligned(0x08)));
static uint32_t PITCallbackThreadStack[THREAD_STACK_SIZE] __attribute__ ((aligned(0x08)));
static uint32_t RTCCallbackThreadStack[THREAD_STACK_SIZE] __attribute__ ((aligned(0x08)));
static uint32_t FTMCh0ThreadStack[THREAD_STACK_SIZE] __attribute__ ((aligned(0x08)));
static uint32_t FTMCh1ThreadStack[THREAD_STACK_SIZE] __attribute__ ((aligned(0x08)));

// functions prototype
static void InitPackets(void);
static void HandlePacket(void);
static BOOL HandleFlashProgramPacket(void);
static BOOL HandleFlashReadPacket(void);
static BOOL HandleModePacket(void);
static BOOL HandleNumberPacket(void);
static BOOL HandleStartupPacket(void);
static BOOL HandleTimePacket(void);
static BOOL HandleVersionPacket(void);
static BOOL HandleProtocolModePacket(void);


/*! @brief turn the blue led off when the time outs
 *  @param arguments used for thread
 *
 *  @return void
 */
static void FTMCh0Thread(void *arguments)
{
  for (;;)
  {
    OS_SemaphoreWait(FTMCh0Semaphore, 0);
    LEDs_Off(LED_BLUE);
  }
}

/*! @brief Sends ADC data to PC
*  @param parameters used for thread
*
*  @return void
*/
static void FTMCh1Thread(void *arguments)
{
  for (;;)
  {
    OS_SemaphoreWait(FTMCh1Semaphore, 0);
    // Restart timer
    (void)FTM_StartTimer(&FTMChannel1);

    uint8_t channelNb;
    for (channelNb = 0; channelNb < ANALOG_NB_INPUTS; channelNb++)
      Analog_Get(channelNb);

    // Asynchronous mode
    // Send packet only if value has changed
    if (ProtocolMode == 0)
    {
      for (uint8_t channelNb = 0; channelNb < ANALOG_NB_INPUTS; channelNb++)
      if (Analog_Input[channelNb].value.l != Analog_Input[channelNb].oldValue.l)
	Packet_Put(CMDANALOGINPUT, channelNb, Analog_Input[channelNb].value.s.Lo, Analog_Input[channelNb].value.s.Hi);
    }

    // Synchronous mode
    // Send all channels
    else if (ProtocolMode == 1)
    {
      for (uint8_t channelNb = 0; channelNb < ANALOG_NB_INPUTS; channelNb++)
	Packet_Put(CMDANALOGINPUT, channelNb, Analog_Input[channelNb].value.s.Lo, Analog_Input[channelNb].value.s.Hi);
    }
  }
}

/*! @brief a thread to toggle green LED
 *  @param arguments used for thread
 *
 *  @return void
 */
static void PITCallbackThread(void *pData)
{
  for (;;)
  {
    OS_SemaphoreWait(PITCallbackSemaphore, 0);
    LEDs_Toggle(LED_GREEN);
  }
}

/*! @brief a thread to toggle yellow LED and send a time packet
 *  @param arguments used for thread
 *
 *  @return void
 */
static void RTCCallbackThread(void *pData)
{
  for (;;)
  {
    OS_SemaphoreWait(RTCCallbackSemaphore, 0);
    uint8_t hours, minutes, seconds;
    RTC_Get(&hours, &minutes, &seconds);
    Packet_Put(CMDTOWERTIME, hours, minutes, seconds);
    LEDs_Toggle(LED_YELLOW);
  }
}

/*! @brief thread to handle the incoming packets with packet acknowledgement
 *
 *  @param pData arguments used for thread
 *  @return void
 */
static void HandlePacketThread(void *pData)
{
  for (;;)
  {
    OS_SemaphoreWait(Packet_HandleSemaphore, 0);

    uint8_t command;  /*!< The packet's command */
    BOOL success = bTRUE; /*!< a bool variable to indicate whether the packet if carried out successfully or not */
    if (Packet_Get())
    {
      //turn a blue led on for 1 second upon a valid packet
      LEDs_On(LED_BLUE);
      (void)FTM_StartTimer(&FTMChannel0);

      command = Packet_Command;
      command &= ~PACKET_ACK_MASK;
      switch (command)
      {
	case CMD_STARTUP:
	  success = HandleStartupPacket();
	  break;
	case CMD_TOWERVERSION:
	  success = HandleVersionPacket();
	  break;
	case CMD_TOWERNUMBER:
	  success = HandleNumberPacket();
	  break;
	case CMD_TOWERMODE:
	  success = HandleModePacket();
	  break;
	case CMD_FLASHPROGRAM:
	  success = HandleFlashProgramPacket();
	  break;
	case CMD_FLASHREAD:
	  success = HandleFlashReadPacket();
	  break;
	case CMD_TOWERTIME:
	  success = HandleTimePacket();
	  break;
	case CMD_PROTOCOL:
	  success = HandleProtocolModePacket();
	  break;
	default:
	  success = bFALSE;
	  break;
      }

      if (Packet_Command != command) //with request ACK
      {
	if (success == bTRUE)
	    //handle successfully
	  Packet_Put(Packet_Command, Packet_Parameter1, Packet_Parameter2, Packet_Parameter3);
	else
	    //not successfully
	  Packet_Put(command, Packet_Parameter1, Packet_Parameter2, Packet_Parameter3);
      }
    }

    OS_SemaphoreSignal(Packet_HandleSemaphore);
  }
}

static void TowerInitThread(void *pData)
{
  for (;;)
  {
    OS_SemaphoreWait(TowerInitSemaphore, 0);

    OS_DisableInterrupts();

    if (Packet_Init(BAUDRATE, MODULE_CLK) && LEDs_Init() && Flash_Init() && RTC_Init() && PIT_Init(MODULE_CLK) && FTM_Init() && Analog_Init(CPU_BUS_CLK_HZ))   //send 3 startup packets upon power up
    {
      //Aclocate space for non-volatile tower number and mode
      Flash_AllocateVar(&NvTowerNb, sizeof(*NvTowerNb));
      Flash_AllocateVar(&NvTowerMd, sizeof(*NvTowerMd));
      towerNumber.l = NvTowerNb->l;
      towerMode.l = NvTowerMd->l;
      if (towerNumber.l == 0xFFFF)
      {
        towerNumber.l = 0x208D;
	Flash_Write16((uint16 *)NvTowerNb, towerNumber.l);
      }
      if (towerMode.l == 0xFFFF)
      {
	towerMode.l = 0x0100;
	Flash_Write16((uint16 *)NvTowerMd, towerMode.l);
      }

      PIT_Enable(bTRUE);
      PIT_Set(500, bTRUE);

      FTMChannel0.channelNb = 0;
      FTMChannel0.delayCount = TIMEDELAY1SECOND;
      FTMChannel0.timerFunction = TIMER_FUNCTION_OUTPUT_COMPARE;

      FTMChannel1.channelNb = 1;
      FTMChannel1.delayCount = TIMEDELAY1SECOND / 100;              // Set to 10 ms
      FTMChannel1.ioType.outputAction = TIMER_OUTPUT_DISCONNECT;
      FTMChannel1.timerFunction = TIMER_FUNCTION_OUTPUT_COMPARE;

      FTM_Set(&FTMChannel0);
      FTM_Set(&FTMChannel1);

      InitPackets();

      LEDs_On(LED_ORANGE);

      OS_EnableInterrupts();

      (void)FTM_StartTimer(&FTMChannel1);

      OS_ThreadDelete(OS_PRIORITY_SELF);

    }
  }
}

/*! @brief In response to the reception of a 0x04 - get startup values packet from
 *   the pc, the tower should transmit three packets.
 *
 *  @return void
 */
static void InitPackets(void)
{
  Packet_Put(CMDSTARTUP, 0, 0, 0);
  Packet_Put(CMDTOWERVERSION, 'v', 1, 0);
  Packet_Put(CMDTOWERNUMBER, 1, towerNumber.s.Lo, towerNumber.s.Hi);
  Packet_Put(CMDTOWERMODE, 1, towerMode.s.Lo, towerMode.s.Hi);
  Packet_Put(CMDPROTOCOL, 1, ProtocolMode, 0);
}

/*! @brief Routine to handle the startup command
 *
 *  @return BOOL - bTRUE handle the packet successfully
 *               bFALSE not successfully
 */
static BOOL HandleStartupPacket()
{
  if ((Packet_Parameter1 == 0) && (Packet_Parameter2 == 0) && (Packet_Parameter3 == 0))
  {
    InitPackets();
    return bTRUE;
  }
  else
    return bFALSE;
}

/*! @brief Routine to handle the Tower version command
 *
 *  @return BOOL - bTRUE handle the packet successfully
 *               bFALSE not successfully
 */
static BOOL HandleVersionPacket()
{
  if ((Packet_Parameter1 == 'v') && (Packet_Parameter2 == 'x'))
  {
    Packet_Put(CMDTOWERVERSION, 'v', 1, 0);
    return bTRUE;
  }
  else
    return bFALSE;
}

/*! @brief Routine to handle the Tower number command
 *
 *  @return BOOL - bTRUE handle the packet successfully
 *               bFALSE not successfully
 */
static BOOL HandleNumberPacket()
{
  //get number
  if ((Packet_Parameter1 == 1) && (Packet_Parameter2 == 0) && (Packet_Parameter3 == 0))
  {
    Packet_Put(CMDTOWERNUMBER, 1, NvTowerNb->s.Lo, NvTowerNb->s.Hi);
    return bTRUE;
  }
  //set number
  else if (Packet_Parameter1 == 2)
  {
    return Flash_Write16((uint16 *)NvTowerNb, Packet_Parameter23);
  }
  else
    return bFALSE;
}

/*! @brief Routine to handle the Tower mode command
 *
 *  @return BOOL - bTRUE handle the packet successfully
 *               bFALSE not successfully
 */
static BOOL HandleModePacket()
{
  //get mode
  if ((Packet_Parameter1 == 1) && (Packet_Parameter2 == 0) && (Packet_Parameter3 == 0))
  {
    Packet_Put(CMDTOWERMODE, 1, NvTowerMd->s.Lo, NvTowerMd->s.Hi);
    return bTRUE;
  }
  //set mode
  else if (Packet_Parameter1 == 2)
  {
    return Flash_Write16((uint16 *)NvTowerMd, Packet_Parameter23);
  }
  else
    return bFALSE;
}

/*! @brief Routine to handle the Tower program byte command
 *
 *  @return BOOL - bTRUE handle the packet successfully
 *               bFALSE not successfully
 */
static BOOL HandleFlashProgramPacket()
{
  if ((Packet_Parameter1 >= 0) && (Packet_Parameter1 <= 0x07) && (Packet_Parameter2 == 0))
    return Flash_Write8((uint8 *)(FLASH_DATA_START + Packet_Parameter1), Packet_Parameter3);
  else if ((Packet_Parameter1 == 0x08) && (Packet_Parameter2 == 0))
    return Flash_Erase();
  else
    return bFALSE;
}

/*! @brief Routine to handle the flash read command
 *
 *  @return BOOL - bTRUE handle the packet successfully
 *               bFALSE not successfully
 */
static BOOL HandleFlashReadPacket()
{
  if ((Packet_Parameter1 >= 0) && (Packet_Parameter1 <= 0x07) && (Packet_Parameter2 == 0) && (Packet_Parameter3 == 0))
  {
    Packet_Put(CMDFLASHREAD, Packet_Parameter1, 0, _FB(FLASH_DATA_START + (uint32_t)Packet_Parameter1));
    return bTRUE;
  }
  else
    return bFALSE;
}

/*! @brief Routine to handle the time packet
 *
 *  @return BOOL - bTRUE handle the packet successfully
 *               bFALSE not successfully
 */
static BOOL HandleTimePacket()
{
  if ((Packet_Parameter1 >= 0) && (Packet_Parameter1 <= 23) && (Packet_Parameter2 >= 0) && (Packet_Parameter2 <= 59) && (Packet_Parameter3 >= 0) && (Packet_Parameter3 <= 59))
  {
    RTC_Set(Packet_Parameter1, Packet_Parameter2, Packet_Parameter3);
    return bTRUE;
  }
  else
    return bFALSE;
}

static BOOL HandleProtocolModePacket()
{
  if ((Packet_Parameter1 == 1) && (Packet_Parameter2 == Packet_Parameter3 == 0))          // Verify packet parameters for protocol mode "get"
  {
    Packet_Put(CMDPROTOCOL, 1, ProtocolMode, 0);
    return bTRUE;
  }
  else if ((Packet_Parameter1 == 2) && (Packet_Parameter3 == 0)                           // Verify packet parameters for protocol mode "set"
          && ((Packet_Parameter2 == 0) || (Packet_Parameter2 == 1)))
  {
    ProtocolMode = Packet_Parameter2;
    return bTRUE;
  }

  return bFALSE;
}


/*lint -save  -e970 Disable MISRA rule (6.3) checking. */
int main(void)
/*lint -restore Enable MISRA rule (6.3) checking. */
{
  /* Write your local variable definition here */
  OS_ERROR error;
  TowerInitSemaphore = OS_SemaphoreCreate(1);

  /*** Processor Expert internal initialization. DON'T REMOVE THIS CODE!!! ***/
  PE_low_level_init();
  /*** End of Processor Expert internal initialization.                    ***/

  /* Write your code here */

  OS_Init(CPU_CORE_CLK_HZ, bFALSE);

  //create threads
  error = OS_ThreadCreate(HandlePacketThread,
			  NULL,
			  &HandlePacketThreadStack[THREAD_STACK_SIZE - 1],
			  3);
  error = OS_ThreadCreate(TowerInitThread,
			  NULL,
			  &TowerInitThreadStack[THREAD_STACK_SIZE - 1],
			  0);
  error = OS_ThreadCreate(PITCallbackThread,
			  NULL,
			  &PITCallbackThreadStack[THREAD_STACK_SIZE - 1],
			  6);
  error = OS_ThreadCreate(RTCCallbackThread,
			  NULL,
			  &RTCCallbackThreadStack[THREAD_STACK_SIZE - 1],
			  7);
  error = OS_ThreadCreate(RxThread,
			  NULL,
			  &RxThreadStack[THREAD_STACK_SIZE - 1],
			  1);
  error = OS_ThreadCreate(TxThread,
			  NULL,
			  &TxThreadStack[THREAD_STACK_SIZE - 1],
			  2);

  error = OS_ThreadCreate(FTMCh0Thread,
			  NULL,
			  &FTMCh0ThreadStack[THREAD_STACK_SIZE - 1],
			  4);
  error = OS_ThreadCreate(FTMCh1Thread,
			  NULL,
			  &FTMCh1ThreadStack[THREAD_STACK_SIZE - 1],
			  5);


  OS_Start();



  /*** Don't write any code pass this line, or it will be deleted during code generation. ***/
  /*** RTOS startup code. Macro PEX_RTOS_START is defined by the RTOS component. DON'T MODIFY THIS CODE!!! ***/
  #ifdef PEX_RTOS_START
    PEX_RTOS_START();                  /* Startup of the selected RTOS. Macro is defined by the RTOS component. */
  #endif
  /*** End of RTOS startup code.  ***/
  /*** Processor Expert end of main routine. DON'T MODIFY THIS CODE!!! ***/
  for(;;){}
  /*** Processor Expert end of main routine. DON'T WRITE CODE BELOW!!! ***/
} /*** End of main routine. DO NOT MODIFY THIS TEXT!!! ***/

/* END main */
/*!
 ** @}
 */
/*
 ** ###################################################################
 **
 **     This file was created by Processor Expert 10.5 [05.21]
 **     for the Freescale Kinetis series of microcontrollers.
 **
 ** ###################################################################
 */
