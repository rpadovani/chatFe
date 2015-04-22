#ifndef GESTIONE_UTENTI_H
#define GESTIONE_UTENTI_H

#include <hash.h>

static hash_t HASH_TABLE;

/*
    La funzione si occupa di aprire il file passato come argomento, leggerlo,
    caricare gli utenti ivi contenuti nella tabella hash, e poi chiuderlo
*/
void carica_utenti(char *file_utenti);
#endif
