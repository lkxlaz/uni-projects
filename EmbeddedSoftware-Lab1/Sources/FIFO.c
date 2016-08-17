/*! @file
 *
 *  @brief Routines to implement a FIFO buffer.
 *
 *  This contains the structure and "methods" for accessing a byte-wide FIFO.
 *
 *  @author LIN TAO, ANGZE LI
 *  @date 2016-08-03
 */


// new types
#include "FIFO.h"
#include "types.h"

void FIFO_Init(TFIFO * const FIFO) // why is TFIFO; F->buf = buf;
{
  FIFO->Start = 0;
  FIFO->End = 0;
  FIFO->NbBytes = 0;
}


BOOL FIFO_Put(TFIFO * const FIFO, const uint8_t data)
{
  // check if the buffer is full
  // if yes, return bFALSE
  // if no, put a data into the buffer
  if (FIFO->NbBytes == FIFO_SIZE)
    return bFALSE;

  FIFO->Buffer[FIFO->Start] = data;
  FIFO->Start++;
  FIFO->NbBytes++;

  // check to see if needed to warp around
  if (FIFO->Start == FIFO_SIZE)
    FIFO->Start =0;

  return bTRUE;

}

BOOL FIFO_Get(TFIFO * const FIFO, uint8_t * const dataPtr)
{
  // check if the buffer is empty
  // if yes, return bFALSE
  // if no, get a data from the buffer
  if (FIFO->NbBytes == 0)
    return bFALSE;

  *dataPtr = FIFO->Buffer[FIFO->End];
  FIFO->End++;
  FIFO->NbBytes--;
  // check for warp around
  if (FIFO->End == FIFO_SIZE)
    FIFO->End = 0;

  return bTRUE;

}


