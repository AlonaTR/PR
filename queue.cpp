#include "queue.hpp"

struct Queue* create_queue() {
    struct Queue *queue = (struct Queue*)malloc(sizeof(struct Queue));
    queue->size = 0;
}

bool is_empty(struct Queue *queue) {
    if (queue->size == 0) return true;
    else return false;
}

void print_queue(struct Queue *queue) {
    struct part* element = queue->head;
    debug("kolejka");
    if (is_empty(queue)) return;
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