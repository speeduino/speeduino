#include "dwell.h"

uint16_t computeDwell(const statuses &current, const config2 &page2, const config4 &page4, const table3d4RpmLoad &dwellTable)
{
    uint16_t dwell;
    if ( current.engineIsCranking )
    {
        dwell = page4.dwellCrank; //use cranking dwell
    }
    else 
    {
        if ( page2.useDwellMap == true )
        {
            dwell = get3DTableValue(&dwellTable, current.ignLoad, current.RPM); //use running dwell from map
        }
        else
        {
            dwell =  page4.dwellRun; //use fixed running dwell
        }
    }
      
    // Dwell is stored as ms * 10. ie Dwell of 4.3ms would be 43 in configPage4. 
    // This number therefore needs to be multiplied by 100 to get dwell in uS
    return dwell * 100U;
}