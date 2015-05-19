#ifndef MAIN_SERVER_H
#define MAIN_SERVER_H

#include <signal.h>

#define K 256

/*
    La variabile globale go controlla il flusso di esecuzione del programma.
    Fintanto che è diversa da zero, le varie operazioni continuano.
    Per gestire al meglio l'interruzione del segnale bisogna dichiararla
    sig_atomic_t, perché l'interruzione è asincrona
 */
extern sig_atomic_t go;

/*
    Le due variabili seguenti mantengono il riferimo al nome dei file degli
    utenti e del file di log
 */
extern char *file_log;
extern char *file_utenti;

/*
    La funzione signal_handler si occupa di gestire i segnali che arrivano
    per stoppare il server.

    Essendo stata invocata con sigaction() rimuove SA_RESTART, in questo modo
    gli accept() e i read() ritornano -1 e sbloccano il while in cui sono
    inseriti
 */
void signal_handler(int signal_number);

#endif
