#include <stdlib.h>
#include <stdio.h>
#include "data.h"

void add_state_to_list(struct state_list *list, int state)
{
  struct state_list *last = list->last;
  last->next = calloc(1,sizeof(struct state_list));
  if ((last->next) == NULL)
    {
      perror("Error allocating memory for state list element");
    }
  last->next->state = state;
  last->next->first = last->first;
  last->next->last  = last->next;
  last->next->next  = NULL;
  last->next->prev  = last;
  last->last        = NULL;
  last->first->last = last->next;
}

void add_rule_to_list(struct rule_list *list, struct rule rule)
{
  struct rule_list *last = list->last;
  last->next = calloc(1,sizeof(struct rule_list));
  if ((last->next) == NULL)
    {
      perror("Error allocating memory for state list element");
    }
  last->next->rule  = rule;
  last->next->first = last->first;
  last->next->last  = last->next;
  last->next->next  = NULL;
  last->next->prev  = last;
  last->last        = NULL;
  last->first->last = last->next;
}

int find_state_in_list(struct state_list *list, int state)
{
  struct state_list *e = list->next;
  while (e != NULL)
    {
      if (e->state == state)
	return 1;
      e = e-> next;
    }
  return 0;
}

struct rule find_rule_in_list(struct rule_list *list, struct pre_rule *pre)
{
  struct rule_list *e = list->next;
  while (e != NULL)
    {
      if (e->rule.pre.state == pre->state && e->rule.pre.symbol == pre->symbol)
	return e->rule;
      e = e-> next;
    }
  fprintf(stderr,"No useable rule found for state %d and symbol '%c'\n",pre->state,pre->symbol);
  exit(-1);
}

int check_rule_list(struct rule_list *rules, unsigned int rule_count)
{
  return 1;
}
