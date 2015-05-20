#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include <thread_dispatcher.h>
#include <gestione_utenti.h>
#include <main_server.h>

#define K 256
#define MAX 2000

#define MSG_SINGLE 'S'
#define MSG_BRDCAST 'B'

// main_server.h
int go;

typedef struct {
  char *messaggio[K];
  char tipo_messaggio[K];
  int sockid_destinatario[K];

  pthread_mutex_t mutex;

  int readpos, writepos;
  int cont;

  pthread_cond_t PIENO;
  pthread_cond_t VUOTO;
} buffer_circolare;

static buffer_circolare buffer;
static buffer_circolare *puntatore_buffer = &buffer;

void *thread_dispatcher(void *arg)
{
    char *messaggio = malloc(sizeof(char));
    char tipo_messaggio;
    int sockid_destinatario = -1;
    char *elenco_utenti = malloc(sizeof(char));
    //char *nome_utente[256];

    pthread_mutex_init(&puntatore_buffer->mutex, NULL);
    pthread_cond_init(&puntatore_buffer->PIENO, NULL);
    pthread_cond_init(&puntatore_buffer->VUOTO, NULL);
    puntatore_buffer->cont     = 0;
    puntatore_buffer->readpos  = 0;
    puntatore_buffer->writepos = 0;

    while (go) {
        estrai(&messaggio, &tipo_messaggio, &sockid_destinatario);

        if (tipo_messaggio == MSG_SINGLE) {
            if (write(sockid_destinatario, messaggio, strlen(messaggio)) == -1) {
                // TODO gestione errore
            }
        } else {
            elenca_utenti_connessi(elenco_utenti);
            // TODO broadcast
            //while ((nome_utente = strtok(elenco_utenti, "\n")) != 0)
        }

        free(messaggio);
        messaggio = malloc(sizeof(char));
    }
    pthread_exit(NULL);
}

void inserisci (char *messaggio, char tipo_messaggio, int sockid_destinatario)
{
  pthread_mutex_lock(&puntatore_buffer->mutex);

  while (puntatore_buffer->cont == K) {
      pthread_cond_wait(&puntatore_buffer->PIENO, &puntatore_buffer->mutex);
  }

  // scrivi MSG e aggiornato stato puntatore_buffer
  puntatore_buffer->messaggio[puntatore_buffer->writepos] = strdup(messaggio);
  puntatore_buffer->sockid_destinatario[puntatore_buffer->writepos] = sockid_destinatario;

  puntatore_buffer->tipo_messaggio[puntatore_buffer->writepos] = tipo_messaggio;

  puntatore_buffer->cont++;
  puntatore_buffer->writepos++;

  // gestione circolare
  if (puntatore_buffer->writepos >= K) {
      puntatore_buffer->writepos = 0;
  }

  // risveglia un eventuale thread consumatore sospeso
  pthread_cond_signal(&puntatore_buffer->VUOTO);
  pthread_mutex_unlock(&puntatore_buffer->mutex);
}

void estrai(char **messaggio, char *tipo_messaggio, int *sockid_destinatario)
{
  pthread_mutex_lock(&puntatore_buffer->mutex);

  while (puntatore_buffer->cont == 0) {
      pthread_cond_wait(&puntatore_buffer->VUOTO, &puntatore_buffer->mutex);
  }

  *tipo_messaggio = puntatore_buffer->tipo_messaggio[puntatore_buffer->readpos];
  *sockid_destinatario = puntatore_buffer->sockid_destinatario[puntatore_buffer->readpos];

  *messaggio = strdup(puntatore_buffer->messaggio[puntatore_buffer->readpos]);

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
