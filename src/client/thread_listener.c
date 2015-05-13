#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#include <thread_listener.h>

#define MSG_SINGLE 'S'
#define MSG_BRDCAST 'B'
#define MSG_LIST 'I'
#define MSG_LOGOUT 'X'

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

    // Lunghezza del prossimo campo da leggere dal socket
    int lunghezza_stringa;

    int i;

    while (read(socket_id, buffer, sizeof(char)) > 0) {
        // Leggiamo il primo carattere che rappresenta il tipo di messaggio
        messaggio->type = buffer[0];

        /*
            Dopo il primo carattere abbiamo una serie di 2 blocchi formati
            nello stesso modo.

            Prima 4 byte di int, poi n byte di messaggio, dove n è il numero
            indicato nei primi 4 byte
         */
        for (i = 0; i < 2; i++) {
            // Il buffer deve essere pronto a leggere un'int
            buffer = realloc(buffer, sizeof(int));
            // Leggiamo la lunghezza della stringa
            if (read(socket_id, buffer, sizeof(int)) != sizeof(int)) {
               printf("WOPS 1 %s\n", buffer);
            }
            /*
                A questo punto sappiamo la lunghezza del prossimo campo da
                leggere.
                Convertiamo la dimensione letta in un'int, e allochiamo lo
                spazio di cui abbiamo bisogno
             */
            lunghezza_stringa = atoi(buffer) * sizeof(char);

            /*
                Leggiamo il nostro prossimo campo, di cui sappiamo la
                lunghezza
                Se non riusciamo a leggere tutto il campo, generiamo un
                errore
             */
            if (lunghezza_stringa > 0) {
                buffer = realloc(buffer, lunghezza_stringa);
                if (read(socket_id, buffer,
                    lunghezza_stringa) != lunghezza_stringa) {
                    printf("WOPS 2\n");
                }

                /*
                    Abbiamo 2 campi, al primo giro leggiamo il mittente
                    poi il messaggio stesso.
                    Per ogni messaggio che manda il server il destinatario
                    è vuoto.
                    In alcuni casi anche il mittente potrebbe essere
                    vuoto.

                    Ad ognuno aggiungiamo il terminatore di stringa
                 */
                switch (i) {
                    case 0:
                        messaggio->sender = malloc(lunghezza_stringa + 1);
                        strcpy(messaggio->sender, buffer);
                        messaggio->sender[lunghezza_stringa] = '\0';
                        break;
                    case 1:
                        messaggio->msg = malloc(lunghezza_stringa + 1);
                        strcpy(messaggio->msg, buffer);
                        messaggio->msg[lunghezza_stringa] = '\0';
                        break;
                    default:
                        printf("Questo messaggio non sarà mai stampato\n");
                }
            }
        }

        if (messaggio->type == MSG_LIST) {
            printf("%s\n", messaggio->msg);
        }

        bzero(buffer, 5);
    }

    pthread_exit(NULL);
}
