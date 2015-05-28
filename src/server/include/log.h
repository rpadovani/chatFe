/*****************************************************************
 *  ChatFe - Progetto di Sistemi Operativi '14/'15 UniFe         *
 *                                                               *
 *  Riccardo Padovani (115509) riccardo@rpadovani.com            *
 *****************************************************************/
#ifndef LOG_H
#define LOG_H

/*
    In tutto il file non ci sono mutex perché l'implementazione di glibc delle
    funzioni che riguardano i file sono thread-safe, così come descritto da
    https://www.gnu.org/software/libc/manual/html_node/Streams-and-Threads.html

    Ovviamente se ci fossero più processi (non thread) separati del server in
    esecuzione che puntano allo stesso flie potrebbero esserci dei problemi,
    ma questo esula dagli scopi del progetto.
 */

/*
    La funzione viene chiamata all'avvio del server, si occupa di creare il file
    di log e di inserire l'intestazione.
 */
void init_log(void);

/*
    La funzione prende in input uno username e scrive sul file di log l'accesso
    di quell'utente
 */
void log_login(char *username);

/*
    La funzione prende in input uno username e scrive sul file di log l'uscita
    di quell'utente
 */
void log_logout(char *username);

/*
    La funzione si occupa di loggare il messaggio passato come argomento.
    Vengono passati anche mittente e destinatario.

    La funzione viene usata anche da log_messaggio_broadcast
 */
void log_messaggio_singolo(char *mittente, char *destinatario, char *messaggio);

/*
    Dato un mittente e un messaggio la funzione si occupa di inserire nel file
    di log una riga con il messaggio per ogni utente connesso, così come da
    consenga
 */
void log_messaggio_broadcast(char *mittente, char *messaggio);

/*
    Funzione che si è stata fornita nella consegna che si occupa di creare un
    timestamp e inserirlo nella variabile passata come argomento
 */
void timestamp(char *ts);

#endif
