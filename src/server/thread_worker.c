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
    typedef struct {
        char type ;             // message type
        char *sender ;          // message sender
        char *receiver ;        // message receiver
        unsigned int msglen ;   // message length
        char *msg ;             // message buffer
    } msg_t;

    msg_t *messaggio = malloc(sizeof(msg_t));

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

    /*
        Il worker rimane in ascolto fintanto che il server non si ferma o il
        client si disconnette
     */
    while (go) {
        // Puliamo il buffer per leggere il tipo del messaggio
        bzero(buffer, sizeof(char));

        /*
            Per prima cosa leggiamo il tipo di messaggio e salviamolo nella
            struct
         */
        if (read(socket, buffer, sizeof(buffer)) >= 0) {
            // Abbiamo letto il tipo di messaggio, salviamolo
            messaggio->type = buffer[0];
            printf("type : %c\n", messaggio->type );

        } else {
            // todo error
            printf("Errore leggendo dalla socket\n");
        }

        /*
            Riallochiamo il buffer per leggere la dimensione del prossimo campo.
            Oltre alla lunghezza del campo dobbiamo leggere anche lo shebang
            Se non c'è lo shebang ritorniamo un errore
        */
        buffer = realloc(buffer, sizeof(char));
        bzero(buffer, sizeof(char));

        if (read(socket, buffer, sizeof(char)) >= 0) {
            if (buffer[0] == '#') {
                buffer = realloc(buffer, sizeof(int));
                bzero(buffer, sizeof(int));

                // Leggiamo la dimensione della prossima stringa da leggere
                lunghezza_stringa = buffer[0];
                printf("lunghezza = %i\n", lunghezza_stringa);
                buffer = realloc(
                                    buffer,
                                    sizeof(char) * (lunghezza_stringa + 1)
                                );

                if (read(socket, buffer, sizeof(buffer)) >= 0) {
                    strcpy(messaggio->sender, buffer);
                    printf("sender: %s\n", messaggio->sender);
                } else {
                    // todo error
                    printf("Errore leggendo dalla socket\n");
                }
            } else {
                // todo error
                printf("Errore nella formattazione della stringa, scarto...\n");
            }
        } else {
            // todo error
            printf("Errore leggendo dalla socket\n");
        }
    } // end while

    // Prima di uscire chiudiamo la socket
    close(socket);
    pthread_exit(NULL);
}
