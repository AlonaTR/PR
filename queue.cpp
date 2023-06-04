#include "queue.hpp"

struct Queue* create_queue() {
    // struct Queue *queue = (struct Queue*)malloc(sizeof(struct Queue));
    struct Queue *queue = new Queue();
    queue->size = 0;
}

bool is_empty(struct Queue *queue) {
    if (queue->size == 0) return true;
    else return false;
}

void print_queue(struct Queue *queue) {
    debug("--- kolejka ---");
    if (is_empty(queue)) { 
        debug("--- kolejka pusta ---");
        return;
    }
    
    struct part* element = queue->head;
    while (element) {
        debug(" id:%d | ts:%-5d | c:%d ", element->src_id, element->ts, element->cuchy);
        element = element->next;
    }
}

struct part* create_part(int ts, int src_id, int cuchy) {
    struct part* new_elemet = new part();
    new_elemet->cuchy = cuchy;
    new_elemet->src_id = src_id;
    new_elemet->ts = ts;
}

void add_by_time(struct Queue *queue, int ts, int src_id, int cuchy) {
    struct part *new_element = create_part(ts, src_id, cuchy);

    // jeśli kolejka jest pusta ustaw element jako nowy 
    if (is_empty(queue)) {
        queue->head = new_element;
        queue->size++;
        return;
    }

    //znajdź element z większą etykietą czasową (zapis pod head)
    struct part *head = queue->head;
    while ((head->next) && (head->ts < new_element->ts)) {
        head = head->next;
    }

    // nie znaloziono z większym znacznikiem - wstaw na koniec
    if (head->ts <= new_element->ts) {
        head->next = new_element;
        new_element->prev = head;
    } else {
    // znaleniono większy znacznik czasowy
        new_element->next = head;
        // head nie na początku queue
        if (head->prev) {
            head->prev->next = new_element;
            new_element->prev = head->prev;
        } else {
            queue->head = new_element;
        }
    }
    queue->size++;
    return;
}

int find_by_src(struct Queue *queue, int src_id) {
    struct part *head = queue->head;
    if (is_empty(queue)) {
        debug("!!! Próba pobrania pozycji żądania według id procesu %d na pustej kolejce !!!", src_id);
        exit(-1);
    }
    int i = 0;
    while ((head->next) && (head->src_id != src_id)) {
        head = head->next;
        i++;
    }
    if (head->src_id != src_id) {
        debug("!!! Próba pobrania id prcesu: %d, który nie istnieje w kolejce !!!", src_id);
        exit(-1);
    }
    return i;
}

void pop_by_src(struct Queue *queue, int src_id) {
    struct part *head = queue->head;
    if(is_empty(queue)) {
        debug("!!! Próba pobrania żądanie według id procesu %d na pustej kolejce !!!", src_id);
        return;
    }
    while(head && head->src_id != src_id) {
        head = head->next;
    }
    if (!head) {
        debug("!!! Próba usunięcia z kolejki nieistniejącego id procesu %d !!!", src_id);
        return;
    }
    // first element
    if (! head->prev) queue->head = head->next;
    else head->prev->next = head->next;
    if (head->next) head->next->prev = head->prev;
    queue->size--;
}

struct part* get_by_id(struct Queue *queue, int i) {
    struct part *head = queue->head;
    if(is_empty(queue)) {
        debug("!!! Próba pobrania żądanie według pozycji %d na pustej kolejce !!!", i);
        return nullptr;
    }
    while(head && --i >= 0) {
        head = head->next;
    }
    return head;
}
