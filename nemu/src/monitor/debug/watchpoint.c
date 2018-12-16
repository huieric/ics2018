#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP* new_wp() {
  if (free_ == NULL) {
    Log("Not enough watchpoint");
    assert(0);
    return NULL;
  }
  WP* wp = free_;
  free_ = free_->next;

  wp->next = head;
  head = wp;

  return head;
}

void free_wp(int n) {
  if (head->NO == n) {
    WP* wp = head->next;
    head->next = free_;
    free_ = head;
    head = wp;
    return;
  }
  WP* p = NULL;
  for (p = head; p->next != NULL; p = p->next) {
    if (p->next->NO == n) {
      WP* wp = p->next;
      p->next = wp->next;
      wp->next = free_;
      free_ = wp;
      return;
    }
  }
  /*if (p == NULL) {*/
    /*Log("Watchpoint not found");*/
    /*assert(0);*/
    /*return;*/
  /*}*/
}

void list_wp() {
  if (head == NULL) {
    printf("No watchpoints.\n");
    return;
  }
  printf("Num     Type          Disp Enb Address          What\n");
  for (WP* p = head; p != NULL; p = p->next) {
    printf("%-8d%-14s%-5s%-4s%-17s%s\n", p->NO, "hw watchpoint", "keep", "y", "", p->expr);
  }
}

bool check_wp() {
  bool bChanged = false;
  for (WP* p = head; p != NULL; p = p->next) {
    Log("check watchpoint %d: %s.", p->NO, p->expr);
    bool success;
    uint32_t val = expr(p->expr, &success);
    if (val != p->val) {
      printf("nemu: watchpoint %d expr %s changed from %u to %u\n", 
	  p->NO, p->expr, p->val, val);
      p->val = val;
      bChanged = true;
    }
  }
  return bChanged;
}
