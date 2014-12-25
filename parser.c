#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <limits.h>

#include "parser.h"

void parse_config_file(char *file, struct program_info *pinfo)
{
  FILE *filehandle = fopen(file,"r");
  if (filehandle == NULL)
    {
      perror("Error opening program file");
      exit(-1);
    }
  int pre_state, post_state;
  char pre_symbol, post_symbol, direction;
  int status = 0;
  unsigned int rule_count = 0;
  errno = 0;
  pinfo->rules->prev = NULL;
  pinfo->rules->next = NULL;
  pinfo->rules->first = pinfo->rules;
  pinfo->rules->last = pinfo->rules;
  while (status != EOF)
    {
      char line[255];
      status = fscanf(filehandle,"%s",line);
      if (errno !=0)
  	{
  	  perror("Error reading configuration file");
  	  fclose(filehandle);
  	  exit(-1);
  	}
      if (strncmp(line,"program:",strlen("program:")) == 0)
	{
  	  status = fscanf(filehandle,"(%d,%c)->(%d,%c,%c)\n",&pre_state,&pre_symbol,&post_state,&post_symbol,&direction);
	  if (errno !=0)
	    {
	      perror("Error reading rules");
	      fclose(filehandle);
	      exit(-1);
	    }
  	  struct rule rule;
  	  rule.pre.symbol  = pre_symbol;
  	  rule.pre.state   = pre_state;
  	  rule.post.symbol = post_symbol;
  	  rule.post.state  = post_state;
  	  rule.post.move   = direction;
  	  add_rule_to_list(pinfo->rules,rule);
  	  rule_count++;
	  printf("Read %d rules\n",rule_count);
	  check_rule_list(pinfo->rules,rule_count);
	}
      else if(strncmp(line,"tape-length:",strlen("tape-length:")) == 0)
	{
	  status = fscanf(filehandle,"%s\n",line);
	  if (errno !=0)
	    {
	      perror("Error reading rules");
	      fclose(filehandle);
	      exit(-1);
	    }
	  pinfo->tape_length = parse_tape_length(line);
	  printf("got length %lld\n",pinfo->tape_length);
	}
      else if(strncmp(line,"final-states:",strlen("final-states:")) == 0)
	{

	}
      else if(strncmp(line,"initial-state:",strlen("initial-state:")) == 0)
	{
	  puts("Reading initial state");
	}
      else if(strncmp(line,"initial-tape:",strlen("initial-tape:")) == 0)
	{
	  puts("Reading rules");
	}
    }
}

long long int parse_tape_length(char *length)
{
  long long int temp_length = strtoll(length, NULL, 10);
  if (temp_length > 0 && temp_length <= LLONG_MAX)
    return temp_length;
  else
    {
      if (temp_length == LLONG_MIN || temp_length == LLONG_MAX)
	perror("");
      fputs("Invalid input for tape-length. It must be a number between 1 and ",stderr);
      fprintf(stderr,"%lld\n",LLONG_MAX);
      exit(-1);
    }
}

void parse_final_states(struct state_list *final_states, char *states)
{
  char tmp[16];
  int tmppos = 0;
  int pos = 0;
  int state;
  final_states->prev = NULL;
  final_states->next = NULL;
  final_states->first = final_states;
  final_states->last = final_states;
  for (; pos< strlen(states); pos++)
    {
      if (states[pos] != ',')
	{
	  tmp[tmppos] = states[pos];
	  tmppos++;
	}
      else
	{
	  tmp[tmppos+1] = '\0';
	  tmppos = 0;
	  state = parse_state(tmp);
	  add_state_to_list(final_states,state);
	}
    }
  state = parse_state(tmp);
  add_state_to_list(final_states,state);
}

int parse_state(char *state)
{
  char *rest=calloc(16,sizeof(char));
  long int temp_state = strtol(state, (char **) &rest, 10);
  free(rest);
  if (temp_state >= INT_MIN && temp_state <= INT_MAX && state != NULL && strcmp(state,rest) != 0)
    return temp_state;
  else
    {
      if (temp_state == LONG_MIN || temp_state == LONG_MAX)
	perror("Bla");
      fputs("Invalid input for state. It must be a number between ",stderr);
      fprintf(stderr,"%d and %d\n", INT_MIN, INT_MAX);
      exit(-1);
    }
}

void init_empty_tape(char **tape, long long int length)
{
  *tape = calloc(length,sizeof(char));
  if (*tape == NULL)
    {
      perror("Error allocating data tape");
      exit(-1);
    }
  printf("Initialized empty tape with size %lld\n",length);
}

void init_data_tape(char **tape, long int length, char *datafile)
{
  init_empty_tape(tape,length);
  FILE *filehandle = fopen(datafile,"r");
if (filehandle == NULL)
    {
      perror("Error opening data file for tape");
      exit(-1);
    }
  size_t bytes = fread(*tape,sizeof(char),length,filehandle);
  printf("Read %zd bytes of data to the tape\n",bytes);
  if (ferror(filehandle))
    {
      perror("Error reading tape data");
      fclose(filehandle);
      exit(-1);
    }
  fclose(filehandle);
}
