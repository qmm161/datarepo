#ifndef _MACRO_H_
#define _MACRO_H_

#include "log.h"

#define CHECK_GOTO(cond, target) if((cond)) {goto target;}
#define CHECK_DO_GOTO(cond, dothing, target) if((cond)) {dothing; goto target;}
#define CHECK_RTN_VAL(cond, val) if((cond)) {return (val);}
#define CHECK_RTN(cond) if((cond)) {return;}
#define CHECK_DO_RTN_VAL(cond, dothing, val) if((cond)) {dothing; return (val);}
#define CHECK_DO_RTN(cond, dothing) if((cond)) {dothing; return;}
#define CHECK_NULL(p) if(!(p)) { LOG_WARN("Null pointer");return;}
#define CHECK_NULL2(p1, p2) if(!(p1) || !(p2)) { LOG_WARN("Null pointer");return;}
#define CHECK_NULL3(p1, p2, p3) if(!(p1) || !(p2) || !(p3)) { LOG_WARN("Null pointer");return;}
#define CHECK_NULL_RTN(p, rt) if(!(p)) { LOG_WARN("Null pointer"); return (rt);}
#define CHECK_NULL_RTN2(p1, p2, rt) if(!(p1) || !(p2)) { LOG_WARN("Null pointer"); return (rt);}
#define CHECK_NULL_RTN3(p1, p2, p3, rt) if(!(p1) || !(p2) || !(p3)) { LOG_WARN("Null pointer"); return (rt);}

#endif

