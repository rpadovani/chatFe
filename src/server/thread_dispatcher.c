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

#include <thread_dispatcher.h>
#include <gestione_utenti.h>
#include <main_server.h>

// Dimensione del dispatcher
// XXX: se viene aggiornato qua, aggiornarlo anche in thread_dispatcher.h
#define K 256

#define MSG_SINGLE 'S'
#define MSG_BRDCAST 'B'

// main_server.h
int go;

/*
    Creiamoci il nostro buffer, accessibile solo da questa funzione, e un
    puntatore al buffer stesso
 */
static buffer_circolare buffer;
static buffer_circolare *puntatore_buffer = &buffer;

// thread_dispatcher.h
void *thread_dispatcher(void *arg)
{
    // Preariamo un po' di variabili di supporto
    char *messaggio = malloc(sizeof(char));
    char tipo_messaggio;
    int sockid_destinatario = -1;
    char *elenco_utenti = malloc(sizeof(char));

    // Resettiamo la struttura del buffer
    pthread_mutex_init(&puntatore_buffer->mutex, NULL);
    pthread_cond_init(&puntatore_buffer->PIENO, NULL);
    pthread_cond_init(&puntatore_buffer->VUOTO, NULL);
    puntatore_buffer->cont     = 0;
    puntatore_buffer->readpos  = 0;
    puntatore_buffer->writepos = 0;

    while (go) {
        // Estraiamo un messaggio
        estrai(&messaggio, &tipo_messaggio, &sockid_destinatario);

        // Se il messaggio è singolo lo proviamo a recapitare
        if (tipo_messaggio == MSG_SINGLE) {
            if (write(sockid_destinatario, messaggio, strlen(messaggio)) == -1) {
                fprintf(stderr, "Impossibile recapire il messaggio\n");
            }
        } else {
            // Il messaggio è in broadcast, prendiamo tutti gli utenti connessi
            elenca_utenti_connessi(elenco_utenti);
            // Almeno un utente connesso c'è (chi ha inviato il messaggio)
            char *nome_utente = strtok(elenco_utenti, ":");

            do {
                /*
                    Per ogni utente connesso recuperiamo il suo sockid e
                    proviamo ad inviargli un messaggio
                 */
                if (write(
                      sockid_username(nome_utente),
                      messaggio,
                      strlen(messaggio)
                  ) == -1) {
                    fprintf(stderr, "Impossibile recapire il messaggio\n");
                }
                nome_utente = strtok(NULL, ":");
            } while (nome_utente != NULL);
        }

        // Prepariamoci per il prossimo messaggio
        free(messaggio);
        messaggio = malloc(sizeof(char));
    }

    free(messaggio);
    free(elenco_utenti);
    pthread_exit(NULL);
}

// thread_dispatcher.h
void inserisci(char *messaggio, char tipo_messaggio, int sockid_destinatario)
{
  pthread_mutex_lock(&puntatore_buffer->mutex);

  // Fintanto che il buffer è pieno, attendiamo
  while (puntatore_buffer->cont == K) {
      pthread_cond_wait(&puntatore_buffer->PIENO, &puntatore_buffer->mutex);
  }

  // Scrive tutto quello che è stato passato come argomento
  puntatore_buffer->messaggio[puntatore_buffer->writepos] = strdup(messaggio);
  puntatore_buffer->sockid_destinatario[puntatore_buffer->writepos] =
                                                            sockid_destinatario;
  puntatore_buffer->tipo_messaggio[puntatore_buffer->writepos] = tipo_messaggio;

  // Aggiorna il contatore e la posizione in cui scrive
  puntatore_buffer->cont++;
  puntatore_buffer->writepos++;

  // Gestione circolare
  if (puntatore_buffer->writepos >= K) {
      puntatore_buffer->writepos = 0;
  }

  // risveglia un eventuale thread consumatore sospeso
  pthread_cond_signal(&puntatore_buffer->VUOTO);
  pthread_mutex_unlock(&puntatore_buffer->mutex);
}

// thread_dispatcher.h
void estrai(char **messaggio, char *tipo_messaggio, int *sockid_destinatario)
{
  pthread_mutex_lock(&puntatore_buffer->mutex);

  // Attendiamo ci sia qualcosa
  while (puntatore_buffer->cont == 0) {
      pthread_cond_wait(&puntatore_buffer->VUOTO, &puntatore_buffer->mutex);
  }

  // Estraiamo tutti i dati dal buffer
  *tipo_messaggio = puntatore_buffer->tipo_messaggio[puntatore_buffer->readpos];
  *sockid_destinatario =
              puntatore_buffer->sockid_destinatario[puntatore_buffer->readpos];
  *messaggio = strdup(puntatore_buffer->messaggio[puntatore_buffer->readpos]);

  // Aggiorniamo contatore e posizione del lettore
  puntatore_buffer->cont--;
  puntatore_buffer->readpos++;

  // gestione circolare
  if (puntatore_buffer->readpos >= K) {
      puntatore_buffer->readpos = 0;
  }

  // risveglia eventuale thread produttore
  pthread_cond_signal(&puntatore_buffer->PIENO);
  pthread_mutex_unlock(&puntatore_buffer->mutex);
}
