#ifndef SYNC_API_H
#define SYNC_API_H

//FIXME: Make this a more proper type
typedef int carbon_mutex_t;

// These are the dummy functions that will get replaced
// by the simulator
extern "C" {
   void mutexInit(carbon_mutex_t *mux);
   void mutexLock(carbon_mutex_t *mux);
   void mutexUnlock(carbon_mutex_t *mux);
}

bool isMutexValid(carbon_mutex_t *mux);

#endif
