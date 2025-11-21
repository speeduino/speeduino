#ifndef INIT_H
#define INIT_H

#include "config_pages.h"
#include "statuses.h"

void initialiseAll(void);
void setPinMapping(byte boardID);
void changeHalfToFullSync(const config2 &page2, const config4 &page4, statuses &current);
void changeFullToHalfSync(const config2 &page2, const config4 &page4, statuses &current);

#define VSS_USES_RPM2() (isExternalVssMode(configPage2) && (pinVSS == pinTrigger2) && (!getDecoderFeatures().hasSecondary)) // VSS is on the same pin as RPM2 and RPM2 is not used as part of the decoder
#define FLEX_USES_RPM2() ((configPage2.flexEnabled > 0U) && (pinFlex == pinTrigger2) && (!getDecoderFeatures().hasSecondary)) // Same as above, but for Flex sensor

#endif