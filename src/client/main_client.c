#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include <main_client.h>

/*
    La define PROTOCOLLO indica con quale protocollo creare la socket.
    Accetta due valori:
    - AF_UNIX se il server e i client girano sullo stesso sistema
    - AF_INET per connettersi ad un server esterno
 */
#define PROTOCOLLO AF_INET

// Indirizzo del server su cui è in esecuzione il programma server
#define SERVER_IP "127.0.0.1"

/*
    La define SERVER_PORT definisce la porta su cui ci aspettiamo che il server
    sia in ascolto
 */
#define SERVER_PORT 8887

#define MSG_LOGIN 'L'
#define MSG_REGLOG 'R'
#define MSG_OK 'O'
#define MSG_ERROR 'E'
#define MSG_SINGLE 'S'
#define MSG_BRDCAST 'B'
#define MSG_LIST 'I'
#define MSG_LOGOUT 'X'

int main(int argc, char const *argv[])
{
    /*
        La variabile rappresenta il riferimento alla socket che verrà aperta
        e sarà usata da altre funzioni riguardanti la socket
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
        di volta in volta
     */
    char *buffer = malloc(sizeof(char));

    char *username;
    char *messaggio_registrazione;

    /*
        Per inviare il buffer dobbiamo saperne la dimensione.
        Visto che può tornarci utile in varie occasioni, li dedichiamo
        una variabile a parte
     */
    int dimensione_buffer;

    int numero_tentativi_connessione = 0;

    /*
        Se l'utente ha inserito come primo argomento -h, stampiamo il manuale
        e usciamo
     */
    if (strcmp(argv[1], "-h") == 0) {
        printf("## Manuale utente ##\n");
        printf("Questo è un client testuale per una chat\n");
        printf("Sintassi del comando:\n");
        printf("chat-client [-h] [-r ”Name Surname email”] username\n");
        printf("-h invoca questo manuale\n");
        printf("-r indica la registrazione, seguita da nome, cognome, mail\n");
        return 0;
    }

    // Creiamo una socket
    socket_id = socket(PROTOCOLLO, SOCK_STREAM, 0);

    if (socket_id < 0) {
        printf("Impossibile creare la socket");
    }

    // Prepariamo la nostra sockaddr_in in una struttra dedicata
    client.sin_family = PROTOCOLLO;
    client.sin_addr.s_addr = inet_addr(SERVER_IP);
    client.sin_port = htons(SERVER_PORT);

    // Proviamo a connetterci
    while (connect(socket_id, (struct sockaddr *)&client, sizeof(client)) < 0) {
        numero_tentativi_connessione++;
        printf("Connessione fallita. Ritento... [%i/10]\n", numero_tentativi_connessione);
        sleep(1);

        if (numero_tentativi_connessione == 10) {
            printf("Impossibile connettersi al server. Bye!\n");
            return -1;
        }
    }

    /*
        Se siamo arrivati qua o l'utente o vuole registrarsi o vuole fare
        direttamente il login.
        In ogni caso prendiamo l'username e facciamo il login
     */
    username = malloc(sizeof(argc == 4 ? argv[3] : argv[1]));
    strcpy(username, argc == 4 ? argv[3] : argv[1]);

    /*
        Se l'utente ha inserito come primo argomento -r, iniziamo la procedura
        di registrazione, a cui poi seguirà il login
    */
    if (argc == 4 && strcmp(argv[1], "-r")) {
        /*
            Mesaggio di login, la struttura del messaggio da inviare
            al server è la seguente:
            - 1 byte che rappresenta il tipo di connessione
            - 4 byte (1 int) che rappresentano la dimensione del
                campo destinatario (in questo caso 0)
            - 4 byte (1 int) che rappresenta la lunghezza del messaggio
            - n byte di messaggio, in base alla lunghezza dello
                stesso
         */
        dimensione_buffer = 1 + 4 + 4 + strlen(username);

        /*
            sprintf inserisce automaticamente un terminatore di
            stringa quindi lasciamo lo spazio anche per quello
         */
        buffer = realloc(buffer, dimensione_buffer + 1);
        sprintf(buffer, "%c0000%04zu%s", MSG_LOGIN, strlen(username), username);

    }

    /*
        A questo punto, o viene passato solo un argomento oltre l'eseguibile
        oppure c'è stata la registrazione, oppure gli argomenti sono sbagliati
     */
    if (argc != 2 || (argc == 4 && strcmp(argv[1], "-r"))) {
        printf("Comando invocato in maniera sbagliata.\n");
        printf("Consultare -h per ulteriori informazioni\n");

        return -1;
    }

    /*
        Mesaggio di login, la struttura del messaggio da inviare
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
        printf("Impossibile effettuare il login");
        return -1;
    }

    /*
        A questo punto il messaggio è stato inviato. Attendiamo un messaggio di
        risposta dal server per vedere se il nome utente è valido ed
        eventualmente creare i thread che si occupano di leggere e scrivere.
        Il reply è di 1 char: nel caso ci sia stato un errore allocheremo
        altro spazio per leggere il messaggio d'errore
    */
    buffer = realloc(buffer, sizeof(char));
    if (read(socket_id, buffer, sizeof(char)) < 0) {
        printf("Errore leggendo la risposta del server");
        return -1;
    }

    if (buffer[0] != MSG_OK) {
        // Leggiamo quanto è lungo l'errore
        //buffer = realloc(buffer, sizeof(int));
        //read(socket_id, buffer, sizeof(int));

        // Allochiamo lo spazio per il messaggio
        //buffer = realloc(buffer, atoi(buffer));
        //read(socket_id, buffer, sizeof(int));

        printf("Impossibile effettuare il login:");
        return -1;
    }

    printf("Login effettuato con successo\n");

    // Crea i thread

    return 0;
}
