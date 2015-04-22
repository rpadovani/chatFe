#include <stdio.h>

#include <thread_main.h>
#include <gestione_utenti.h>

/*
    Per la descrizione delle funzioni presenti in questo file consultare i
    rispettivi header, indicati da un commento all'inizio della funzione
    stessa
*/

// thread_main.h
void thread_main(char *file_utenti, char *file_log)
{
    // Per prima cosa ci occupiamo di caricare gli utenti nella tabella hash
    // gestione_utenti.h
    carica_utenti(file_utenti);
}
