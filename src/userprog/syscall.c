#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "filesys/file.h"
#include "filesys/inode.h"
#include "threads/malloc.h"
#include "threads/synch.h"
#include <list.h>

static struct list all_inodes;
static struct list open_inodes;
static struct lock file_lock;

struct inode_name{
    struct inode *inode;        /* File's inode. */
    const char * file;
    struct list_elem elem;              /* Element in inode list. */   
    int file_descriptor;
    	
};
static void syscall_handler (struct intr_frame *);
bool create (const char * file , unsigned initial_size );
bool remove (const char * file );
int open (const char * file );
int filesize (int fd );
int read (int fd , void * buffer , unsigned size );
int write (int fd , const void * buffer , unsigned size );
void seek (int fd , unsigned position );
unsigned tell (int fd );
void close (int fd );
int file_descriptor_counter=1;
void
syscall_init (void) 
{
    intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
    list_init(&all_inodes);
    list_init(&open_inodes);
    lock_init(&file_lock);
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  printf ("system call!\n");
//  thread_exit ();
  
  int call=&(f->esp);
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
  val=create(&f->esp+4,&f->esp+8);
  f->eax=val;
  break;
  case     SYS_REMOVE:
  val=remove(&f->esp+4);
  f->eax=val;
  break;
  case    SYS_OPEN:
  value=open(&f->esp+4);
  f->eax=value;  
  break;
  case SYS_FILESIZE:
  value=filesize(&f->esp+4);  
    f->eax=value;  
  break;
  case SYS_READ:
  value=read(&f->esp+4,&f->esp+8,&f->esp+12);  
    f->eax=value;  
  break;  
  case SYS_WRITE:
  value=write(&f->esp+4,&f->esp+8,&f->esp+12);  
    f->eax=value;  
  break;
  case SYS_SEEK:
  seek(&f->esp+4,&f->esp+8);  
  break;
  case SYS_TELL:
  value=tell(&f->esp+4);
  f->eax=value;
  break;
  case SYS_CLOSE:
  close(&f->esp+4);
  break;
  }
}










bool create (const char * file , unsigned initial_size ){
block_sector_t sector;
lock_acquire(&file_lock);
bool res=inode_create(sector,initial_size);
if(!res)
return false;
struct inode * inode=inode_open(sector);
lock_release (&file_lock); 
if(inode==NULL)
return false;
struct inode_name *temp=NULL;
temp=calloc(1,sizeof *temp);
temp->file=file;
temp->inode=inode;
list_push_back (&all_inodes, &temp->elem);
lock_acquire(&file_lock);
inode_close(inode);
lock_release (&file_lock); 
return res;
}
bool remove (const char * file ){
 struct list_elem *e;
 struct inode_name *inode_name;
  for (e = list_begin (&all_inodes); e != list_end (&all_inodes);
       e = list_next (e)) 
    {
      inode_name = list_entry (e, struct inode_name, elem);
      if (strcmp(file,inode_name->file)==0) 
        {
                lock_acquire(&file_lock);
		inode_remove(inode_name->inode);
		lock_release (&file_lock); 
		list_remove(e);
		return true;
        }
    }
 return false;
}
int open (const char * file ){

 struct list_elem *e;
 struct inode_name *inode_name;
  for (e = list_begin (&all_inodes); e != list_end (&all_inodes);
       e = list_next (e)) 
    {
      inode_name = list_entry (e, struct inode_name, elem);
      if (strcmp(file,inode_name->file)==0) 
        {       lock_acquire (&file_lock); 
		struct file * file=file_open(inode_name->inode);
		lock_release (&file_lock); 
		if(file==NULL)
			return -1;
		else{
		file_descriptor_counter++;
		inode_name->file_descriptor=file_descriptor_counter;		
		list_push_back (&open_inodes, &inode_name->elem);
		return inode_name->file_descriptor;
		}
        }
    }
return -1;
}
int filesize (int fd ){
if(fd<=0 || fd == 1 )
return;

 struct list_elem *e;
 struct inode_name *inode_name;
  for (e = list_begin (&open_inodes); e != list_end (&open_inodes);
       e = list_next (e)) 
    {
      inode_name = list_entry (e, struct inode_name, elem);
      if (inode_name->file_descriptor==fd) 
        {
        lock_acquire (&file_lock); 
		
	off_t length=inode_length(inode_name);
	lock_release (&file_lock); 
		return length;
        }
    }
return 0;
}
int read (int fd , void * buffer , unsigned size ){
if(fd<0 || fd == 1 )
return;
if(size<=0)
return 0;

if(fd==0)
{
buffer=input_getc();
return 1;
}
else{
 struct list_elem *e;
 struct inode_name *inode_name;
 bool found=false;
 off_t read;
  for (e = list_begin (&open_inodes); e != list_end (&open_inodes);
       e = list_next (e)) 
    {
      inode_name = list_entry (e, struct inode_name, elem);
      if (inode_name->file_descriptor==fd) 
        {
	found=true;
        lock_acquire (&file_lock); 
	struct file *f=	file_open(inode_name);
	read=file_read (f, buffer, size); 
	lock_release (&file_lock); 
        }
    }
	if(found)
	return read;
	else{
		return -1;
	     }
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
putbuf(buffer,size);
else{
int unit=size/400;
int count=1;
while(temp_size>0){
putbuf(buffer+count*unit,unit);
temp_size--;
count++;
}
}
return size;
}
else{
 struct list_elem *e;
 struct inode_name *inode_name;
 off_t written=0;
  for (e = list_begin (&open_inodes); e != list_end (&open_inodes);
       e = list_next (e)) 
    {
      inode_name = list_entry (e, struct inode_name, elem);
      if (inode_name->file_descriptor==fd) 
        {
        lock_acquire (&file_lock); 
	struct file *f=	file_open(inode_name);
	written=file_write (f, buffer, size); 
	lock_release (&file_lock); 
        }
    }
return written;
}
}
void seek (int fd , unsigned position ){
if(fd<=0 || fd == 1 )
return;

 struct list_elem *e;
 struct inode_name *inode_name;
  for (e = list_begin (&open_inodes); e != list_end (&open_inodes);
       e = list_next (e)) 
    {
      inode_name = list_entry (e, struct inode_name, elem);
      if (inode_name->file_descriptor==fd) 
        {	
        	lock_acquire (&file_lock); 
        	struct file *f=	file_open(inode_name);
		file_seek (f, position);
		lock_release (&file_lock); 
        }
    }
}
unsigned tell (int fd ){
if(fd<=0 || fd == 1 )
return;

 struct list_elem *e;
 struct inode_name *inode_name;
  for (e = list_begin (&open_inodes); e != list_end (&open_inodes);
       e = list_next (e)) 
    {
      inode_name = list_entry (e, struct inode_name, elem);
      if (inode_name->file_descriptor==fd) 
        {	
        	lock_acquire (&file_lock); 
        	struct file *f=	file_open(inode_name);
		off_t val=file_tell (f); 
		lock_release (&file_lock); 
		return val;
        }
    }
}
void close (int fd ){
if(fd<=0 || fd == 1 )
return;
 struct list_elem *e;
 struct inode_name *inode_name;
  for (e = list_begin (&open_inodes); e != list_end (&open_inodes);
       e = list_next (e)) 
    {
      inode_name = list_entry (e, struct inode_name, elem);
      if (inode_name->file_descriptor==fd) 
        {	
	        lock_acquire (&file_lock); 
        	struct file *f=	file_open(inode_name);
		file_close (f);
		lock_release (&file_lock); 
		list_remove(e);		
		return;
        }
    }
}
