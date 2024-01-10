#include "queue.hpp"
#include "main.hpp"

struct Queue* create_queue() {
    struct Queue *queue = new Queue();
    queue->size = 0;
    return queue;
}

bool is_empty(struct Queue *queue) {
    if (queue->size == 0) return true;
    else return false;
}

void print_queue(struct Queue *queue) {
    printf("%d --- kolejka --- \n", rank_comm);
    if (is_empty(queue)) { 
        printf("%d --- kolejka pusta --- \n", rank_comm);
        return;
    }
    
    struct part* element = queue->head;
    while (element) {
        printf("%d id:%d | ts:%-5d | c:%d  \n",rank_comm, element->src_id, element->ts, element->cuchy);
        element = element->next;
    }
}

struct part* create_part(int ts, int src_id, int cuchy) {
    struct part* new_elemet = new part();
    new_elemet->cuchy = cuchy;
    new_elemet->src_id = src_id;
    new_elemet->ts = ts;
    return new_elemet;
}

void add_by_time(struct Queue *queue, int ts, int src_id, int cuchy) {
    struct part *new_element = create_part(ts, src_id, cuchy);  //dane o oTAku od którego otrzymaliśmy

    // jeśli kolejka jest pusta ustaw element jako nowy 
    if (is_empty(queue)) {
        queue->head = new_element;
        queue->size++;
        return;
    }

    //znajdź element z większą etykietą czasową (zapis pod head)
    struct part *head = queue->head;
    while ((head->next) && (head->ts < new_element->ts || ((head->ts == new_element->ts) && (head->src_id < new_element->src_id)))) { //przesuwamy nowy element wkolejce do przodu dopóki on ma wjększą wartość zegara od pozostałch
        head = head->next;
    }

    // nie znaloziono z większym znacznikiem - wstaw na koniec
    if ((head->ts < new_element->ts) || (head->ts == new_element->ts && head->src_id < new_element->src_id)) {
        head->next = new_element; //nastepny element będę dla nas head
        new_element->prev = head; // a my dla niego prev
    } else {
    // znaleniono większy znacznik czasowy
        new_element->next = head;  //dla nas najwjększy element jest następnikiem

        // head nie na początku queue
        if (head->prev) {
            head->prev->next = new_element;  // ustałamy dla poprzednika najwjększego elementu wskażnik na nas, że teraz będziemy dla niego next
        } else {
            queue->head = new_element;      //jeśli nie istnieje poprzednika najwiekszego to stajemy na koniec kolejki
        } 
         
        head->prev = new_element;  //jesteśmy poprzednim elementem dla najwjększego
    }
    queue->size++;
    return;
}

int find_by_src(struct Queue *queue, int dupa) {
    printf("%d find_by_src: %d \n",rank_comm, dupa);
    struct part *head = queue->head;
    if (is_empty(queue)) {
        printf("%d !!! Próba pobrania pozycji żądania według id procesu %d na pustej kolejce !!! \n",rank_comm, dupa);
        exit(-1);
    }
    int i = 0;
    while ((head->next) && (head->src_id != dupa)) {
        head = head->next;
        i++;
    }
    if (head->src_id != dupa) {
        printf("%d !!! Próba pobrania id prcesu: %d, który nie istnieje w kolejce !!! \n",rank_comm, dupa);
        exit(-1);
    }
    return i;
}

int find_by_rank(struct Queue *queue) {
    int dupa = rank_comm;
    printf("%d find_by_rank: %d",rank_comm, dupa);
    struct part *head = queue->head;
    if (is_empty(queue)) {
        printf("%d !!! Próba pobrania pozycji żądania według id procesu %d na pustej kolejce !!! \n",rank_comm, dupa);
        exit(-1);
    }
    int i = 0;
    while ((head->next) && (head->src_id != dupa)) {
        head = head->next;
        i++;
    }
    if (head->src_id != dupa) {
        printf("%d !!! Próba pobrania id prcesu: %d, który nie istnieje w kolejce !!! \n",rank_comm, dupa);
        exit(-1);
    }
    return i;
}

void pop_by_src(struct Queue *queue, int src_id) {
    if(is_empty(queue)) {
        printf("%d !!! Próba pobrania żądanie według id procesu %d na pustej kolejce !!! \n",rank_comm, src_id);
        return;
    }
    
    struct part *head = queue->head;
    while(head && head->src_id != src_id) {
        head = head->next;
    }
    if (!head) {
        printf("%d !!! Próba usunięcia z kolejki nieistniejącego id procesu %d !!! \n",rank_comm, src_id);
        return;
    }
    // first element
    if (head == queue->head) queue->head = head->next;
    else head->prev->next = head->next;
    // posiada nastepniki
    if (head->next) head->next->prev = head->prev;
    queue->size--;
    delete head;
}

struct part* get_by_id(struct Queue *queue, int id) {
    struct part *head = queue->head;
    if(is_empty(queue)) {
        printf("%d !!! Próba pobrania żądanie według pozycji %d na pustej kolejce !!! \n",rank_comm, id);
        return nullptr;
    }
    while(head && --id >= 0) {
        head = head->next;
    }
    return head;
}
