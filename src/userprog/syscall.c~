#include "userprog/syscall.h"
#include "user/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "filesys/file.h"
#include "filesys/inode.h"
#include "threads/malloc.h"
#include "threads/synch.h"
#include "threads/vaddr.h"
#include <list.h>

struct file_owned{
    struct file * file;        /* File's pointer. */
    struct list_elem elem;              /* Element in file list. */   
    int file_descriptor;
    	
};
static void syscall_handler (struct intr_frame *);
struct lock * file_lock;
void
syscall_init (void) 
{
    intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
    lock_init(&file_lock);
}
void * valid_refrence (void * ptr){
if(!is_user_vaddr ((int *)ptr) || ptr< (void *)0x08048000)
thread_exit(); 
return ptr;

}
static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  printf ("system call!\n");
//  thread_exit ();
  
  int call=*((int *)valid_refrence(f->esp));
  bool val;
  int value;
  switch(call){
  case SYS_HALT:
  halt();
  break;
  case SYS_EXIT:
  exit(*((int *)valid_refrence(f->esp+1)));
  break;
  case SYS_EXEC:  
  value=exec(*((int *)valid_refrence(f->esp+1)));
  f->eax=value;	
  break;
  case SYS_WAIT:
  value=wait(*((int *)valid_refrence(f->esp+1)));
  f->eax=value;
  break;
  case     SYS_CREATE:
  val=create(*((char *)valid_refrence(f->esp+1)),*((unsigned *)valid_refrence(f->esp+2)));
  f->eax=val;
  break;
  case     SYS_REMOVE:
  val=remove(*((char *)valid_refrence(f->esp+1)));
  f->eax=val;
  break;
  case    SYS_OPEN:
  value=open(*((char *)valid_refrence(f->esp+1)));
  f->eax=value;  
  break;
  case SYS_FILESIZE:
  value=filesize(*((int *)valid_refrence(f->esp+1)));  
    f->eax=value;  
  break;
  case SYS_READ:
  value=read(*((int *)valid_refrence(f->esp+1)),((void *)valid_refrence(f->esp+2)),*((unsigned *)valid_refrence(f->esp+3)));  
    f->eax=value;  
  break;  
  case SYS_WRITE:
  value=write(*((int *)valid_refrence(f->esp+1)),((void *)valid_refrence(f->esp+2)),*((unsigned *)valid_refrence(f->esp+3)));  
    f->eax=value;  
  break;
  case SYS_SEEK:
  seek(*((int *)valid_refrence(f->esp+1)),*((unsigned *)valid_refrence(f->esp+2)));  
  break;
  case SYS_TELL:
  value=tell(*((int *)valid_refrence(f->esp+1)));
  f->eax=value;
  break;
  case SYS_CLOSE:
  close(*((int *)valid_refrence(f->esp+1)));
  break;
  }
}


/*
Terminates Pintos by calling shutdown_power_off()(declared
in ‘devices/shutdown.h’).This should be seldom used, because you lose
some information about possible deadlock situations, etc.
*/
void halt(void)
{
  shutdown_power_off();	
}

/*
Terminates the current user program, returning status to the kernel. If the process’s parent waits for it (see below), this is the status that will be returned. Conventionally,
a status of 0 indicates success and nonzero values indicate errors
*/
void exit (int status)
{
  struct thread *t=thread_current();
  printf ("%s: exit(%d)\n", t->name, status);
  t->child1->status=status;
  sema_up(&t->child1->terminated);
  thread_exit();
}

/*
Runs the executable whose name is given in cmd line, passing any given arguments,
and returns the new process’s program id (pid). Must return pid -1, which otherwise should not be a valid pid, if the program cannot load or run for any reason. Thus,
the parent process cannot return from the exec until it knows whether the child process successfully loaded its executable. You must use appropriate synchronization to ensure this.
*/
pid_t exec(const char *cmd_line)
{
  pid_t pid=process_execute(cmd_line);
  struct child* cp=getChild(pid);
  while(cp->loaded == 0){
  	//terminated.wait();
  }
  if(cp->loaded == 2)
  {
	return -1;
  }	
  return pid;
}



int wait (pid_t pid)
{
  process_wait(pid);
}

//setting a new child with the given pid
struct child* setChild(int pid){
  struct child* newChild=malloc(sizeof(struct child));
  newChild->pid=pid;
  newChild->exit=false;
  newChild->wait=false;
  newChild->status=-1;
  newChild->loaded=0;
  sema_init(&newChild->terminated,1);
  list_push_back(&thread_current()->children,&newChild->elem);
  return newChild;
}


/*get the child with the given pid from the children list of the current thread */
struct child* getChild(int pid){
  struct thread *t=thread_current();
  struct list_elem *element;
  
  for(element=list_begin(&t->children); element!=list_end(&t->children);element=list_next(element)){
	struct child *newChild=list_entry(element,struct child,elem);	  
	if(pid == newChild->pid)
		{
			return newChild;
		}
  	}
	return NULL;
}








bool create (const char * file , unsigned initial_size ){
lock_acquire(&file_lock);
bool value=filesys_create (file, initial_size); 
lock_release (&file_lock); 
return value;
}
bool remove (const char * file ){
 lock_acquire(&file_lock);
 bool value=filesys_remove(file);
 lock_release (&file_lock); 
 return value;
}
int open (const char * file ){
    lock_acquire (&file_lock); 
    struct file * opened=filesys_open(file);
    if(opened== NULL){
    lock_release (&file_lock); 
    return -1;
    }
    thread_current()->file_descriptor=thread_current()->file_descriptor+1;
    int file_desc=thread_current()->file_descriptor;
    struct file_owned *owned=malloc(sizeof(struct file_owned));
    owned->file=opened;
    owned->file_descriptor=file_desc;
    list_insert(&thread_current()->files_owned,owned);
    lock_release (&file_lock); 
    return file_desc;
}
int filesize (int fd ){
if(fd<=0 || fd == 1 )
return;
lock_acquire (&file_lock); 
	
 struct list_elem *e;
 struct file_owned *owned;
 	off_t length=-1;
  for (e = list_begin (&thread_current()->files_owned); e != list_end (&thread_current()->files_owned);
       e = list_next (e)) 
    {
      owned = list_entry (e, struct file_owned, elem);
      if (owned->file_descriptor==fd) 
        {
	length=file_length(owned->file);
	lock_release (&file_lock); 
	}
    }
    lock_release (&file_lock); 
return length;
        }
int read (int fd , void * buffer , unsigned size ){
if(fd<0 || fd == 1 )
return -1;
if(size<=0)
return -1;

if(fd==0)
{
int count=0;
char * temp_buffer=*((char *)buffer);
while(count<size){
temp_buffer[count++]=input_getc();
}
return size;
}
else{
        lock_acquire (&file_lock); 
 struct list_elem *e;
 struct file_owned *owned;
 off_t read=-1;
  for (e = list_begin (&thread_current()->files_owned); e != list_end (&thread_current()->files_owned);
       e = list_next (e)) 
    {
      owned = list_entry (e, struct file_owned, elem);
      if (owned->file_descriptor==fd) 
        {
	
	struct file *f=	file_open(owned->file);
	read=file_read (f, buffer, size); 
	
        }
    }

		lock_release (&file_lock); 

		return read;
}
}
int write (int fd , const void * buffer , unsigned size ){
if(fd<=0)
return -1;
if(size<=0)
return -1;
if(fd==1){
putbuf(buffer,size);
return size;
}
else{
        lock_acquire (&file_lock); 

 struct list_elem *e;
 struct file_owned *owned;
 off_t written=0;
  for (e = list_begin (&thread_current()->files_owned); e != list_end (&thread_current()->files_owned);
       e = list_next (e)) 
    {
      owned = list_entry (e, struct file_owned, elem);
      if (owned->file_descriptor==fd) 
        {
	written=file_write (owned->file, buffer, size); 
        }
    }
    	lock_release (&file_lock); 

return written;
}
}
void seek (int fd , unsigned position ){
if(fd<=0 || fd == 1 )
return;
        	lock_acquire (&file_lock); 

 struct list_elem *e;
 struct file_owned *owned;
  for (e = list_begin (&thread_current()->files_owned); e != list_end (&thread_current()->files_owned);
       e = list_next (e)) 
    {
      owned = list_entry (e, struct file_owned, elem);
      if (owned->file_descriptor==fd) 
        {	
		file_seek (owned->file, position);
        }
    }
    		lock_release (&file_lock); 

}
unsigned tell (int fd ){
if(fd<=0 || fd == 1 )
return;
        	lock_acquire (&file_lock); 

 struct list_elem *e;
 struct file_owned *owned;
 		off_t val=-1;
  for (e = list_begin (&thread_current()->files_owned); e != list_end (&thread_current()->files_owned);
       e = list_next (e)) 
    {
      owned = list_entry (e, struct file_owned, elem);
      if (owned->file_descriptor==fd) 
        {	
        	struct file *f=	file_open(owned->file);
		val=file_tell (f); 
		
        }
    }
    		lock_release (&file_lock); 
    		return val;

}
void close (int fd ){
if(fd<=0 || fd == 1 )
return;
	        lock_acquire (&file_lock); 

 struct list_elem *e;
 struct file_owned *owned;
  for (e = list_begin (&thread_current()->files_owned); e != list_end (&thread_current()->files_owned);
       e = list_next (e)) 
    {
      owned= list_entry (e, struct file_owned, elem);
      if (owned->file_descriptor==fd) 
        {	
		file_close (owned->file);
		list_remove(&owned->elem);		
		free(owned);
       }
    }
    		lock_release (&file_lock); 

}
