
//Replace the standard arduino map() function to use the div function instead
int fastMap(int x, int in_min, int in_max, int out_min, int out_max)
{
  return div( ((x - in_min) * (out_max - out_min)) , (in_max - in_min) ).quot + out_min;
}

/*
The following are all fast versions of specific divisions
Ref: http://www.hackersdelight.org/divcMore.pdf
*/

//Unsigned divide by 10
unsigned divu10(unsigned n) {
 unsigned q, r;
 q = (n >> 1) + (n >> 2);
 q = q + (q >> 4);
 q = q + (q >> 8);
 q = q + (q >> 16);
 q = q >> 3;
 r = n - q*10;
 return q + ((r + 6) >> 4);
// return q + (r > 9);
}

//Signed divide by 10
int divs10(int n) {
 int q, r;
 n = n + (n>>31 & 9);
 q = (n >> 1) + (n >> 2);
 q = q + (q >> 4);
 q = q + (q >> 8);
 q = q + (q >> 16);
 q = q >> 3;
 r = n - q*10;
 return q + ((r + 6) >> 4);
// return q + (r > 9);
}
