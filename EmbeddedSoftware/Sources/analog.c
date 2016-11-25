/*! @file
 *
 *  @brief Routines for setting up and reading from the ADC.
 *
 *  This contains the functions needed for accessing the external TWR-ADCDAC-LTC board.
 *
 *  @author LIN TAO, ANGZE LI
 *
 *  @date 2016-09-26
 */
/*!
**  @addtogroup analog_module analog module documentation
**  @{
*/

#include "SPI.h"
#include "median.h"
#include "analog.h"


#define LTC1857 7

TAnalogInput Analog_Input[ANALOG_NB_INPUTS];

BOOL Analog_Init(const uint32_t moduleClock)
{
  TSPIModule aADCSPI2;

  // set up SPI configuration for reading ADC
  aADCSPI2.isMaster = bTRUE;
  aADCSPI2.continuousClock = bFALSE;
  aADCSPI2.inactiveHighClock = bFALSE;
  aADCSPI2.changedOnLeadingClockEdge = bFALSE;
  aADCSPI2.LSBFirst = bFALSE;
  aADCSPI2.baudRate = 1000000;

  // Initialize other values 
  uint8_t channelNb, sampleNb;
  for (channelNb = 0; channelNb < ANALOG_NB_INPUTS; channelNb++)
  {
    Analog_Input[channelNb].value.l = 0;
    Analog_Input[channelNb].oldValue.l = 0;
    for (sampleNb = 0; sampleNb < ANALOG_WINDOW_SIZE; sampleNb++)
      Analog_Input[channelNb].values[sampleNb] = 0;
    Analog_Input[channelNb].putPtr = &(Analog_Input[channelNb].values[0]);
  }
  
  return SPI_Init(&aADCSPI2, moduleClock);
}

BOOL Analog_Get(const uint8_t channelNb)
{
  uint16_t commandByte;

  SPI_SelectSlaveDevice(LTC1857);              

  switch (channelNb)
  {
    case 0:
      // command byte: channel 0, single-ended, +-10V
      commandByte = 0x8400;
      break;
    case 1:
      //command byte: channel 1, single-ended, +-10V
      commandByte = 0xC400;
      break;
    default:
      return bFALSE;
  }

  // point to the first element if reached the end
  if (Analog_Input[channelNb].putPtr == &(Analog_Input[channelNb].values[ANALOG_WINDOW_SIZE - 1]))
    Analog_Input[channelNb].putPtr = &(Analog_Input[channelNb].values[0]);
  else
    // else move forward
    Analog_Input[channelNb].putPtr++;

  // send the command byte to start a conversion
  SPI_ExchangeChar(commandByte, Analog_Input[channelNb].putPtr);
  // receive data from ADC module
  SPI_ExchangeChar(commandByte, Analog_Input[channelNb].putPtr);
  Analog_Input[channelNb].oldValue = Analog_Input[channelNb].value;
  Analog_Input[channelNb].value.l = Median_Filter(Analog_Input[channelNb].values, ANALOG_WINDOW_SIZE);

  return bTRUE;
}

/*!
** @}
*/
