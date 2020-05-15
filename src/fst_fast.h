#ifndef FST_FAST_H
#define FST_FAST_H

#include <stdlib.h>

typedef union FstStateEntry FstStateEntry;

struct FstStateEntryComponents {
  char flags;
  char outchar;
  unsigned short out_state;
};

union FstStateEntry {
  struct FstStateEntryComponents components;
  int entry;
};

typedef struct InstructionTape InstructionTape;

struct InstructionTape {
  unsigned char *beginning;
  unsigned char *current;
  size_t length;
  size_t capacity;
};

void fst_clear_flag(FstStateEntry *fse);

void fst_set_initial_flag(FstStateEntry *fse);

void fst_set_valid_flag(FstStateEntry *fse);

void fse_set_final_flags(InstructionTape *instrtape);

void fse_set_initial_flags(InstructionTape *instrtape);

void fse_set_outchar(FstStateEntry *fse, char a);

void fse_set_outstate(FstStateEntry *fse, unsigned short outstate);

void fse_initialize_tape(InstructionTape *instrtape);

void fse_grow(InstructionTape *instrtape, int targetlen);

void fse_clear_instr(InstructionTape *instrtape, unsigned short errorstate);

FstStateEntry *fse_get_outgoing(InstructionTape *instrtape, char c);

void fse_finish(InstructionTape *instrtape);

void instruction_tape_destroy(InstructionTape *instrbuff);

void create_pegreg_diffmatch(InstructionTape *instrtape);

void match_one_char(char input, char *output, int *state_number,
                    FstStateEntry *states, FstStateEntry *current_state_start,
                    FstStateEntry **next_state);

char *match_string(const char *input, InstructionTape *instrtape,
                   int *match_success, unsigned short **matched_states);

#endif /* FST_FAST_H */
