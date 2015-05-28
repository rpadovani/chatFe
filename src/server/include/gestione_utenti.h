/*****************************************************************
 *  ChatFe - Progetto di Sistemi Operativi '14/'15 UniFe         *
 *                                                               *
 *  Riccardo Padovani (115509) riccardo@rpadovani.com            *
 *****************************************************************/
#ifndef GESTIONE_UTENTI_H
#define GESTIONE_UTENTI_H

#include <hash.h>

/*
    La funzione si occupa di aprire il file degli utent, leggerlo,
    caricare gli utenti ivi contenuti nella tabella hash, e poi chiuderlo.

    Poichè viene eseguita all'avvio del server non ha bisogno di avere il
    controllo sui mutex perché non c'è concorrenza (ancora)
*/
void carica_utenti(void);

/*
    La funzione cerca l'username passato come argomento nell'hash table.
    Se l'username è presente e l'utente non è loggato esegue il login, altimenti
    restituisce un errore
 */
char login_utente(char *username, int socket_id);

/*
    La funzione cerca di registrare l'utente passato come argomento, usando
    lo stesso formato dei dati con cui è arrivato al server.
    Dopodiché segna l'utente come loggato usando la socket_id passata come
    argomento.
    Se l'utente è già registrato o per qualche motivo la registrazione fallisce
    ritorna un errore.

    Nel parametro username inserisce il nuovo username estraendolo dal messaggio
 */
char registrazione_utente(char *messaggio, int socket_id, char **username);

/*
    La funzione modifica la stringa passata come argomento con l'elenco di
    tutti gli utenti connessi, cioè di tutti gli utenti che nella hash table
    hanno un socket id diverso da -1.

    Il separatore tra i vari nomi utenti sarà :
 */
void elenca_utenti_connessi(char *risposta);

/*
    La funzione setta il socket_id dell'utente nella HASH_TABLE a -1 e elimina
    l'utente dalla lista degli utenti connessi
 */
void logout_utente(char *username);

/*
    La funzione ritorna -1 se l'utente passato come argomento non esiste o 0
    se esiste
 */
int esiste_utente(char *username);

/*
    Dato uno username, la funzione ritorna il sockid corrispondente.
    Non dà informazioni sull'esistenza o meno dell'utente, perché ritorna
    -1 sia per gli utenti disconnessi (perché quello è effettivamente il loro
    sockid) che per gli utenti non presenti nella hash table.

    Per verificare se un utente esiste utilizzare esiste_utente()
 */
int sockid_username(char *username);

/*
    La funzione deve essere evocata quando il server viene spento: si occupa
    di copiare tutta la hash table nel file passato a linea di comando all'avvio
    del server.

    Potrebbe soffrire di race conditions, quindi impostiamo il mutex
 */
void salva_hashtable(void);
#endif
