#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>

#include "data.h"
#include "parser.h"

void usage(char *command)
{
  //  fputs("Usage: ",stdout); fputs(command,stdout); puts(" program final-states [tape-length] [initial-state] [initial-tape] [final-tape]");
  fputs("Usage: ",stdout); fputs(command,stdout); puts(" program [--single-step] [final-tape-output]");
  puts("program: Program file consisting of:");
  puts("\tIdentifier \"program:\" followed by rules of the following form\n\twith each rule in a new line:");
  puts("\t(old-state,old-tape-symbol)->(new-state,new-tape-symbol,move) with\n\t\told-state and new-state a number,\n\t\told-tape-symbol and new-tape-symbol a 8-bit character and\n\t\tmove one of l=left,r=right,n=none,h=halt");
  puts("\tOptional identifier \"final-states:\" followed by a comma-separated\n\t\tlist of final states to automatically stop in");
  puts("\tOptional identifier \"tape-length:\" followed by desired length of\n\t\tthe storage tape (default 4096)");
  puts("\tOptional identifier \"initial-state:\" followed by state to start\n\t\tin (default 0)");
  puts("\tOptional identifier \"initial-tape:\" followed by a data block to be\n\t\tstored on the tape before the program is executed (otherwise\n\t\tinitialized with 0's)");
  puts("--single-step: Optinal parameter to output in every step the state, position\nand tape (for debugging etc.)");
  puts("final-tape-output: Optional parameter of a file to store the tape to after\nprogram finished");
}

int run_program(struct program_info *pinfo)
{
  struct timeval tv_start;
  gettimeofday(&tv_start,NULL);
  char *tape = pinfo->tape_data;
  unsigned int position = pinfo->position;
  char pre_symbol = tape[position];
  int pre_state   = pinfo->initial_state;
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
      struct rule rule = find_rule_in_list(pinfo->rules,&pre);
      post_symbol = rule.post.symbol;
      post_state = rule.post.state;
      move = rule.post.move;
      tape[position] = post_symbol;
      if (move == 'r')
	position++;
      else if (move == 'l')
	position--;
      if (position == UINT_MAX)
	position = pinfo->tape_length -1;
      if (position > pinfo->tape_length -1)
	position = position % pinfo->tape_length;
      pre_symbol = tape[position];
      pre_state = post_state;
      if (pinfo->single_step)
	printf("Step %d State %d Position %d: %s\n", step, post_state, position, tape);
      step++;
      // Check if new state is a final state
      if (move == 'h' || find_state_in_list(pinfo->final_states,post_state)>0)
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
/*   FILE *filehandle = fopen(file,"w"); */
/*   if (filehandle == NULL) */
/*     { */
/*       perror("Error opening tape output file"); */
/*       exit(-1); */
/*     } */
/*   fwrite(tape,sizeof(char),tape_length,filehandle); */
/*   if (ferror(filehandle)) */
/*     { */
/*       perror("Error writing tape data"); */
/*       fclose(filehandle); */
/*       exit(-1); */
/*     } */
/*   fclose(filehandle); */
}

int main(int argc, char **argv)
{
  // Check if at least a program file is given
  if (argc < 2)
    {
      usage(argv[0]);
      exit(0);
    }
  struct program_info pinfo;
  parse_config_file(argv[1],&pinfo);
  /* run_program(&pinfo); */
   if (argc >= 3)
     {
       if (strncmp(argv[2],"--single-step",strlen("--single-step")) == 0)
	 {
	   pinfo.single_step = true;
	   if (argc == 4)
	     save_tape(pinfo.tape_data,pinfo.tape_length,argv[3]);
	 }
       else
	 save_tape(pinfo.tape_data,pinfo.tape_length,argv[3]);
     }
  return 0;
}
