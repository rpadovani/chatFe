/*****************************************************************
 *  ChatFe - Progetto di Sistemi Operativi '14/'15 UniFe         *
 *                                                               *
 *  Riccardo Padovani (115509) riccardo@rpadovani.com            *
 *****************************************************************/
#ifndef MAIN_SERVER_H
#define MAIN_SERVER_H

#include <signal.h>

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

    Viene chiamata senza SA_RESTART per sbloccare eventuali funzioni che sono
    bloccate nel main.

    Per le chiamate bloccanti degli altri thread simula delle finte chiamate:
    una connect() per sbloccare l'accept del thread main, un messaggio (che
    serve anche per notificare lo spegnimento del server) al thread dispatcher.

    Un altro modo per farlo sarebbe di passare con pthread_kill il segnale ad
    ogni thread, e installare dei signal_handler senza SA_RESTART per sbloccare
    le funzioni. Per quanto più elegante, richiederebbe di mantenere una lista
    con l'id di tutti i thread worker creati per ogni client connesso.

    Ovviamente la cosa non scalerebbe bene, quindi ho preferito come soluzione
    le chiamate simulate
 */
void signal_handler(int signal_number);

#endif
