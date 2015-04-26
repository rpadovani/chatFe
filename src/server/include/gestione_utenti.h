#ifndef GESTIONE_UTENTI_H
#define GESTIONE_UTENTI_H

#include <hash.h>

/*
    La tabella di HASH verrà mantenuta durante tutta la sessione, ma per
    mantenere l'integrità della stessa solo le funzioni presenti in questo
    file potranno accederci, e lo faranno in mutua esclusione
*/
static hash_t HASH_TABLE;

/*
    La funzione si occupa di aprire il file degli utent, leggerlo,
    caricare gli utenti ivi contenuti nella tabella hash, e poi chiuderlo
*/
void carica_utenti(void);
#endif
