/*****************************************************************
 *  ChatFe - Progetto di Sistemi Operativi '14/'15 UniFe         *
 *                                                               *
 *  Riccardo Padovani (115509) riccardo@rpadovani.com            *
 *****************************************************************/
#ifndef THREAD_DISPATCHER
#define THREAD_DISPATCHER

#include <pthread.h>

// Dimensione del dispatcher
// XXX: se viene aggiornato qua, aggiornarlo anche in thread_dispatcher.c
#define K 256

/*
    Il thread dispatcher svolge il compito di consumatore del buffer
    circolare.

    Fintanto che la variabile globale go è settata, continua ad
    estrarre messaggi dal buffer e scriverli sul socketid corrispondente
    allo username letto
 */
void *thread_dispatcher(void *arg);

/*
    La funzione inserisci prende in input:

    * un array di char - il messaggio - che sarà poi processato dal dispatcher e
      inviato così com'è al client destinatario
    * un char che indica il tipo di messaggio, singolo o broadcast
    * il socketid del destinatario. Se il messaggio è di tipo broadcast, questo
      parametro verrà poi ignorato
 */
void inserisci(char *messaggio, char tipo_messaggio, int sockid_destinatario);

/*
    La funzione estrai prende gli elementi dal buffer circolare e li scrive
    nelle tre variabili passate come argomento.
 */
void estrai(char **messaggio, char *tipo_messaggio, int *sockid_destinatario);

/*
    Il grande protagonista: il buffer circolare in tutta la sua bellezza.
    È una struct con alcune variabili di controllo e tre array di dimensione K
    al cui interno ci sono, rispettivamente, i messaggi, il tipo di messaggi
    (singoli o broadcast) e il socket id del destinatario
*/
typedef struct {
    // Array dei contenuti
    char *messaggio[K];
    char tipo_messaggio[K];
    int sockid_destinatario[K];

    // Mutex per l'accesso separato
    pthread_mutex_t mutex;

    // Posizione del cursore in lettura e in scrittura
    int readpos, writepos;
    // Numero di elementi
    int cont;

    // Condizioni del buffer, se pieno o vuoto
    pthread_cond_t PIENO;
    pthread_cond_t VUOTO;
} buffer_circolare;

#endif
