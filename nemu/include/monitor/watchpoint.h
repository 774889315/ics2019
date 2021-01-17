#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  
  // struct watchpoint *prev; /* the previous Watchpoint  */

  char expr[32];

  int Old_value, New_value;
  int value, hit;
   
} WP;

bool watchpoint_monitor();
#endif
