#ifndef GESTIONE_UTENTI_H
#define GESTIONE_UTENTI_H

#include <hash.h>

/*
    La tabella di HASH verrà mantenuta durante tutta la sessione, ma per
    mantenere l'integrità della stessa solo le funzioni presenti in questo
    file potranno accederci, e lo faranno in mutua esclusione
*/
//static hash_t HASH_TABLE;

/*
    La funzione si occupa di aprire il file degli utent, leggerlo,
    caricare gli utenti ivi contenuti nella tabella hash, e poi chiuderlo
*/
void carica_utenti(void);

/*
    La funzione cerca di registrare l'utente passato come argomento, usando
    lo stesso formato dei dati con cui è arrivato al server.
    Dopodiché segna l'utente come loggato usando la socket_id passata come
    argomento.
    Se l'utente è già registrato o per qualche motivo la registrazione fallisce
    ritorna un errore.
 */
char registrazione_utente(char *messaggio, int socket_id);

/*
    La funzione cerca l'username passato come argomento nell'hash table.
    Se l'username è presente e l'utente non è loggato esegue il login, altimenti
    restituisce un errore
 */
char login_utente(char *username, int socket_id);

void elenca_utenti(void);
#endif
