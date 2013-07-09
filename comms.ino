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
        digitalWrite(10, HIGH);
        digitalWrite(9, LOW);
        digitalWrite(13, LOW);
        currentPage = Serial.read(); //Not doing anything with this currently, but need to read the 2nd byte from the buffer
        break; 

      case 'R': // send 39 bytes of realtime values
        sendValues(39);
        break;    

      case 'S': // send code version
        Serial.write(ms_version);
        break;

      case 'Q': // send code version
        digitalWrite(9, LOW);
        digitalWrite(10, LOW);
        digitalWrite(13, LOW);
        Serial.write(ms_version);
        break;

      case 'V': // send VE table and constants
        digitalWrite(9, LOW);
        digitalWrite(10, LOW);
        digitalWrite(13, HIGH);
        sendPage();
        break;

      case 'W': // receive new VE or constant at 'W'+<offset>+<newbyte>
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
  
  switch (currentPage) 
  {
      case vePage:
        Serial.write((uint8_t *)&fuelTable.values, sizeof(fuelTable.values));
        break;
      case ignPage:
        Serial.write((uint8_t *)&ignitionTable.values, sizeof(ignitionTable.values));
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
