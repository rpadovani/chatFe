/*****************************************************************
 *  ChatFe - Progetto di Sistemi Operativi '14/'15 UniFe         *
 *                                                               *
 *  Riccardo Padovani (115509) riccardo@rpadovani.com            *
 *****************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include <main_client.h>
#include <thread_writer.h>
#include <thread_listener.h>

// Indirizzo IP e porta del server
#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 57223

// Define del tipo di messaggi così come concordati con il server
#define MSG_LOGIN 'L'
#define MSG_REGLOG 'R'
#define MSG_OK 'O'
#define MSG_ERROR 'E'

int main(int argc, char *argv[])
{
    /*
        La variabile rappresenta il riferimento alla socket che verrà aperta
        e sarà poi usata da thread_writer e thread_listener
     */
    int socket_id;

    /*
        La struct serve per configurare la connessione della socket del client
        al server che sarà effettuata a SERVER_IP:SERVER_ADDRESS
     */
    struct sockaddr_in client;

    /*
        Il puntatore punta a una stringa, la cui dimensione sarà allocata
        dinamicamente, su cui scriveremo il messaggio da inviare al server
        di volta in volta per la registrazione e il login
     */
    char *buffer = malloc(sizeof(char));

    /*
        Queste variabili sono di supporto: all'interno della funzione le useremo
        varie volte.
        In particolare: username conterrà lo username passato da linea di
        comando al lancio dell´utente.

        Il messaggio_registrazione servirà per contenere una copia del comando
        di registrazione da inviare al server, che poi sarà inviato da
        buffer.
     */
    char *username;
    char *messaggio_registrazione;

    /*
        Per inviare il buffer dobbiamo saperne la dimensione.
        Visto che può tornarci utile in varie occasioni, li dedichiamo
        una variabile.
     */
    int dimensione_buffer;

    // Teniamo conto di quanti tentavi di connessione facciamo
    int numero_tentativi_connessione = 0;

    /*
        Se l'utente ha inserito come primo argomento -h, stampiamo il manuale
        e usciamo
     */
    if (argv[1] != NULL && strcmp(argv[1], "-h") == 0) {
        printf("## Manuale utente ##\n");
        printf("Questo è un client testuale per una chat\n");
        printf("Sintassi del comando:\n");
        printf("chat-client [-h] [-r ”Name Surname email”] username\n");
        printf("-h invoca questo manuale\n");
        printf("-r indica la registrazione, seguita da nome, cognome, mail\n");
        return 0;
    }

    /*
        A questo punto, o viene passato solo un argomento oltre l'eseguibile,
        oppure c'è stata la registrazione, oppure gli argomenti sono sbagliati
     */
    if (!(argc == 2 ||
        (argc == 4 && argv[1] != NULL && strcmp(argv[1], "-r") == 0))
    ) {
        printf("Comando invocato in maniera sbagliata.\n");
        printf("Consultare -h per ulteriori informazioni.\n");

        return -1;
    }

    // Creiamo una socket
    socket_id = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_id < 0) {
        fprintf(stderr, "Impossibile creare la socket\n");
    }

    // Prepariamo la nostra sockaddr_in in una struttra dedicata
    client.sin_family = AF_INET;
    client.sin_addr.s_addr = inet_addr(SERVER_IP);
    client.sin_port = htons(SERVER_PORT);

    // Proviamo a connetterci
    while (connect(socket_id, (struct sockaddr *)&client, sizeof(client)) < 0) {
        numero_tentativi_connessione++;
        printf("Connessione fallita. Ritento... [%i/10]\n",
          numero_tentativi_connessione);

        // Attendiamo un secondo prima di riprovare a connetterci
        sleep(1);

        if (numero_tentativi_connessione == 10) {
            fprintf(stderr, "Impossibile connettersi al server. Bye!\n");
            return -1;
        }
    }

    /*
        Se siamo arrivati qua l'utente o vuole registrarsi o vuole fare
        direttamente il login.
        In ogni caso ricaviamo l'username e salviamolo.
        Nel caso di registrazione o login l'username è in una posizione diversa
        Usiamo una varaibile di supporto per scegliere l'argomento giusto.
     */
    char *argument = (argc == 4 ? argv[3] : argv[1]);
    username = malloc(strlen(argument) + 1);
    strcpy(username, argument);

    /*
        Se l'utente ha inserito come primo argomento -r, iniziamo la procedura
        di registrazione, che si occupa anche di fare il login
    */
    if (argc == 4 && strcmp(argv[1], "-r") == 0) {
        messaggio_registrazione = malloc(strlen(argv[2]) + 1);
        strcpy(messaggio_registrazione, argv[2]);

        /*
            Il messaggio di registrazione passato dall'utente è nel formato
            Nome Cognome mail

            Noi dobbiamo passarlo al server nel formato
            username:Nome Cognome:mail

            Per quanto riguarda l'username l'abbiamo già recuperato e lo
            concatenaiamo in un secondo momento.

            Dobbiamo quindi sostituire l'ultima occorrenza dello spazio con un
            :

            Non sappiamo a priori quanti spazi ci sono, perché un nome potrebbe
            contenere uno spazio (Maria Antonietta) o nessuno se l'utente
            non desidera fornire il cognome.

            Quindi usando le caratteristiche dei puntatori inizieremo a scorrere
            la stringa all'indietro, finché non troviamo il primo spazio,
            che sostituiamo con un :
         */
        char *char_da_modificare =
                    &messaggio_registrazione[strlen(messaggio_registrazione)];

        while (char_da_modificare > messaggio_registrazione &&
                *char_da_modificare != ' ') {
            char_da_modificare--;
        }

        if (*char_da_modificare == ' ') {
            *char_da_modificare = ':';
        }

        /*
            Mesaggio di login, la struttura del messaggio da inviare
            al server è la seguente:
            - 1 byte che rappresenta il tipo di messaggio
            - 4 byte (1 int) che rappresentano la dimensione del
                campo destinatario (in questo caso 0)
            - 4 byte (1 int) che rappresenta la lunghezza del messaggio
            - n byte di messaggio, in base alla lunghezza dello
                stesso
            - 1 byte di terminatore di stringa aggiunto da sprintf
         */
        dimensione_buffer = 1 + 4 + 4 + strlen(messaggio_registrazione)
                                                        + strlen(username) + 1;

        buffer = realloc(buffer, dimensione_buffer);
        sprintf(
            buffer,
            "%c0000%04zu%s:%s",
            MSG_REGLOG,
            strlen(messaggio_registrazione) + strlen(username) + 1,
            username,
            messaggio_registrazione
        );

        /*
            Inviamo il messaggio SENZA il terminatore di stringa
            perché grazie alla dimensione del campo il server
            sa esattamente quanti byte leggere
         */
        if (write(socket_id, buffer, dimensione_buffer) == -1) {
            fprintf(stderr, "Impossible contattare il server\n");
            return -1;
        }

        buffer = realloc(buffer, sizeof(char));
        if (read(socket_id, buffer, sizeof(char)) < 0) {
            fprintf(stderr, "Errore leggendo la risposta del server\n");
            return -1;
        }

        if (buffer[0] != MSG_OK) {
            fprintf(stderr, "Impossibile registrare l'utente.\n");
            return -1;
        }

        printf("Registrazione e login effettuati con successo.\n");

        // Liberiamo alcune delle variaibli utilizzate
        free(messaggio_registrazione);
    } else {
        /*
            Messaggio di login, la struttura del messaggio da inviare
            al server è la seguente:
            - 1 byte che rappresenta il tipo di connessione
            - 4 byte (1 int) che rappresentano la dimensione del
                campo destinatario (in questo caso 0)
            - 4 byte (1 int) che rappresenta la lunghezza del nome
                utente con cui fare il login
            - n byte di nome utente, in base alla lunghezza dello
                stesso
         */
        dimensione_buffer = 1 + 4 + 4 + strlen(username);

        /*
            sprintf inserisce automaticamente un terminatore di
            stringa quindi lasciamo lo spazio anche per quello
         */
        buffer = realloc(buffer, dimensione_buffer + 1);
        sprintf(buffer, "%c0000%04zu%s", MSG_LOGIN, strlen(username), username);

        /*
            Inviamo il messaggio SENZA il terminatore di stringa
            perché grazie alla dimensione del campo il server
            sa esattamente quanti byte leggere
         */
        if (write(socket_id, buffer, dimensione_buffer) == -1) {
            fprintf(stderr, "Impossibile effettuare il login\n");
            return -1;
        }

        /*
            A questo punto il messaggio è stato inviato. Attendiamo un
            messaggio di risposta dal server per vedere se il nome utente è
            valido ed eventualmente creare i thread che si occupano di leggere
            e scrivere.
            Il reply è di 1 char: nel caso ci sia stato un errore allocheremo
            altro spazio per leggere il messaggio d'errore
        */
        buffer = realloc(buffer, sizeof(char));
        if (read(socket_id, buffer, sizeof(char)) < 0) {
            fprintf(stderr, "Errore leggendo la risposta del server\n");
            return -1;
        }

        if (buffer[0] != MSG_OK) {
            // Leggiamo quanto è lungo l'errore
            //buffer = realloc(buffer, sizeof(int));
            //read(socket_id, buffer, sizeof(int));

            // Allochiamo lo spazio per il messaggio
            //buffer = realloc(buffer, atoi(buffer));
            //read(socket_id, buffer, sizeof(int));

            fprintf(stderr, "Impossibile effettuare il login\n");
            return -1;
        }

        printf("Login effettuato con successo\n");
    }

    // Liberiamo tutte le variabili che abbiamo utilizzato e non servono più
    free(buffer);
    free(username);

    /*
        Arrivati a questo punto il login è avvenuto con successo. Dobbiamo
        quindi creare i due thread che si occupano di gestire l'invio e la
        ricezione dei messaggi
     */
    pthread_t thread_listener_id;
    pthread_t thread_writer_id;

    if (pthread_create(
            &thread_listener_id,
            NULL,
            &thread_listener,
            (void *) &socket_id
        ) != 0) {
        fprintf(stderr, "Impossible creare il thread listener\n");
        return -1;
    }

    if (pthread_create(
            &thread_writer_id,
            NULL,
            &thread_writer,
            (void *) &socket_id
        ) != 0) {
        fprintf(stderr, "Impossible creare il thread writer\n");
        return -1;
    }

    /*
        Quando uno dei due thread joina è perché ha ricevuto il messaggio di
        logout, quindi si occupa di chiudere l'altro thread, e poi di terminare
        il programma.

        In particolare il thread_listener viene chiuso dal messaggio di chiusura
        inviato dal server quando il server va giù, mentre il thread_writer
        verrà chiuso se in stdin l'utente scrive #logout
     */
    pthread_join(thread_listener_id, NULL);
    pthread_kill(thread_writer_id, 2);

    close(socket_id);
    return 0;

    pthread_join(thread_writer_id, NULL);
    pthread_kill(thread_listener_id, 2);

    close(socket_id);
    return 0;
}
