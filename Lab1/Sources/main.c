/* ###################################################################
**     Filename    : main.c
**     Project     : Lab1
**     Processor   : MK70FN1M0VMJ12
**     Version     : Driver 01.01
**     Compiler    : GNU C Compiler
**     Date/Time   : 2015-07-20, 13:27, # CodeGen: 0
**     Abstract    :
**         Main module.
**         This module contains user's application code.
**     Settings    :
**     Contents    :
**         No public methods
**
** ###################################################################*/
/*!
** @file main.c
** @version 1.0
** @brief
**         Main module.
**         This module contains user's application code.
*/         
/*!
**  @addtogroup main_module main module documentation
**  @{ FIFO, UART, packet
*/         
/* MODULE main */


// CPU mpdule - contains low level hardware initialization routines
#include "Cpu.h"
#include "FIFO.h"
#include "UART.h"
#include "packet.h"
#include "types.h"

#define CMD_STARTUP 0x04
#define CMD_TOWERVERSION 0x09
#define CMD_TOWERNUMBER 0x0B

// define packet commands
static const uint8_t CMDSTARTUP = 0x04;
static const uint8_t CMDTOWERVERSION = 0x09;
static const uint8_t CMDTOWERNUMBER = 0x0B;
const uint8_t PACKET_ACK_MASK = 0x80;

static const uint32_t BAUDRATE  = 38400;
static const uint32_t MODULE_CLK = CPU_BUS_CLK_HZ;

static uint8 TowerMSB = 0x20;
static uint8 TowerLSB = 0x8D;

/*! @brief In response to the reception of a 0x04 - get startup values packet from
 *   the pc, the tower should transmit three packets.
 *
 *  @return void
 */
void InitPackets()
{
  Packet_Put(CMDSTARTUP, 0, 0, 0);
  Packet_Put(CMDTOWERVERSION, 'v', 1, 0);
  Packet_Put(CMDTOWERNUMBER, 1, TowerLSB, TowerMSB);
}

/*! @brief Routine to handle the startup command
 *
 *  @return BOOL - bTRUE handle the packet successfully
 *  	           bFALSE not successfully
 */
BOOL HandleStartupPacket()
{
  if ((Packet_Parameter1 == 0) && (Packet_Parameter2  == 0) && (Packet_Parameter3 == 0) )
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
 *  	           bFALSE not successfully
 */
BOOL HandleVersionPacket()
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
 *  	           bFALSE not successfully
 */
BOOL HandleNumberPacket()
{
  //get mode
  if ((Packet_Parameter1 == 1) && (Packet_Parameter2 == 0) && (Packet_Parameter3 == 0))
  {
    Packet_Put(CMDTOWERNUMBER, 1, TowerLSB, TowerMSB);
    return bTRUE;
  }
  //set mode
  else if (Packet_Parameter1 == 2)
  {
    TowerLSB = Packet_Parameter2;
    TowerMSB = Packet_Parameter3;
    return bTRUE;
  }
  else
    return bFALSE;
}

/*! @Routine to handle the incoming packets with packet acknowledgement
 *
 *  @return void
 */
void HandlePacket()
{
  uint8_t command;
  BOOL success = bTRUE; // a bool variable to indicate whether the packet if carried out successfully or not

  if (Packet_Get())
  {
    uint8_t command = Packet_Command;
    command &= ~PACKET_ACK_MASK;
    switch(command)
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
}

/*lint -save  -e970 Disable MISRA rule (6.3) checking. */
int main(void)
/*lint -restore Enable MISRA rule (6.3) checking. */
{
  /* Write your local variable definition here */

  /*** Processor Expert internal initialization. DON'T REMOVE THIS CODE!!! ***/
  PE_low_level_init();
  /*** End of Processor Expert internal initialization.                    ***/

  /* Write your code here */
  if (Packet_Init(BAUDRATE, MODULE_CLK))   //send 3 startup packets upon power up
    InitPackets();

  for (;;)
  {
    UART_Poll();
    HandlePacket();
  }

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
