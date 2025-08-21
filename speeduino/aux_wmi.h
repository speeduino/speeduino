#ifndef WMI_H
#define WMI_H

void wmiControl(void);

#define WMI_TANK_IS_EMPTY() ((configPage10.wmiEmptyEnabled) ? ((configPage10.wmiEmptyPolarity) ? digitalRead(pinWMIEmpty) : !digitalRead(pinWMIEmpty)) : 1)

#endif