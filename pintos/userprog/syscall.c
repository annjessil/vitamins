#include "userprog/syscall.h"

#include <stdio.h>
#include <syscall-nr.h>

#include "threads/interrupt.h"
#include "threads/thread.h"

static void syscall_handler(struct intr_frame *);

void syscall_init(void) {
    intr_register_int(0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void syscall_handler(struct intr_frame *f UNUSED) {
    uint32_t *args = ((uint32_t *) f->esp);

    /*
     * The following print statement, if uncommented, will print out the syscall
     * number whenever a process enters a system call. You might find it useful
     * when debugging. It will cause tests to fail, however, so you should not
     * include it in your final submission.
     */

    /* printf("System call number: %d\n", args[0]); */

    if (args[0] == SYS_EXIT) {
        f->eax = args[1];
        printf("%s: exit(%d)\n", thread_current()->name, args[1]);
        thread_exit();
    } else if (args[0] ==  SYS_INCREMENT){
        int val = args[1];
        f->eax = val + 1; 
    } else if (args[0] == SYS_WRITE){
        int fd = args[1];
        const void *buffer = args[2];
        unsigned size = args[3];

        




        /*System Call: int write(int fd, const void *buffer, unsigned size) This system call writes size bytes
from buffer to the open file corresponding to fd. It returns the number of bytes actually written,
which may be less than size.
Standard output corresponds to the file descriptor 1. For this assignment, your implementation of the write
system call only needs to work for file descriptor 1, not any other file descriptors. As before, you do not need
to validate system call arguments or their addresses for this assignment.
Writes to standard output (file descriptor 1) should be output via the console. In the Pintos kernel,
you can accomplish this by calling the putbuf function declared in lib/kernel/stdio.h and defined in
lib/kernel/console.c.
The interface provided by the write system call allows it to process fewer bytes than it is provided (i.e., return
short), since the number of bytes that it actually processed is returned to the caller. For standard output
(file descriptor 1), your implementation should always process all of the bytes on a successful
call to write. Additionally, these bytes should be processed via a single call to putbuf.
The write-stdout test checks that the write system call is implemented properly for standard output (file
descriptor 1). You should read the source code for this test (by now, you should know where to find it).
Once you complete this part of the assignment, the write-stdout test should pass.*/
    }
}
