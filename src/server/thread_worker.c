#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>

#include <thread_worker.h>
#include <thread_dispatcher.h>
#include <gestione_utenti.h>
#include <log.h>

// main_server.h
int go;

#define MSG_OK 'O'
#define MSG_ERROR 'E'
#define MSG_SINGLE 'S'
#define MSG_BRDCAST 'B'
#define MSG_LIST 'I'
#define MSG_LOGOUT 'X'

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
        alle necessità.
        Serve sia per ricevere che per inviare.
     */
    char *buffer = malloc(sizeof(char));

    /*
        Array di char che conterrà la risposta da inviare al client, contenente
        eventuali messaggi di errore se l'operazione è fallita, oppure la
        risposta in caso di successo
     */
    char *risposta = malloc(sizeof(char));

    // Riferimento alla socket passato per argometno
    int socket_id = *(int*)connessione;

    // Il nome dell'utente che sta utilizzando questo worker
    char *username = malloc(sizeof(char));

    /*
        Salveremo man mano qua la lunghezza della successiva stringa da leggere
        dal buffer, in modo da allocare la dimensione della stringa apposita.

        Nel momento dell'invio poi lo useremo per inserire la lunghezza del
        buffer da inviare
     */
    int lunghezza_stringa;

    // Contatore usato da 0 a 2 per evitare di ripetere il codice
    int i;

    /*
        Il worker rimane in ascolto fintanto che il server non si ferma o il
        client si disconnette
     */
    while (go && read(socket_id, buffer, sizeof(char)) > 0) {
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
            bzero(buffer, sizeof(int));

            /*
                Leggiamo il nostro prossimo campo, di cui sappiamo la lunghezza
                Se non riusciamo a leggere tutto il campo, generiamo un errore
             */
            if (lunghezza_stringa > 0) {
                buffer = realloc(buffer, lunghezza_stringa + 1);
                buffer[lunghezza_stringa] = '\0';
                if (read(socket_id, buffer, lunghezza_stringa) != lunghezza_stringa) {
                    printf("WOPS 2\n");
                }

                /*
                    Abbiamo 2 campi, al primo giro leggiamo il destinatario,
                    poi il messaggio stesso.

                    Per ogni messaggio che manda il client il sender è vuoto.
                    In alcuni casi anche il destinatario potrebbe essere
                    vuoto.

                    Ad ognuno aggiungiamo il terminatore di stringa
                 */
                switch (i) {
                    case 0:
                        messaggio->receiver = strdup(buffer);
                        bzero(buffer, strlen(buffer));
                        break;
                    case 1:
                        messaggio->msg = strdup(buffer);
                        bzero(buffer, strlen(buffer));
                        break;
                    default:
                        printf("Questo messaggio non sarà mai stampato\n");
                }
            }
        }

        if (messaggio->type == 'R') {
            // gestore_utenti.h
            buffer[0] = registrazione_utente(messaggio->msg, socket_id, &username);
            write(socket_id, buffer, 1);
        } else if (messaggio->type == 'L') {
            // gestore_utenti.h
            username = malloc(sizeof(strlen(messaggio->msg)) + 1);
            strcpy(username, messaggio->msg);
            buffer[0] = login_utente(messaggio->msg, socket_id);
            write(socket_id, buffer, 1);
        } else if (messaggio->type == MSG_LIST) {
            // gestore_utenti.h
            elenca_utenti_connessi(risposta);
            /*
                Prepariamo la risposta con questa struttura:
                - 4 byte di int che indica la lunghezza della risposta
                - n byte di risposta
                - 1 byte di terminatore di stringa
             */
            lunghezza_stringa = 4 + strlen(risposta) + 1;

            buffer = realloc(buffer, lunghezza_stringa);

            sprintf(
                buffer,
                "%04zu%s",
                strlen(risposta) + 1,
                risposta
            );
            buffer[lunghezza_stringa] = '\0';
            write(socket_id, buffer, lunghezza_stringa);
        } else if (messaggio->type == MSG_LOGOUT) {
          // gestione_utenti.h
          logout_utente(username);
          // Usciamo dal ciclo
          break;
        } else if (messaggio->type == MSG_BRDCAST){
            // thread_dispatcher.h
            // thread_dispatcher.h
            /*
                La lunghezza della stringa tiene conto di:
                - n bytes di mittente
                - 1 byte di :
                - 1 byte di destinatario
                - 1 byte di :
                - n bytes di messaggio

                Il messaggio contiene già il terminatore di stringa
             */
            lunghezza_stringa = strlen(username) + 1 + 1 + 1 + strlen(messaggio->msg);

            // Nella stringa ci deve essere spazio anche per i 4 bytes di int
            char *stringa_supporto = malloc(lunghezza_stringa + 4);

            sprintf(
                stringa_supporto,
                "%04i%s:*:%s",
                lunghezza_stringa,
                username,
                messaggio->msg
            );

            log_messaggio_broadcast(username, messaggio->msg);

            inserisci(stringa_supporto, MSG_BRDCAST, -1);
            free(stringa_supporto);
        } else if (messaggio->type == MSG_SINGLE) {
            // gestione_utenti.h
            if (esiste_utente(messaggio->receiver) == 1) {
                // Se l'utente non esiste passiamo al ciclo succesivo
                continue;
            }

            // thread_dispatcher.h
            /*
                La lunghezza della stringa tiene conto di:
                - n bytes di mittente
                - 1 byte di :
                - n bytes di destinatario
                - 1 byte di :
                - n bytes di messaggio

                Il messaggio contiene già il terminatore di stringa
             */
            lunghezza_stringa = strlen(username) + 1 +
            strlen(messaggio->receiver) + 1 + strlen(messaggio->msg);

            // Nella stringa ci deve essere spazio anche per i 4 bytes di int
            char *stringa_supporto = malloc(lunghezza_stringa + 4);

            sprintf(
                stringa_supporto,
                "%04i%s:%s:%s",
                lunghezza_stringa,
                username,
                messaggio->receiver,
                messaggio->msg
            );

            log_messaggio_singolo(username, messaggio->receiver, messaggio->msg);

            inserisci(
                stringa_supporto,
                MSG_SINGLE,
                sockid_username(messaggio->receiver)
            );
            free(stringa_supporto);
        } else {
            // TODO error
            printf("->%c<- \n", messaggio->type);
            printf("Cella in avaria, tento un atterraggio di fortuna\n");
        }

        // Resettiamo il buffer in modo da evitare errori alla prossima lettura
        free(buffer);
        buffer = malloc(sizeof(char));
    } // end while

    // Prima di uscire chiudiamo la socket e puliamo le malloc
    free(buffer);
    free(risposta);
    free(messaggio);
    free(username);
    close(socket_id);
    pthread_exit(NULL);
}
