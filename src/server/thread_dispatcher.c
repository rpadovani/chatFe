#include <stdio.h>
#include <pthread.h>

#include <thread_dispatcher.h>
#include <main_server.h>

// main_server.h
int go;

void *thread_dispatcher(void *arg)
{
    while (go) {

    }
    pthread_exit(NULL);
}
