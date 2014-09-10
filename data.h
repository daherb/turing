#ifndef _DATA_H
#define _DATA_H 1


struct state_list {
  int state;
  struct state_list *next;
  struct state_list *prev;
  struct state_list *first;
  struct state_list *last; // only valid in the first element
};

struct pre_rule {
  int state; //current state
  char symbol; //current tape symbol
};

struct post_rule {
  int state; // new state
  char symbol; // new tape symbol
  char move; // move direction
};

struct rule {
  struct pre_rule pre;
  struct post_rule post;
};

struct rule_list {
  struct rule rule;
  struct rule_list *next;
  struct rule_list *prev;
  struct rule_list *first;
  struct rule_list *last; // only valid in the first element
};

struct program_info {
  unsigned int tape_length;
  char *tape_data;
  int initial_state;
  struct state_list *final_states;
  struct rule_list *rules;
  unsigned int position;
};

extern void add_state_to_list(struct state_list *list, int state);
extern int find_state_in_list(struct state_list *list, int state);
extern void add_rule_to_list(struct rule_list *list, struct rule rule);
extern struct rule find_rule_in_list(struct rule_list *list, struct pre_rule *pre);
extern int check_rule_list(struct rule_list *rules, unsigned int rule_count);
#endif
