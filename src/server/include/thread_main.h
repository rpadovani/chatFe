#ifndef THREAD_MAIN_H
#define THREAD_MAIN_H

/*
    Il thread main si occupa di gestire tutto il server, così come descritto
    nella consegna.

    Essendo una funzione puntata alla creazione di un thread è un puntatore e
    non ritorna nulla, come richiesto da pthread_create
*/
void *thread_main(void *arg);

#endif
