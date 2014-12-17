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
if(!is_user_vaddr (ptr))
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
  //halt();
  break;
  case SYS_EXIT:
  //exit(&f->esp+4);
  break;
  case SYS_EXEC:  
  //value=exec(&f->esp+4);
  f->eax=value;	
  break;
  case SYS_WAIT:
  //value=wait(&f->esp+4);
  f->eax=value;
  break;
  case     SYS_CREATE:
  val=create(*((char *)valid_refrence(f->esp+1)),*((int *)valid_refrence(f->esp+2)));
  f->eax=val;
  break;
  case     SYS_REMOVE:
  val=remove(*((char *)valid_refrence(f->esp+1)));
  f->eax=val;
  break;
  case    SYS_OPEN:
  value=open(*((int *)valid_refrence(f->esp+1)));
  f->eax=value;  
  break;
  case SYS_FILESIZE:
  value=filesize(*((int *)valid_refrence(f->esp+1)));  
    f->eax=value;  
  break;
  case SYS_READ:
  value=read(*((int *)valid_refrence(f->esp+1)),*((char *)valid_refrence(f->esp+2)),*((int *)valid_refrence(f->esp+3)));  
    f->eax=value;  
  break;  
  case SYS_WRITE:
  value=write(*((int *)valid_refrence(f->esp+1)),*((char *)valid_refrence(f->esp+2)),*((int *)valid_refrence(f->esp+3)));  
    f->eax=value;  
  break;
  case SYS_SEEK:
  seek(*((int *)valid_refrence(f->esp+1)),*((int *)valid_refrence(f->esp+2)));  
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
  for (e = list_begin (&thread_current()->files_owned); e != list_end (&thread_current()->files_owned);
       e = list_next (e)) 
    {
      owned = list_entry (e, struct file_owned, elem);
      if (owned->file_descriptor==fd) 
        {
	off_t length=file_length(owned->file);
	lock_release (&file_lock); 
	return length;
        }
    }
    lock_release (&file_lock); 
return 0;
}
int read (int fd , void * buffer , unsigned size ){
if(fd<0 || fd == 1 )
return;
if(size<=0)
return 0;

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
 off_t read;
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
return;
if(size<=0)
return 0;
if(fd==1){
int temp_size=size/400;
if(temp_size==0)
putbuf(*((char *)buffer),size);
else{
int unit=size/400;
int count=1;
while(temp_size>0){
putbuf(*((char *)buffer+count*unit),unit);
temp_size--;
count++;
}
}
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
 		off_t val;
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
		list_remove(e);		
       }
    }
    		lock_release (&file_lock); 

}
