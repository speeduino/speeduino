

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
  Serial.print(map1/8);
  Serial.print(",");
  Serial.print(map2/8);
  Serial.print(",");
  Serial.print(map3/8);
  Serial.print(",");
  Serial.print(map4/8);
  Serial.println("");
}

void sendStats()
{
  Serial.println("Version: 0.1");
  Serial.print("Loops/s: ");
  Serial.println(loopsPerSecond);
  Serial.print("Output: ");
  Serial.println(outputValue);
}

