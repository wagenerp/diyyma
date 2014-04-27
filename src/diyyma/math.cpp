
#include "diyyma/math.h"

float modf(float a, float b) { return a-(int)(a/b)*b; }
double modf(double a, double b) { return a-(int)(a/b)*b; }


