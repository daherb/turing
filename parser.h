#ifndef _PARSER_H
#define _PARSER_H 1

#include "data.h"

void parse_config_file(char *file, struct program_info *pinfo);
long long int parse_tape_length(char *length);
void parse_final_states(struct state_list *final_states, char *states);
int parse_state(char *state);

#endif
