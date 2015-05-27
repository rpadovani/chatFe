/*****************************************************************
 *  ChatFe - Progetto di Sistemi Operativi '14/'15 UniFe         *
 *                                                               *
 *  Riccardo Padovani (115509) riccardo@rpadovani.com            *
 *****************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#include <thread_writer.h>

// Tipi di messaggi così come se li aspetta il server
#define MSG_OK 'O'
#define MSG_ERROR 'E'
#define MSG_SINGLE 'S'
#define MSG_BRDCAST 'B'
#define MSG_LIST 'I'
#define MSG_LOGOUT 'X'

// thread_writer.h
void *thread_writer(void *connessione) {
    /*
        Questa struct rappresenta il formato di un messaggio. Verrà usata
        sempre una sola variabile, perché non ho bisogno di mantenere
        disponibile più di un messaggio alla volta, poiché vengono inviati
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
        da stdin
     */
    msg_t *messaggio = malloc(sizeof(struct Message));

    /*
        Il buffer attraverso cui passiamo i messaggi. La prima cosa che leggo
        è il tipo di messaggio, che ha sempre dimensione di un char.
        Dopodiché cambieremo dinamicamente la dimensione del buffer in base
        alle necessità
     */
    char *buffer = malloc(sizeof(char));
    int dimensione_buffer;

    // Riferimento alla socket passato per argometno
    int socket_id = *(int*)connessione;

    // La linea letta da stdin e la sua lunghezza
    char *linea = NULL;
    size_t lunghezza_linea = 0;

    // Leggiamo una linea per volta da stdin
    while (getline(&linea, &lunghezza_linea, stdin) != -1) {
        /*
            Se il sesto elemento della linea è un : (#dest :whatever) abbiamo
            un messaggio di broadcast, altrimenti leggiamo il tipo
            del messaggio
         */
        if (linea[6] == ':') {
            strtok(linea, ":");
            messaggio->msg = strtok(NULL, "\n");
            messaggio->type = MSG_BRDCAST;
        } else {
            messaggio->type = tipo_messaggio(strtok(linea, " "));
        }

        if (messaggio->type == '0') {
          /*
              Il messaggio non è valido, saltiamo alla prossima iterazione.
              Non stampiamo niente perché un messaggio di errore per ogni
              errore di battitura sarebbe snervante
           */
          continue;
        }

        // Ricaviamo i dati che ci interessano
        if (messaggio->type == MSG_SINGLE) {
            messaggio->receiver = strtok(NULL, ":");
            messaggio->msg = strtok(NULL, "\n");
        }

        /*
          Dimensione del messaggio da inviare:
          - 1 byte di tipo di messaggio
          - 4 byte di int che indica la lungezza del receiver
          - n byte di receiver
          - 4 byte di int che indica la lunghezza del messaggio
          - n byte di messaggio
        */
        if (messaggio->type == MSG_SINGLE || messaggio->type == MSG_BRDCAST) {
            if (messaggio->msg == NULL) {
                // Ignoriamo i messaggi vuoti
                continue;
            }

            dimensione_buffer = 1 + 4 + 4 + strlen(messaggio->msg);

            if (messaggio->type == MSG_SINGLE) {
                dimensione_buffer += strlen(messaggio->receiver);
            }

            // Aggiungamo un bit per il terminatore, richiesto da sprintf
            buffer = realloc(buffer, dimensione_buffer + 1);

            /*
                Messaggio singolo e broadcast hanno una struttura leggermente
                diversa
             */
            if (messaggio->type == MSG_SINGLE) {
                sprintf(
                    buffer,
                    "%c%04zu%s%04zu%s",
                    messaggio->type,
                    strlen(messaggio->receiver),
                    messaggio->receiver,
                    strlen(messaggio->msg),
                    messaggio->msg
                );
            } else {
                sprintf(
                    buffer,
                    "%c0000%04zu%s",
                    messaggio->type,
                    strlen(messaggio->msg),
                    messaggio->msg
                );
            }
        } else {
          // Messaggio di logout e ls
          dimensione_buffer = 1 + 4 + 0 + 4 + 0;
          buffer = realloc(buffer, dimensione_buffer + 1);
          sprintf(
              buffer,
              "%c00000000",
              messaggio->type
          );
        }

        /*
            Inviamo il messaggio SENZA il terminatore di stringa
            perché grazie alla dimensione del campo il server
            sa esattamente quanti byte leggere
         */
        if (write(socket_id, buffer, dimensione_buffer) == -1) {
            printf("Impossibile inviare il messaggio\n");
            continue;
        }

        // L'utente si disconnette, fine dei giochi
        if (messaggio->type == MSG_LOGOUT) {
          break;
        }
    }

    free(linea);
    free(buffer);
    free(messaggio);
    pthread_exit(NULL);
}

// thread_writer.h
char tipo_messaggio(char *tipo) {
    if (strcmp(tipo, "#ls\n") == 0) {
        return MSG_LIST;
    } else if (strcmp(tipo, "#logout\n") == 0) {
        return MSG_LOGOUT;
    } else if (strcmp(tipo, "#dest") == 0) {
        return MSG_SINGLE;
    }

    return '0';
}
