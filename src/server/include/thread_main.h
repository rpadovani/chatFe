/*****************************************************************
 *  ChatFe - Progetto di Sistemi Operativi '14/'15 UniFe         *
 *                                                               *
 *  Riccardo Padovani (115509) riccardo@rpadovani.com            *
 *****************************************************************/
#ifndef THREAD_MAIN_H
#define THREAD_MAIN_H

/*
    Il thread main si occupa di gestire tutto il server, così come descritto
    nella consegna.

    Essendo una funzione puntata alla creazione di un thread è un puntatore e
    non ritorna nulla, come richiesto da pthread_create().

    L'argomento è un puntatore void
*/
void *thread_main(void *arg);

#endif
