/*! @file
 *
 *  @brief Routines to implement a FIFO buffer.
 *
 *  This contains the structure and "methods" for accessing a byte-wide FIFO.
 *
 *  @author LIN TAO, ANGZE LI
 *  @date 2016-08-03
 */
/*!
**  @addtogroup FIFO_module FIFO module documentation
**  @{
*/
/* MODULE FIFO */
// new types
#include "FIFO.h"
#include "types.h"
#include "PE_Types.h"
#include "OS.h"

void FIFO_Init(TFIFO * const FIFO)
{

  FIFO->BufferAccess = OS_SemaphoreCreate(1);
  FIFO->SpaceAvailable = OS_SemaphoreCreate(FIFO_SIZE);
  FIFO->ItemsAvailable = OS_SemaphoreCreate(0);

  FIFO->Start = 0;
  FIFO->End = 0;
  FIFO->NbBytes = 0;
  FIFO->Buffer[FIFO_SIZE];

}


void FIFO_Put(TFIFO * const FIFO, const uint8_t data)
{
  OS_ERROR error;

  error = OS_SemaphoreWait(FIFO->SpaceAvailable, 0);
  error = OS_SemaphoreWait(FIFO->BufferAccess, 0);

  FIFO->Buffer[FIFO->End] = data;
  FIFO->NbBytes++;
  FIFO->End =  (FIFO->End + 1) % FIFO_SIZE;

  OS_SemaphoreSignal(FIFO->BufferAccess);
  OS_SemaphoreSignal(FIFO->ItemsAvailable);

}

void FIFO_Get(TFIFO * const FIFO, uint8_t * const dataPtr)
{
  OS_ERROR error;

  error = OS_SemaphoreWait(FIFO->ItemsAvailable, 0);
  error = OS_SemaphoreWait(FIFO->BufferAccess, 0);

  *dataPtr = FIFO->Buffer[FIFO->Start];
  FIFO->NbBytes--;
  FIFO->Start =  (FIFO->Start + 1) % FIFO_SIZE;

  OS_SemaphoreSignal(FIFO->BufferAccess);

  OS_SemaphoreSignal(FIFO->SpaceAvailable);

}
/* END FIFO */
/*!
** @}
*/


