/*
This file is used for everything related to maps/tables including their definition, functions etc
*/

struct table {
  //All tables must be the same size for simplicity
  const static int xSize = 8;
  const static int ySize = 8;
  
  int values[xSize][ySize];
  //static boolean useInterp = false; //Whether or not interpolation should be used (Assuming we have enough CPU for it)
  
};

int getTableValue(struct table fromTable, int X, int Y)
  {
     return 1; 
  }
