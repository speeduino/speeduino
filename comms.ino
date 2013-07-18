/*
This is called when a command is received over serial from TunerStudio / Megatune
It parses the command and calls the relevant function
A detailed description of each call can be found at: http://www.msextra.com/doc/ms1extra/COM_RS232.htm
*/
void command()
{
  switch (Serial.read()) 
  {
      case 'A': // send 22 bytes of realtime values
        sendValues(22);
        break;

      case 'B': // Burn current values to eeprom
        saveConfig();
        break;

      case 'C': // test communications. This is used by Tunerstudio to see whether there is an ECU on a given serial port
        testComm();
        break;

      case 'P': // set the current page
        //Blue
        digitalWrite(10, HIGH);
        digitalWrite(9, LOW);
        digitalWrite(13, LOW);
        
        //A 2nd byte of data is required after the 'P' specifying the new page number. 
        //This loop should never need to run as the byte should already be in the buffer, but is here just in case
        while (Serial.available() == 0) { }
        currentPage = Serial.read();
        break; 

      case 'R': // send 39 bytes of realtime values
        sendValues(39);
        break;    

      case 'S': // send code version
        Serial.write(ms_version);
        break;

      case 'Q': // send code version
        //Off
        digitalWrite(9, LOW);
        digitalWrite(10, LOW);
        digitalWrite(13, LOW);
        Serial.write(ms_version);
        break;

      case 'V': // send VE table and constants
        //Red
        digitalWrite(9, LOW);
        digitalWrite(10, LOW);
        digitalWrite(13, HIGH);
        sendPage();
        break;

      case 'W': // receive new VE or constant at 'W'+<offset>+<newbyte>
        //Green
        digitalWrite(9, HIGH);
        digitalWrite(10, LOW);
        digitalWrite(13, LOW);
        
        receiveValue(Serial.read(), Serial.read());
	break;

      default:
	break;
  } 
}

/*
This function returns the current values of a fixed group of variables
*/
void sendValues(int length)
{
  byte response[22];
  
  response[0] = (uint8_t)1; //rtc.sec;

  boolean a = 0; //inj_port1.status;
  boolean b = 0; //inj_port2.status;
  response[1] =  ((a & 0x01) << 0) | ((a & 0x02) << 1) | ((a & 0x04) << 1) | ((b & 0x01) << 1) | ((b & 0x02) << 3) | ((b & 0x04) << 3); //squirt NOT YET WORKING

  response[2] = (byte)128; // Engine Status NOT YET WORKING
  response[3] = 0x00; //baro
  response[4] = currentStatus.MAP; //map
  response[5] = 0x00; //mat
  response[6] = 0x00; //Coolant
  response[7] = 0x00; //TPS
  response[8] = 0x00; //battery voltage
  response[9] = 0x00; //O2
  response[10] = 0x00; //Exhaust gas correction (%)
  response[11] = 0x00; //Air Correction (%)
  response[12] = 0x00; //Warmup enrichment (%)
  response[13] = (currentStatus.RPM / 100); //rpm / 100
  response[14] = currentStatus.PW / 100; //Pulsewidth 1 divided by 10 (in ms)
  response[15] = 0x00; //acceleration enrichment (ms)
  response[16] = 0x00; //Barometer correction (%)
  response[17] = 0x00; //Total GammaE (%)
  response[18] = currentStatus.VE; //Current VE 1 (%)
  response[19] = 0x00; //Pulsewidth 2 divided by 10 (in ms)
  response[20] = 0x00; //mCurrent VE 2 (%)
  response[21] = 0x00; //Idle

  Serial.write(response, (size_t)22);
  return; 
}

void receiveValue(byte offset, byte newValue)
{
  
}

void saveConfig()
{
  return; 
}

void sendPage()
{
  byte response[125];
  byte offset;
  byte* pnt_configPage;
  
  switch ((int)currentPage) 
  {
      case vePage:
        //Need to perform a translation of the values[MAP/TPS][RPM] into the MS expected format
        //MS format has origin (0,0) in the bottom left corner, we use the top left for efficiency reasons
        for(byte x=0;x<64;x++) { response[x] = fuelTable.values[7-x/8][x%8]; }
        for(byte x=64;x<72;x++) { response[x] = fuelTable.axisX[(x-64)] / 100; } //RPM Bins for VE table (Need to be dvidied by 100)
        for(byte y=72;y<80;y++) { response[y] = fuelTable.axisY[7-(y-72)]; } //MAP or TPS bins for VE table 
        
        //All other bytes can simply be copied from the config table
        pnt_configPage = (byte *)&configPage1; //Create a pointer to Page 1 in memory
        offset = 80; //Offset is based on the amount already copied above (table + bins)
        for(byte x=offset;x<125;x++)
        { 
          response[offset] = *(pnt_configPage + offset + x); //Each byte is simply the location in memory of configPage1 + the offset + the variable number (x)
        }
        /*
        response[80] = configPage1.crankCold; //Cold cranking pulsewidth. This is added to the fuel pulsewidth when cranking under a temp threshold (ms)
        response[81] = configPage1.crankHot; //Warm cranking pulsewidth. This is added to the fuel pulsewidth when cranking (ms)
        response[82] = configPage1.asePct; //Afterstart enrichment (%)
        response[83] = configPage1.aseCount; //Afterstart enrichment cycles. This is the number of ignition cycles that the afterstart enrichment % lasts for
        for(byte x=84;x<94;x++) { response[x] = configPage1.wueBins[x-84]; } //Warm up enrichment array (10 bytes, % values)
        response[94] = configPage1.taeBins1; //TPS based acceleration enrichment bin 1 of 4 (ms)
        response[95] = configPage1.taeBins2; //TPS based acceleration enrichment bin 2 of 4 (ms)
        response[96] = configPage1.taeBins3; //TPS based acceleration enrichment bin 3 of 4 (ms)
        response[97] = configPage1.taeBins4; //TPS based acceleration enrichment bin 4 of 4 (ms)
        response[98] = 0;
        response[99] = 0;
        response[100] = 0;
        response[101] = 0;
        response[102] = 0;
        response[103] = 0;
        response[104] = 0;
        response[105] = 0;
        response[106] = configPage1.reqFuel;
        response[107] = 0;
        response[108] = 0;
        response[109] = 0;
        response[110] = 0;
        response[111] = 0;
        response[112] = 0;
        response[113] = 0;
        response[114] = 0; //rpmk (16 bits)
        response[116] = ((configPage1.nCylinders-1) * 16) + (1 * 8) + ((configPage1.strokes / 4) * 4) + 2; // (engineCylinders * 16) + (1 * 8) + ((engineStrokes / 4) * 4) + 4
        response[117] = 0;
        response[118] = 0;
        response[119] = 0;
        response[120] = 0;
        response[121] = 0;
        response[122] = 0;
        response[123] = 0;
        response[124] = 0;
        */

        Serial.write((uint8_t *)&response, sizeof(response));
        break;
        
      case ignPage:
        //Need to perform a translation of the values[MAP/TPS][RPM] into the MS expected format
        for(byte x=0;x<64;x++) { response[x] = ignitionTable.values[7-x/8][x%8]; }
        for(byte x=64;x<72;x++) { response[x] = ignitionTable.axisX[(x-64)] / 100; }
        for(byte y=72;y<80;y++) { response[y] = ignitionTable.axisY[7-(y-72)]; }
        
        //All other bytes can simply be copied from the config table
        pnt_configPage = (byte *)&configPage2; //Create a pointer to Page 1 in memory
        offset = 80; //Offset is based on the amount already copied above (table + bins)
        for(byte x=offset;x<125;x++)
        { 
          response[offset] = *(pnt_configPage + offset + x); //Each byte is simply the location in memory of configPage + the offset + the variable number (x)
        }
        
        Serial.write((byte *)&response, sizeof(response)); 
        break;
        
      default:
	break;
  }

  return; 
}

void testComm()
{
  Serial.write(1);
  return; 
}
