#ifndef TAU_SYS_H
#define TAU_SYS_H

#include <pthread.h>
#include <semaphore.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <sys/time.h>

#ifdef __MACH__
    #include <sys/filio.h>
    #include "sys/event.h"
#else
    #include <sys/wait.h> 
    #include <sys/fcntl.h>
    #include <sys/epoll.h>
    #include <sys/timerfd.h>
    #include <sys/eventfd.h>
#endif

#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#include <dlfcn.h>
#include <arpa/inet.h>
#include <execinfo.h>
#include <unistd.h> 
#include <errno.h> 





#endif