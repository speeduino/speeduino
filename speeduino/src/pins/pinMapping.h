
/**
 * @file 
 * @brief Pin translation functions
 */

#include <stdint.h>

/**
 * @brief Translate between the pin list that appears in TS and the actual pin numbers.
 * 
 * For the **digital IO**, this will simply return the same number as the rawPin value as 
 * those are mapped directly. For **analog pins**, it will translate them into the correct internal 
 * pin number.
* @param rawPin High level pin number
* @return Translated / usable pin number
*/
uint8_t pinTranslate(uint8_t rawPin);

/**
 * @brief Translate a pin number (0 - 22) to the relevant Ax (analog) pin reference.
 * 
 * This is required as some ARM chips do not have all analog pins in order (EG pin A15 != A14 + 1).
 * @param rawPin 
 * @return uint8_t 
 */
uint8_t pinTranslateAnalog(uint8_t rawPin);
