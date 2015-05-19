#ifndef THREAD_DISPATCHER
#define THREAD_DISPATCHER

/*
    Il thread dispatcher svolge il compito di consumatore del buffer
    circolare.

    Fintanto che la variabile globale go è settata, continua ad
    estrarre messaggi dal buffer e scriverli sul socketid corrispondente
    allo username letto
 */
void *thread_dispatcher(void *arg);

/*
    La funzione inserisci prende in input un array di char che funge da
    messaggio, un char come tipo di messaggio e la lunghezza del messaggio.

    Se il messaggio è di tipo broadcast, l'array del messaggio conterrà
    solo il messaggio.

    Se invece il messaggio è di tipo singolo, il formato del messaggio
    sarà

    %i%s%s

    dove la %i è la lunghezza del nome utente, che è la prima stringa
    inserita di seguito, a cui segue il messaggio vero e proprio.

    Ci penserà il thread_dispatcher a separare lo username dal messaggio
 */
void inserisci (char *messaggio, char tipo_messaggio, int sockid_destinatario);

/*
    La funzione estrai prende gli elementi dal buffer circolare e li scrive
    nelle tre variabili passate come argomento.
 */
void estrai (char **messaggio, char *tipo_messaggio, int *sockid_destinatario);

#endif
