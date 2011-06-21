#ifndef _MACRO_H_
#define _MACRO_H_
#define ALIGN(a,b) ((((a)+(b)-1)/(b))*(b))
#define round_up(x,n) (-(-(x) & -(n)))
#endif
