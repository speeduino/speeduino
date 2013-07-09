void command()
{
  switch (Serial.read()) 
  {
      case 'A': // send 22 bytes of realtime values
        sendValues(22);
        break;

      case 'B': // store to eeprom
        saveConfig();
        break;

      case 'C': // test communications
        testComm();
        break;

      case 'P': // set the current page
        //Blue
        digitalWrite(10, HIGH);
        digitalWrite(9, LOW);
        digitalWrite(13, LOW);
        while (Serial.available() == 0) 
        {
        }
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
        Serial.read();
        Serial.read(); //Not doing anything with this currently, but need to read the next 2 bytes from the buffer
	break;

      default:
	break;
  } 
}

void sendValues(int length)
{
  byte response[22];
  
  response[0] = (uint8_t)1; //rtc.sec;

  boolean a = 0; //inj_port1.status;
  boolean b = 0; //inj_port2.status;
  response[1] =  ((a & 0x01) << 0) | ((a & 0x02) << 1) | ((a & 0x04) << 1) | ((b & 0x01) << 1) | ((b & 0x02) << 3) | ((b & 0x04) << 3); //squirt

  response[2] = 0; // Engine Status 
  response[3] = 0x00; //baro
  response[4] = 0x00; //map
  response[5] = 0x00; //mat
  response[6] = 0x00; //Coolant
  response[7] = 0x00; //TPS
  response[8] = 0x00; //battery voltage
  response[9] = 0x00; //O2
  response[10] = 0x00; //Exhaust gas correction (%)
  response[11] = 0x00; //Air Correction (%)
  response[12] = 0x00; //Warmup enrichment (%)
  response[13] = (rpm / 100); //rpm / 100
  response[14] = 0x00; //Pulsewidth 1 divided by 10 (in ms)
  response[15] = 0x00; //acceleration enrichment (ms)
  response[16] = 0x00; //Barometer correction (%)
  response[17] = 0x00; //Total GammaE (%)
  response[18] = 0x00; //Current VE 1 (%)
  response[19] = 0x00; //Pulsewidth 2 divided by 10 (in ms)
  response[20] = 0x00; //mCurrent VE 2 (%)
  response[21] = 0x00; //Idle

  Serial.write(response, (size_t)22);
  return; 
}

void saveConfig()
{
  return; 
}

void sendPage()
{
  byte response[125];
  
  switch ((int)currentPage) 
  {
      case vePage:
        //Need to perform a translation of the values[MAP/TPS][RPM] into the MS expected format
        for(byte x=0;x<64;x++) { response[x] = fuelTable.values[7-x/8][x%8]; }
        
        response[64] = 0;
        response[65] = 0;
        response[66] = 0;
        response[67] = 0;
        response[68] = 0;
        response[78] = 0;
        response[79] = 0;
        response[80] = 0;
        response[81] = 0;
        response[82] = 0;
        response[83] = 0;
        response[84] = 0;
        response[85] = 0;
        response[86] = 0;        
        response[87] = 0;
        response[88] = 0;
        response[89] = 0;
        response[90] = (byte)req_fuel;
        response[91] = 0;
        response[92] = 0;
        response[93] = 0;
        response[94] = 0;
        response[95] = 0;
        response[96] = 0;
        response[97] = 0;
        response[98] = 0;
        response[99] = 0;
        for(byte x=100;x<108;x++) { response[x] = fuelTable.axisX[(x-100)] / 100; }
        for(byte y=108;y<116;y++) { response[y] = fuelTable.axisY[7-(y-108)]; }
        response[116] = 0;
        response[117] = 0;
        response[118] = 0;
        response[119] = 0;
        response[120] = 0;
        response[121] = 0;
        response[122] = 0;
        response[123] = 0;
        response[124] = 0;

        Serial.write((uint8_t *)&response, sizeof(response));
        break;
      case ignPage:
        //Need to perform a translation of the values[MAP/TPS][RPM] into the MS expected format
        for(byte x=0;x<64;x++) { response[x] = ignitionTable.values[7-x/8][x%8]; }
        for(byte x=64;x<72;x++) { response[x] = ignitionTable.axisX[(x-64)] / 100; }
        for(byte y=72;y<80;y++) { response[y] = ignitionTable.axisY[7-(y-72)]; }
        response[80] = 0;
        response[81] = 0;
        response[82] = 0;
        response[83] = 0;
        response[84] = 0;
        response[85] = 0;
        response[86] = 0;        
        response[87] = 0;
        response[88] = 0;
        response[89] = 0;
        response[90] = 0;
        response[91] = 0;
        response[92] = 0;
        response[93] = 0;
        response[94] = 0;
        response[95] = 0;
        response[96] = 0;
        response[97] = 0;
        response[98] = 0;
        response[99] = 0;
        response[100] = 0;
        response[101] = 0;
        response[102] = 0;
        response[103] = 0;
        response[104] = 0;
        response[105] = 0;
        response[106] = 0;
        response[107] = 0;
        response[108] = 0;
        response[109] = 0;
        response[110] = 0;
        response[111] = 0;
        response[112] = 0;
        response[113] = 0;
        response[114] = 0;
        response[115] = 0;
        response[116] = 0;
        response[117] = 0;
        response[118] = 0;
        response[119] = 0;
        response[120] = 0;
        response[121] = 0;
        response[122] = 0;
        response[123] = 0;
        response[124] = 0;
        Serial.write((uint8_t *)&response, sizeof(response));
        
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
