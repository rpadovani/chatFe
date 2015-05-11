#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include <thread_listener.h>

void *thread_listener(void *connessione) {
    /*
        Questa struct rappresenta il formato di un messaggio. Verrà usata
        sempre una sola variabile, perché non ho bisogno di mantenere
        disponibile più di un messaggio alla volta, poiché vengono stampati
    */
    typedef struct Message {
        char type;             // message type
        char *sender;          // message sender
        char *receiver;        // message receiver
        unsigned int msglen;   // message length
        char *msg;             // message buffer
    } msg_t;

    /*
        Riferimento alla struttura per salvare i dati che vengono passati
        dalla socket
     */
    msg_t *messaggio = malloc(sizeof(struct Message));

    /*
        Il buffer attraverso cui passiamo i messaggi. La prima cosa che leggo
        è il tipo di messaggio, che ha sempre dimensione di un char.
        Dopodiché cambieremo dinamicamente la dimensione del buffer in base
        alle necessità
     */
    char *buffer = malloc(sizeof(char));

    // Riferimento alla socket passato per argometno
    int socket_id = *(int*)connessione;

    while (read(socket_id, buffer, sizeof(char)) > 0) {
        // Leggiamo il primo carattere che rappresenta il tipo di messaggio
        messaggio->type = buffer[0];
    }

    pthread_exit(NULL);
}
