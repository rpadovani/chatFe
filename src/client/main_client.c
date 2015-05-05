#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

#define SERVER_IP "127.0.0.1"

/*
    La define SERVER_PORT definisce la porta su cui ci aspettiamo che il server
    sia in ascolto
 */
#define SERVER_PORT 8888

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
        La struct serve per configurare la connessione della socket del server
        che sarà bindata alla porta definita dalla costante SERVER_PORT
     */
    struct sockaddr_in client;

    char *buffer = malloc(sizeof(char));

    int dimensione_buffer;

    int lunghezza_nome;

    /*
        Guardiamo quanti argomenti ci sono per capire che tipo di comando
        abbiamo ricevuto.

        Se sono 2 (quindi il comando più un argomento) allora abbiamo o il
        comando di help o il comando di login

        Se sono 4 (il comando più tre argomenti), allora abbiamo la
        registrazione
     */
    if (argc == 2) {
        if (strcmp(argv[1], "-h") == 0) {
            printf("## Manuale utente ##\n");
            printf("Questo è un client testuale per una chat\n");
            printf("Sintassi del comando:\n");
            printf("chat-client [-h] [-r ”Name Surname email”] username\n");
            printf("-h invoca questo manuale\n");
            printf("-r indica la registrazione, seguita da nome, cognome, mail\n");
        } else {
            printf("Login con utente %s\n", argv[1]);

            socket_id = socket(PROTOCOLLO, SOCK_STREAM, 0);
            if (socket_id != -1) {
                // Prepariamo la nostra sockaddr_in in una struttra dedicata
                client.sin_family = PROTOCOLLO;
                client.sin_addr.s_addr = inet_addr(SERVER_IP);
                client.sin_port = htons(SERVER_PORT);

                if (connect(socket_id, (struct sockaddr *)&client, sizeof(client)) >= 0) {
                    lunghezza_nome = strlen(argv[1]);
                    dimensione_buffer = 1 + 8 + 4 + lunghezza_nome;
                    buffer = realloc(buffer, dimensione_buffer + 1);
                    sprintf(buffer,"%c00000000%04d%s", MSG_LOGIN, lunghezza_nome, argv[1]);
                    if (send(socket_id, buffer, dimensione_buffer, 0)) {
                        printf("inviato\n");
                    } else {
                        printf("invio fallito\n");
                    }
                } else {
                    printf("connessione fallita\n");
                }
            }
        }
    } else if (argc == 4 && strcmp(argv[1], "-r") == 0) {
        printf("registrazione utente\n");
    } else {
        printf("Comando invocato in maniera sbagliata. Consultare -h per ulteriori informazioni\n");
    }

    return 0;
}
