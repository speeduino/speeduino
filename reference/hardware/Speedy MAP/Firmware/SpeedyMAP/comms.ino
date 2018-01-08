

void command()
{

  if (cmdPending == false) { currentCommand = Serial.read(); }

  switch (currentCommand)
  {

    case 'S': // send diag stats
      sendStats();
      break;

    case 'C': //Toggle continuous data send mode
      if(serialStream == false) { serialStream = true; }
      else { serialStream = false; }
      break;

  }

}

void sendCSV()
{
  Serial.write(map1/10);
  Serial.print(",");
  Serial.write(map2/10);
  Serial.print(",");
  Serial.write(map3/10);
  Serial.print(",");
  Serial.write(map4/10);
  Serial.println("");
}

void sendStats()
{
  Serial.println("Version: 0.1");
  Serial.print("Loops/s: ");
  Serial.write(loopsPerSecond);
  Serial.println("");
}

