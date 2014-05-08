
//Replace the standard arduino map() function to use the div function instead
long fastMap(long x, long in_min, long in_max, long out_min, long out_max)
{
  return div( ((x - in_min) * (out_max - out_min)) , ((in_max - in_min) + out_min) ).quot;
}

