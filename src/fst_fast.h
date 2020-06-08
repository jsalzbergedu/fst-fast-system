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

typedef struct MatchObject MatchObject;

struct MatchObject {
  /**
   * The state output
   */
  unsigned short *state_output;
  /**
   * The character output
   */
  char *char_output;

  /**
   * Pointer to the end of the states
   */
  unsigned short *state_end;

  /**
   * Pointer to the end of the characters
   */
  char *char_end;

  /**
   * The char length
   */
  size_t char_length;

  /**
   * The char capacity
   */
  size_t char_capacity;

  /**
   * The state length
   */
  size_t state_length;

  /**
   * The state capacity
   */
  size_t state_capacity;

  /**
   * Match success
   */
  int match_success;

  /*
   * For iterating over the FST States
   */

  /**
   * Beginning fst state
   */
  unsigned char *beginning;

  /**
   * Current FST location
   */
  unsigned char *current;
};

void match_one_char(MatchObject *match_object, char input);

/* void match_one_char(char input, char *output, int *state_number, */
/*                     FstStateEntry *states, FstStateEntry
 * *current_state_start, */
/*                     FstStateEntry **next_state); */

void match_string(InstructionTape *instrtape, MatchObject *match_object,
                  char const *input);

#endif /* FST_FAST_H */
