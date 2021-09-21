#ifndef _MACRO_H_
#define _MACRO_H_

#define CHECK_GOTO(cond, target) if((cond)) {goto target;}
#define CHECK_RTN_VAL(cond, val) if((cond)) {return (val);}
#define CHECK_RTN(cond) if((cond)) {return;}
#define CHECK_DO_RTN_VAL(cond, dothing, val) if((cond)) {dothing; return (val);}
#define CHECK_DO_RTN(cond, dothing) if((cond)) {dothing; return;}

#endif

