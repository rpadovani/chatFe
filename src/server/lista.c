#include <stdio.h>
#include <stdlib.h>

typedef struct cella * lista;
typedef lista          posizione;

struct cella {
  posizione precedente;
  void *    elemento;
  posizione successivo;
};


lista CREALISTA () {
  lista L;
  L = (lista) malloc(sizeof (struct cella) );
  L->successivo = L;
  L->precedente = L;
  return L;
}


int LISTAVUOTA (lista L) {
  int listavuota;
  listavuota = ((L->successivo == L) && (L->precedente == L)) ? 1 : 0;
  return listavuota;
}


posizione PRIMOLISTA (lista L) {
  return L->successivo;
}


posizione ULTIMOLISTA (lista L) {
  return L->precedente;
}


posizione SUCCLISTA (posizione p) {
  return p->successivo;
}


posizione PREDLISTA (posizione p) {
  return p->precedente;
}


int FINELISTA (posizione p, lista L) {
  int finelista;
  finelista = (p == L) ? 1 : 0;
  return finelista;
}


void INSLISTA (void * data, posizione * p) {
  struct cella * tmp;

  tmp = (struct cella *) malloc(sizeof(struct cella));

  tmp->precedente = (*p)->precedente;
  tmp->successivo = (*p);

  tmp->elemento = data;

  (*p)->precedente->successivo = tmp;
  (*p)->precedente             = tmp;

  (*p) = tmp;
}


void CANCLISTA (posizione * p) {
  posizione tmp;

  tmp = (*p);

  (*p)->precedente->successivo = (*p)->successivo;
  (*p)->successivo->precedente = (*p)->precedente;

  (*p) = (*p)->successivo;

  free(tmp);
}
