#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>

#include "data.h"

void usage(char *command)
{
  fputs("Usage: ",stdout); fputs(command,stdout); puts(" program final-states [tape-length] [initial-state] [initial-tape] [final-tape]");
  puts("\tprogram: Program file consisting of rules of the following form");
  puts("\t\t(old-state,old-tape-symbol)->(new-state,new-tape-symbol,move) with move one of l=left,r=right,n=none,h=halt");
  puts("\tfinal-states: Comma-separated list of final states to stop in");
  puts("\tape-length: Optional length of the storage tape (default 4096)");
  puts("\tinitial-state: Optional state to start in (default 0)");
  puts("\tinitial-tape: Optional data to be stored on the tape before the program is ran");
  puts("\tfinal-tape: Optional file to store the tape to after program finished");
}

int set_tape_length(char *length)
{
  long int temp_length = strtol(length, NULL, 10);
  if (temp_length > 0 && temp_length <= UINT_MAX)
    return temp_length;
  else 
    {
      if (temp_length == LONG_MIN || temp_length == LONG_MAX)
	perror(""); 
      fputs("Invalid input for tape-length. It must be a number between 1 and ",stderr);
      fprintf(stderr,"%u\n",UINT_MAX);
      exit(-1);
    }
}

int set_state(char *state)
{
  char *rest=calloc(16,sizeof(char));
  long int temp_state = strtol(state, (char **) &rest, 10);
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

void set_final_states(struct state_list *final_states, char *states)
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
	  state = set_state(tmp);
	  add_state_to_list(final_states,state);
	}
    }
  state = set_state(tmp);
  add_state_to_list(final_states,state);
}

void load_program(char *file, struct rule_list *rules)
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
  rules->prev = NULL;
  rules->next = NULL;
  rules->first = rules;
  rules->last = rules;
  while (status != EOF)
    {
      status = fscanf(filehandle,"(%d,%c)->(%d,%c,%c)\n",&pre_state,&pre_symbol,&post_state,&post_symbol,&direction);
      struct rule rule;
      rule.pre.symbol  = pre_symbol;
      rule.pre.state   = pre_state;
      rule.post.symbol = post_symbol;
      rule.post.state  = post_state;
      rule.post.move   = direction;
      add_rule_to_list(rules,rule);
      rule_count++;
    }
  if (errno !=0)
    {
      perror("Error reading program");
        fclose(filehandle);
      exit(-1);
    }
  printf("Read %d rules\n",rule_count);
  check_rule_list(rules,rule_count);
  fclose(filehandle);
}

void init_empty_tape(char **tape, unsigned int length)
{
  *tape = calloc(length,sizeof(char));
  if (*tape == NULL)
    {
      perror("Error allocating data tape");
      exit(-1);
    }
  printf("Initialized empty tape with size %u\n",length);
}

void init_data_tape(char **tape, unsigned int length, char *datafile)
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

int run_program(char *tape, unsigned int position, unsigned int tape_length, int state, struct state_list *final_states, struct rule_list *rules)
{
  struct timeval tv_start;
  gettimeofday(&tv_start,NULL);
  char pre_symbol = tape[position];
  int pre_state   = state;
  char post_symbol;
  int post_state;
  char move;
  unsigned int step = 1;
  while(1)
    {
      // Find rule
      struct pre_rule pre;
      pre.symbol = pre_symbol;
      pre.state = pre_state;
      struct rule rule = find_rule_in_list(rules,&pre);
      post_symbol = rule.post.symbol;
      post_state = rule.post.state;
      move = rule.post.move;
      tape[position] = post_symbol;
      if (move == 'r')
	position++;
      else if (move == 'l')
	position--;
      if (position == UINT_MAX)
	position = tape_length -1;
      if (position > tape_length -1)
	position = position % tape_length;
      pre_symbol = tape[position];
      pre_state = post_state;
      printf("Step %d State %d Position %d: %s\n", step, post_state, position, tape);
      step++;
      // Check if new state is a final state
      if (move == 'h' || find_state_in_list(final_states,post_state)>0)
	{
	  struct timeval tv_end;
	  gettimeofday(&tv_end,NULL);
	  printf("Got final tape in state %d after %ld useconds:\n",post_state,(long int)(tv_end.tv_usec-tv_start.tv_usec));
	  puts(tape);
	  return 0;
	}
    }
}

void save_tape(char *tape, unsigned int tape_length, char *file)
{
  FILE *filehandle = fopen(file,"w");
  if (filehandle == NULL)
    {
      perror("Error opening tape output file");
      exit(-1);
    }
  fwrite(tape,sizeof(char),tape_length,filehandle);
  if (ferror(filehandle))
    {
      perror("Error writing tape data");
      fclose(filehandle);
      exit(-1);
    }
  fclose(filehandle);
}
int main(int argc, char **argv)
{
  // Check if at least a program and some final states were given
  if (argc < 3)
    {
      usage(argv[0]);
      exit(0);
    }
  char *tape = NULL;
  unsigned int tape_length = 4096;
  unsigned int position = 0;
  int state = 0;
  struct state_list final_states;
  struct rule_list rules;
  // Set to user-defined length if enough parameters given
  if (argc > 3)
      tape_length = set_tape_length(argv[3]);
  // Set to user-defined initial state if enough parameters given
  if (argc > 4)
      state = set_state(argv[4]);
  // Set to user-defined tape data if enough parameters given or initialize the tape empty
  if (argc > 5)
    init_data_tape(&tape,tape_length,argv[5]);
  else
    init_empty_tape(&tape,tape_length);
  load_program(argv[1],&rules);
  set_final_states(&final_states,argv[2]);
  run_program(tape,position,tape_length,state,&final_states,&rules);
  if (argc > 6)
    save_tape(tape,tape_length,argv[6]);
  return 0;
}
