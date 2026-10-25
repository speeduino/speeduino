/**
 * @file 
 * @brief Honda J 32 (3.2 liter 6 cyl SOHC) decoder.
 * 
 *  The Honda J32a4 (and all J series I'm aware of) has a crank trigger with nominal 24 teeth (22 teeth actually present). 
 *  It has one missing tooth, then 7 teeth, then another missing tooth, then 15 teeth.
 *  The tooth rising edges all have uniform spacing between them, except for teeth 14 and 22, which measure about
 *  18 degrees between rising edges, rather than 15 degrees as the other teeth do.  These slightly larger
 *  teeth are immediately before a gap, and the extra 3 degrees is made up for in the gap, the gap being about
 *  3 degrees smaller than might be nominally expected, such that the expected rotational angle is restored immediately after
 *  the gap (missing tooth) passes.  
 *  Teeth are represented as 0V at the ECU, no teeth are represented as 5V.
 *  Top dead center of cylinder number 1 occurs as we lose sight of (just pass) the first tooth in the string of 15 teeth
 *  (this is a rising edge).
 *  The second tooth in the string of 15 teeth is designated as tooth 1 in this code. This means that
 *  when the interrupt for tooth 1 fires (as we just pass tooth 1), crank angle = 360/24 = 15 degrees.
 *  It follows that the first tooth in the string of 7 teeth is tooth 16. 
 */
#pragma once

#include "shared/decoder_state.h"
#include "config_pages.h"
#include "statuses.h"

namespace decoders {

namespace hondaj32 {

    shared::state_t initialise(void);

}
}