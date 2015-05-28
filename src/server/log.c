/*****************************************************************
 *  ChatFe - Progetto di Sistemi Operativi '14/'15 UniFe         *
 *                                                               *
 *  Riccardo Padovani (115509) riccardo@rpadovani.com            *
 *****************************************************************/
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

#include <main_server.h>
#include <log.h>
#include <gestione_utenti.h>

// main_server.h
char *file_log;

// log.h
void init_log(void)
{
    // Come da consegna, il file viene troncato ad ogni nuova esecuzione
    FILE *p_file = fopen(file_log, "w+");
    char ts[64];
    timestamp(ts);

    if (p_file == NULL) {
        fprintf(stderr, "Impossibile aprire il file di log\n");
        return;
    }

    fprintf(p_file, "****************************************************\n");
    fprintf(p_file, "** Chat Server started @ %s **\n", ts);
    fprintf(p_file, "****************************************************\n");

    fclose(p_file);
}

// log.h
void log_login(char *username)
{
  // I nuovi messaggi vanno appesi al file
  FILE *p_file = fopen(file_log, "a");
  char ts[64];
  timestamp(ts);

  if (p_file == NULL) {
      fprintf(stderr, "Impossibile aprire il file di log\n");
      return;
  }

  fprintf(p_file, "%s:login:%s\n", ts, username);
  fclose(p_file);
}

// log.h
void log_logout(char *username)
{
  FILE *p_file = fopen(file_log, "a");
  char ts[64];
  timestamp(ts);

  if (p_file == NULL) {
      fprintf(stderr, "Impossibile aprire il file di log\n");
      return;
  }

  fprintf(p_file, "%s:logout:%s\n", ts, username);
  fclose(p_file);
}

// log.h
void log_messaggio_singolo(char *mittente, char *destinatario, char *messaggio)
{
    FILE *p_file = fopen(file_log, "a");
    char ts[64];
    timestamp(ts);

    if (p_file == NULL) {
        fprintf(stderr, "Impossibile aprire il file di log\n");
        return;
    }

    fprintf(p_file, "%s:%s:%s:%s\n", ts, mittente, destinatario, messaggio);
    fclose(p_file);
}

// log.h
void log_messaggio_broadcast(char *mittente, char *messaggio)
{
    char *elenco_utenti = malloc(sizeof(char));

    // gestione_utenti.h
    // Prendiamo l'elenco di tutti gli utenti connessi
    elenca_utenti_connessi(elenco_utenti);

    // Almeno un utente connesso c'è (chi ha inviato il messaggio)
    char *nome_utente = strtok(elenco_utenti, ":");

    do {
        log_messaggio_singolo(mittente, nome_utente, messaggio);

        // Prendiamo un altro utente connesso, se esiste. Altrimenti usciamo
        nome_utente = strtok(NULL, ":");
    } while (nome_utente != NULL);
}

// log.h
void timestamp (char *ts)
{
    time_t t;
    t = time(NULL);
    ctime_r(&t, ts);
    ts[strlen(ts)-1] = '\0';
}
