#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#include <thread_writer.h>

#define MSG_OK 'O'
#define MSG_ERROR 'E'
#define MSG_SINGLE 'S'
#define MSG_BRDCAST 'B'
#define MSG_LIST 'I'
#define MSG_LOGOUT 'X'

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

    char *linea = NULL;
    size_t lunghezza_linea = 0;

    while (getline(&linea, &lunghezza_linea, stdin) != -1) {
        messaggio->type = tipo_messaggio(strtok(linea, " "));

        if (messaggio->type == '0') {
          // Il messaggio non e valido, saltiamo alla prossima iterazione
          // TODO gestione errori
          continue;
        }

        if (messaggio->type == MSG_SINGLE) {
            messaggio->receiver = strtok(NULL, ":");
            messaggio->msg = strtok(NULL, "\n");

            /*
              Se il messaggio e nullo e perche non c'e receiver (essendo letto
              prima e non trovando il delimiter strtok restituisce tutta la
              stringa), quindi abbiamo un messaggio broadcast
             */
            if (messaggio->msg == NULL) {
              messaggio->msg = strtok(messaggio->receiver, "\n");
              messaggio->type = MSG_BRDCAST;
            }
        }

        /*
          Dimensione del messaggio da inviare:
          - 1 byte di tipo di messaggio
          - 4 byte di int che indica la lungezza del receiver
          - n byte di receiver
          - 4 byte di int che indica la lunghezza del messagggio
          - n byte di messaggio
        */
        if (messaggio->type == MSG_SINGLE || messaggio->type == MSG_BRDCAST) {
            dimensione_buffer = 1 + 4 + 4 + strlen(messaggio->msg);

            if (messaggio->type == MSG_SINGLE) {
                dimensione_buffer += strlen(messaggio->receiver);
            }

            buffer = realloc(buffer, dimensione_buffer + 1);

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

        if (messaggio->type == MSG_LOGOUT) {
          break;
        }
    }

    free(linea);
    free(buffer);
    free(messaggio);
    pthread_exit(NULL);
}

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
