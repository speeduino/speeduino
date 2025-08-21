#include "aux_wmi.h"
#include "globals.h"
#include "units.h"
#include "aux_vvt.h" //Required as timer is shared with VVT2

// Water methanol injection control
void wmiControl(void)
{
  int wmiPW = 0;
  
  // wmi can only work when vvt2 is disabled 
  if( (configPage10.vvt2Enabled == 0) && (configPage10.wmiEnabled >= 1) )
  {
    if( WMI_TANK_IS_EMPTY() )
    {
      BIT_CLEAR(currentStatus.status4, BIT_STATUS4_WMI_EMPTY);
      if( (currentStatus.TPS >= configPage10.wmiTPS) && (currentStatus.RPMdiv100 >= configPage10.wmiRPM) && ( (currentStatus.MAP / 2) >= configPage10.wmiMAP) && ( temperatureAddOffset(currentStatus.IAT) >= configPage10.wmiIAT) )
      {
        switch(configPage10.wmiMode)
        {
        case WMI_MODE_SIMPLE:
          // Simple mode - Output is turned on when preset boost level is reached
          wmiPW = 200;
          break;
        case WMI_MODE_PROPORTIONAL:
          // Proportional Mode - Output PWM is proportionally controlled between two MAP values - MAP Value 1 = PWM:0% / MAP Value 2 = PWM:100%
          wmiPW = map(currentStatus.MAP/2, configPage10.wmiMAP, configPage10.wmiMAP2, 0, 200);
          break;
        case WMI_MODE_OPENLOOP:
          //  Mapped open loop - Output PWM follows 2D map value (RPM vs MAP) Cell value contains desired PWM% [range 0-100%]
          wmiPW = get3DTableValue(&wmiTable, (uint16_t)currentStatus.MAP, currentStatus.RPM);
          break;
        case WMI_MODE_CLOSEDLOOP:
          // Mapped closed loop - Output PWM follows injector duty cycle with 2D correction map applied (RPM vs MAP). Cell value contains correction value% [nom 100%] 
          wmiPW = max(0, ((int)currentStatus.PW1 + configPage10.wmiOffset)) * get3DTableValue(&wmiTable, (uint16_t)currentStatus.MAP, currentStatus.RPM) / 200;
          break;
        default:
          // Wrong mode
          wmiPW = 0;
          break;
        }
        if (wmiPW > 200) { wmiPW = 200; } //without this the duty can get beyond 100%
      }
    }
    else { BIT_SET(currentStatus.status4, BIT_STATUS4_WMI_EMPTY); }

    currentStatus.wmiPW = wmiPW;
    vvt2_pwm_value = halfPercentage(currentStatus.wmiPW, vvt_pwm_max_count);

    if(wmiPW == 0)
    {
      // Make sure water pump is off
      VVT2_PIN_LOW();
      vvt2_pwm_state = false;
      vvt2_max_pwm = false;
      if( configPage6.vvtEnabled == 0 ) { DISABLE_VVT_TIMER(); }
      digitalWrite(pinWMIEnabled, LOW);
    }
    else
    {
      digitalWrite(pinWMIEnabled, HIGH);
      if (wmiPW >= 200)
      {
        // Make sure water pump is on (100% duty)
        VVT2_PIN_HIGH();
        vvt2_pwm_state = true;
        vvt2_max_pwm = true;
        if( configPage6.vvtEnabled == 0 ) { DISABLE_VVT_TIMER(); }
      }
      else
      {
        vvt2_max_pwm = false;
        ENABLE_VVT_TIMER();
      }
    }
  }
}