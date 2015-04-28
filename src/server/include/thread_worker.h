#ifndef THREAD_WORKER_H
#define THREAD_WORKER_H

/*
    Ogni connessione in entrata crea un thread worker, che si occupa di gestire
    la comunicazione tra server e client

    Essendo una funzione puntata alla creazione di un thread è un puntatore e
    non ritorna nulla, come richiesto da pthread_create.

    In input riceve una socket funzionante
*/
void *thread_worker(void *arg);

#endif
