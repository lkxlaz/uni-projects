/*! @file
 *
 *  @brief Median filter.
 *
 *  This contains the functions for performing Array median filter on half-word-sized data.
 *
 *  @author Angze Li, Lin Tao
 *  @date 2016-09-24
 */
/*!
**  @addtogroup median_module median module documentation
**  @{
*/

// New types
#include "types.h"
#include "median.h"

int16_t Array[101];
uint32_t Size;

static void quicksort(int32_t left,int32_t right)
{
  int32_t i,j;
  int16_t pivot, temp;
  if (left > right)
    return;

  pivot = Array[left];
  i = left;
  j = right;
  while (i != j)
  {

    while (Array[j]>=pivot && i<j)
      j--;

    while (Array[i]<=pivot && i<j)
      i++;

    if (i<j)
    {
      temp=Array[i];
      Array[i]=Array[j];
      Array[j]=temp;
    }
  }

  Array[left]=Array[i];
  Array[i]=pivot;

  quicksort(left,i-1);
  quicksort(i+1,right);
}

int16_t Median_Filter(const int16_t array[], const uint32_t size)
{
  int16_t median;
  Size = (uint32_t)size;

  uint32_t i;
  for (i = 0; i < size; i++)
    Array[i] = (int16_t)array[i];

  quicksort(0,Size-1);

  if ((size % 2) == 1)
    median = Array[((size - 1) / 2)];

  else
    median = (Array[((size - 2) / 2)] + Array[(size / 2)]) / 2;

  return median;

}

/* END median */
/*!
** @}
*/
