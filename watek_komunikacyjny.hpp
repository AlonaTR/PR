#ifndef WATEK_KOMUNIKACYJNY_HPP
#define WATEK_KOMUNIKACYJNY_HPP

/* wątek komunikacyjny: odbieranie wiadomości i reagowanie na nie poprzez zmiany stanu */
void *startKomWatek(void *ptr);
// sprawdzenie mozliwosci wejscia do pokoju
void try_to_enter();
// update aktualną liczbę cuchów z kolejki
void update_cuchy(int pos);

#endif