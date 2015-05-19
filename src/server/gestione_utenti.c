#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include <thread_main.h>
#include <gestione_utenti.h>
#include <main_server.h>
#include <lista.h>

/*
    Per la descrizione generale delle funzioni presenti in questo file
    consultare i rispettivi header, indicati da un commento all'inizio delle
    funzioni stesse
*/

// main_server.h
char *file_utenti;

// gestione_utenti.h
static hash_t HASH_TABLE;

/*
    Per gestire gli utenti connessi utilizziamo una lista che contiene gli
    username, creata e mantenuta con le funzioni indicate in lista.h
 */
static lista utenti_connessi;

#define MSG_OK 'O'
#define MSG_ERROR 'E'

// gestione_utenti.h
void carica_utenti(void)
{
    hdata_t *utente;

    utenti_connessi = CREALISTA();
    HASH_TABLE = CREAHASH();
    /*
        Controlliamo se il file esiste con la funzione access(). È disponibile
        solo sui sistemi Unix, ma le specifiche indicano chiaramente l'utilizzo
        di sistemi Linux.

        La funzione prende in input il file da controllare e uno dei seguenti
        parametri:
        - 0         Controlla se il file esiste
        - 2         Controlla se ci sono i permessi di scrittura
        - 4         Controlla i permessi di lettura
        - 6         Controlla sia i permessi di lettura che quelli di scrittura

        La funzione ritorna 0 se il file soddisfa i requisiti, -1 altrimenti
    */
    if (access(file_utenti, 4) == 0) {
        // Il file esiste, lo apriamo e lo assegniamo alla variabile file
        FILE *p_file = fopen(file_utenti, "r");
        /*
            Facciamo comunque un controllo se il file è stato caricato, potrebbe
            fallire per vari motivi, come per esempio mancanza di spazio in
            memoria
        */
        if (p_file != NULL) {
            /*
                Il file è stato aperto, leggiamolo linea per linea.
                La dimensione della linea letta è massimo 770 caratteri,
                data da 256 (dimensione massima di ogni campo) * 3 più due
                caratteri separatori (:) più il terminatore di stringa (\0)
            */
            char linea[771];

            /*
                Leggiamo ogni linea del file, una alla volta, e la inseriamo
                nella variabile linea
            */
            while (fgets (linea, sizeof(linea), p_file)) {
                 // Puntatore a struct che sarà usata per inserire la linea letta
                utente = (hdata_t *) malloc(sizeof(hdata_t));

                /*
                    Dalla linea estraiamo i tre campi che ci interessano,
                    delimitati dai due punti
                */
                utente->uname = strdup(strtok(linea, ":"));
                utente->fullname = strdup(strtok(NULL, ":"));
                /*
                    Visto che la mail è l'ultimo campo della stringa vogliamo
                    evitare che abbia la newline al suo interno
                */
                utente->email = strdup(strtok(NULL, ":\n"));

                /*
                    Controlliamo di aver estratto tutti e tre i campi.
                    Se non abbiamo tutti e tre i campi (quindi la linea è
                    nel formato sbagliato), ignoriamo la linea e continuiamo
                    con quella successiva
                */
                if (utente->uname != NULL && utente->fullname != NULL && utente->email != NULL) {
                    /*
                        Popoliamo la struct nuovo utente e inseriamola in hash
                        con le funzioni che ci sono state fornite
                    */
                    utente->sockid   = -1;
                    INSERISCIHASH(utente->uname, (void*) utente, HASH_TABLE);
                }
            }
            // Abbiamo fatto tutto, possiamo chiudere il file
            fclose (p_file);
        } else {
            // TODO: gestione errori
        }
    } // endif access(file_utenti, 4) == 0

    /*
        Poiché il file può non esistere, non c'è un branch else, semplicemente
        non carichiamo niente nella tabella utenti, ma non restituiamo errori
    */
}

// gestione_utenti.h
char login_utente(char *username, int socket_id)
{
    hdata_t *risultato_ricerca = NULL;
    risultato_ricerca = CERCAHASH(username, HASH_TABLE);
    posizione ultimo_elemento = ULTIMOLISTA(utenti_connessi);

    if (risultato_ricerca != NULL && risultato_ricerca->sockid == -1) {
        INSLISTA(username, &ultimo_elemento);
        risultato_ricerca->sockid = socket_id;
        return MSG_OK;
    }

    // TODO error management
    return MSG_ERROR;
}

// gestione_utenti.h
char registrazione_utente(char *messaggio, int socket_id, char *username)
{
    char *nome;
    char *mail;
    hdata_t *utente;

    // Il messaggio è nel formato username:nome:mail
    username = strdup(strtok(messaggio, ":"));

    // Lo username non deve essere registrato
    if (CERCAHASH(username, HASH_TABLE) != NULL) {
      // TODO error management

        return MSG_ERROR;
    }

    nome = strdup(strtok(NULL, ":"));
    mail = strdup(strtok(NULL, ":"));

    utente = (hdata_t *) malloc(sizeof(hdata_t));
    utente->uname = username;
    utente->fullname = nome;
    utente->email = mail;
    // Il login sarà effettuato in un secondo momento
    utente->sockid = socket_id;
    // Inseriamo la struttura appena popolata.
    INSERISCIHASH(utente->uname, (void*) utente, HASH_TABLE);
    return MSG_OK;
}

// gestione_utenti.h
void elenca_utenti_connessi(char *risposta)
{
    posizione elemento_lista = PRIMOLISTA(utenti_connessi);

    // La variabile risposta deve aver già allocato un char
    risposta[0] = '\0';

    // Il primo nome utente non ha il separatore all'inizio
    bool is_first = true;

    while (PREDLISTA(elemento_lista) != ULTIMOLISTA(utenti_connessi)) {
      /*
          L'array della risposta deve avere abbastanza spazio prima di
          poter concatenare il nuovo nome utente.
          Lo riallochiamo quindi aumentandogli la dimensione pari al nome
          utente che dobbiamo inserire + 1 per il separatore
       */
      if (is_first) {
          is_first = false;
          risposta = realloc(risposta,
              strlen(risposta) + strlen(elemento_lista->elemento));
          risposta = strcat(risposta, elemento_lista->elemento);
      } else {
          risposta = realloc(risposta,
              strlen(risposta) + strlen(elemento_lista->elemento) + 1);
          risposta = strcat(strcat(risposta, ":"),
              elemento_lista->elemento);
      }
      elemento_lista = SUCCLISTA(elemento_lista);
    }
}

// gestione_utenti.h
void logout_utente(char *username)
{
    hdata_t *risultato_ricerca = NULL;
    posizione elemento_lista = PRIMOLISTA(utenti_connessi);

    // Lo username deve essere nella hast table
    if ((risultato_ricerca = CERCAHASH(username, HASH_TABLE)) == NULL) {
      // TODO error management
    }

    risultato_ricerca->sockid = -1;

    while (PREDLISTA(elemento_lista) != ULTIMOLISTA(utenti_connessi)) {
        if (strcmp(elemento_lista->elemento, username) == 0) {
          CANCLISTA(&elemento_lista);
          break;
        }
        elemento_lista = SUCCLISTA(elemento_lista);
    }
}

int esiste_utente(char *username)
{
    hdata_t *risultato_ricerca = NULL;
    if ((risultato_ricerca = CERCAHASH(username, HASH_TABLE)) == NULL) {
        return 1;
    }
    return 0;
}

int sockid_username(char *username)
{
  hdata_t *risultato_ricerca = NULL;
  if ((risultato_ricerca = CERCAHASH(username, HASH_TABLE)) == NULL) {
      return -1;
  }
  return risultato_ricerca->sockid;
}
