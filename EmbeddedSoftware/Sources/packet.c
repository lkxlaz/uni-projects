/*! @file
 *
 *  @brief I/O routines for UART communications on the TWR-K70F120M.
 *
 *  This contains the functions for operating the UART (serial port).
 *
 *  @author ANGZE LI, LIN TAO
 *  @date 2016-08-08
 */
/*!
 **  @addtogroup packet_module packet module documentation
 **  @{
 */
/* MODULE packet */
#include "packet.h"
#include "UART.h"
#include "types.h"
#include "OS.h"

TPacket Packet;

BOOL Packet_Init(const uint32_t baudRate, const uint32_t moduleClk)
{

  Packet_PutSemaphore = OS_SemaphoreCreate(1);
  Packet_HandleSemaphore = OS_SemaphoreCreate(1);

  return UART_Init(baudRate, moduleClk);

}

BOOL Packet_Get(void)
{
  static int guard = 0;
  static uint8_t checksum;

  switch(guard)
  {
    case 0:
      UART_InChar(&Packet_Command);
      guard++;
      break;
    case 1:
      UART_InChar(&Packet_Parameter1);
      guard++;
      break;
    case 2:
      UART_InChar(&Packet_Parameter2);
      guard++;
      break;
    case 3:
      UART_InChar(&Packet_Parameter3);
      guard++;
      break;
    case 4:
      UART_InChar(&checksum);
      guard++;
      break;
    case 5:
      if (checksum == (Packet_Command ^ Packet_Parameter1 ^ Packet_Parameter2 ^ Packet_Parameter3))
      {
	guard = 0;
	return bTRUE;
      }
      else
      {
	Packet_Command = Packet_Parameter1;
	Packet_Parameter1 = Packet_Parameter2;
	Packet_Parameter2 = Packet_Parameter3;
	Packet_Parameter3 = checksum;
	guard--;
      }
    default:
      break;
  }
  return bFALSE;
}


void Packet_Put(const uint8_t command, const uint8_t parameter1, const uint8_t parameter2, const uint8_t parameter3)
{

  OS_SemaphoreWait(Packet_PutSemaphore, 0);
  UART_OutChar(command);
  UART_OutChar(parameter1);
  UART_OutChar(parameter2);
  UART_OutChar(parameter3);
  UART_OutChar(command ^ parameter1 ^ parameter2 ^ parameter3);
  OS_SemaphoreSignal(Packet_PutSemaphore);

}
/* END packet */
/*!
 ** @}
 */

