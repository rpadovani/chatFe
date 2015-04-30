#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>

#include <thread_worker.h>

// main_server.h
int go;

// thread_worker.h
void *thread_worker(void *connessione)
{
    /*
        Questa struct rappresenta il formato di un messaggio. Verrà usata
        sempre una sola variabile, perché non ho bisogno di mantenere
        disponibile più di un messaggio alla volta, poiché vengono loggati
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
    int socket = *(int*)connessione;

    /*
        Salveremo man mano qua la lunghezza della successiva stringa da leggere
        dal buffer, in modo da allocare la dimensione della stringa apposita
     */
    int lunghezza_stringa;

    // Contatore usato da 0 a 3 per evitare di ripetere il codice
    int i;

    /*
        Il worker rimane in ascolto fintanto che il server non si ferma o il
        client si disconnette
     */
    while (go) {
        // Creiamo lo spazio e leggiamo il primo carattere
        buffer = realloc(buffer, sizeof(char));
        if (read(socket, buffer, sizeof(char)) == 1) {
            // Abbiamo letto il tipo di messaggio, salviamolo
            messaggio->type = buffer[0];
        } else {
            // todo error
            printf("Errore leggendo dalla socket\n");
        }

        /*
            Dopo il primo carattere abbiamo una serie di 3 blocchi formati
            nello stesso modo.

            Prima 4 byte di int, poi n byte di messaggio, dove n è il numero
            indicato nei primi 4 byte
         */
        for (i = 0; i < 3; i++) {
            // Il buffer deve essere pronto a leggere un'int
            buffer = realloc(buffer, sizeof(int));
            // Leggiamo la lunghezza della stringa
            if (read(socket, buffer, sizeof(int)) != sizeof(int)) {
                printf("WOPS 1\n");
            }

            /*
                A questo punto sappiamo la lunghezza del prossimo campo da
                leggere.
                Convertiamo la dimensione letta in un'int, e allochiamo lo
                spazio di cui abbiamo bisogno
             */
            lunghezza_stringa = atoi(buffer) * sizeof(char);

            /*
                Leggiamo il nostro prossimo campo, di cui sappiamo la lunghezza
                Se non riusciamo a leggere tutto il campo, generiamo un errore
             */
            buffer = realloc(buffer, lunghezza_stringa);
            if (read(socket, buffer, lunghezza_stringa) != lunghezza_stringa) {
                printf("WOPS 2\n");
            }

            /*
                Abbiamo 3 campi, al primo giro leggiamo il mittente, poi il
                destinatario, poi il messaggio stesso.

                Ad ognuno aggiungiamo il terminatore di stringa
             */
            switch (i) {
                case 0:
                    messaggio->sender = malloc(lunghezza_stringa + 1);
                    strcpy(messaggio->sender, buffer);
                    messaggio->sender[lunghezza_stringa] = '\0';
                    break;
                case 1:
                    messaggio->receiver = malloc(lunghezza_stringa + 1);
                    strcpy(messaggio->receiver, buffer);
                    messaggio->receiver[lunghezza_stringa] = '\0';
                    break;
                case 2:
                    messaggio->msg = malloc(lunghezza_stringa + 1);
                    strcpy(messaggio->msg, buffer);
                    messaggio->msg[lunghezza_stringa] = '\0';
                    break;
                default:
                    printf("Questo messaggio non sarà mai stampato\n");
            }
        }

        printf("tipo: %c\n", messaggio->type);
        printf("mittente: %s\n", messaggio->sender);
        printf("destinatario: %s\n", messaggio->receiver);
        printf("messaggio: %s\n", messaggio->msg);
    } // end while

    // Prima di uscire chiudiamo la socket
    close(socket);
    pthread_exit(NULL);
}
