#include "linkedlist.h"
#include <stdlib.h>

Node *list_push_front(Node *head, int val) {

  Node *newNode = malloc(sizeof(Node));
  newNode->val = val;
  newNode->next = head;
  return newNode;
}

size_t list_length(const Node *head) {
  size_t count = 0;
  Node *temp = (Node *)head;
  while (temp) {
    count++;
    temp = temp->next;
  }
  return count;
}

Node *list_reverse(Node *head) {
  Node *prev = NULL;
  while (head) {
    Node *next = head->next;
    head->next = prev;
    prev = head;
    head = next;
  }
  return prev;
}

int list_middle(const Node *head) {
  if (head == NULL) {
    return 0;
  }
  Node *slow = (Node *)head;
  Node *fast = (Node *)head;
  while (fast->next && fast->next->next) {
    slow = slow->next;
    fast = fast->next->next;
  }
  if (fast->next) {
    slow = slow->next;
  }
  return slow->val;
}

bool list_has_cycle(const Node *head) {
  if (head == NULL) {
    return 0;
  }
  Node *slow = (Node *)head;
  Node *fast = (Node *)head;
  while (fast->next && fast->next->next) {
    slow = slow->next;
    fast = fast->next->next;
    if (slow == fast) {
      return true;
    }
  }
  return false;
}

void list_free(Node *head) {
  while (head) {
    Node *temp = head;
    head = head->next;
    free(temp);
  }
}
