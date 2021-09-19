#ifndef _MACRO_H_
#define _MACRO_H_

#define CHECK_GOTO(cond, target) if((cond)) {goto target;}
#define CHECK_RTN_VAL(cond, val) if((cond)) {return (val);}
#define CHECK_RTN(cond) if((cond)) {return;}

#endif

