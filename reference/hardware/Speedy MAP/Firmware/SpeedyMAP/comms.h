#define SERIAL_BUFFER_THRESHOLD 1
byte cmdPending = false;
byte currentCommand;

void command();//This is the heart of the Command Line Interpeter.  All that needed to be done was to make it human readable.
void sendCSV();
