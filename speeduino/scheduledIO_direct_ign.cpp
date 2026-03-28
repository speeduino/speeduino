#include "globals.h"

void coil1Low_DIRECT(void)  {       (*ign1_pin_port &= ~(ign1_pin_mask)); }
void coil1High_DIRECT(void)  {      (*ign1_pin_port |= (ign1_pin_mask)); }
void coil2Low_DIRECT(void)  {       (*ign2_pin_port &= ~(ign2_pin_mask)); }
void coil2High_DIRECT(void)  {      (*ign2_pin_port |= (ign2_pin_mask)); }
void coil3Low_DIRECT(void)  {       (*ign3_pin_port &= ~(ign3_pin_mask)); }
void coil3High_DIRECT(void)  {      (*ign3_pin_port |= (ign3_pin_mask)); }
void coil4Low_DIRECT(void)  {       (*ign4_pin_port &= ~(ign4_pin_mask)); }
void coil4High_DIRECT(void)  {      (*ign4_pin_port |= (ign4_pin_mask)); }
void coil5Low_DIRECT(void)  {       (*ign5_pin_port &= ~(ign5_pin_mask)); }
void coil5High_DIRECT(void)  {      (*ign5_pin_port |= (ign5_pin_mask)); }
void coil6Low_DIRECT(void)  {       (*ign6_pin_port &= ~(ign6_pin_mask)); }
void coil6High_DIRECT(void)  {      (*ign6_pin_port |= (ign6_pin_mask)); }
void coil7Low_DIRECT(void)  {       (*ign7_pin_port &= ~(ign7_pin_mask)); }
void coil7High_DIRECT(void)  {      (*ign7_pin_port |= (ign7_pin_mask)); }
void coil8Low_DIRECT(void)  {       (*ign8_pin_port &= ~(ign8_pin_mask)); }
void coil8High_DIRECT(void)  {      (*ign8_pin_port |= (ign8_pin_mask)); }
