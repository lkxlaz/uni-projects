/*! @file
 *
 *  @brief Routines to access the LEDs on the TWR-K70F120M.
 *
 *  This contains the functions for operating the LEDs.
 *
 *  @author LIN TAO, ANGZE LI
 *  @date 2016-08-21
 */
/*!
**  @addtogroup LEDs_module LEDs module documentation
**  @{
*/
/* MODULE LEDs */
#include "types.h"
#include "LEDs.h"
#include "MK70F12.h"

BOOL LEDs_Init(void)
{
  //enable the clock gate to port A
  SIM_SCGC5 |= SIM_SCGC5_PORTA_MASK;

  //set up correct portA pins to be general purpose output
  PORTA_PCR11 |= PORT_PCR_MUX(1);
  PORTA_PCR29 |= PORT_PCR_MUX(1);
  PORTA_PCR28 |= PORT_PCR_MUX(1);
  PORTA_PCR10 |= PORT_PCR_MUX(1);

  LEDs_Off(LED_ORANGE);
  LEDs_Off(LED_YELLOW);
  LEDs_Off(LED_GREEN);
  LEDs_Off(LED_BLUE);

  //set port data direction as general output
  GPIOA_PDDR |= LED_ORANGE;
  GPIOA_PDDR |= LED_YELLOW;
  GPIOA_PDDR |= LED_GREEN;
  GPIOA_PDDR |= LED_BLUE;



  return bTRUE;
}

void LEDs_On(const TLED color)
{
  GPIOA_PCOR = color;
}


void LEDs_Off(const TLED color)
{
  GPIOA_PSOR = color;
}

void LEDs_Toggle(const TLED color)
{
  GPIOA_PTOR = color;
}
/* END LEDs */
/*!
** @}
*/




