#ifndef _REFCOUNTER_H
#define _REFCOUNTER_H

void init_refcounter();
void incref(void * ptr);
void decref(void * ptr);

#endif
