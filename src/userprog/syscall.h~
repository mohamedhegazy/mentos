#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H
#include "threads/synch.h"
void syscall_init (void);

//new part , version 1 
struct child{
  int pid;
  int status;
  bool exit;
  int loaded;
  bool wait;
  struct list_elem elem;              
  struct semaphore terminated;
}; 	

struct child* setChild(int pid);
struct child* getChild(int pid);

#endif /* userprog/syscall.h */
