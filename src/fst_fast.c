/**
 * FST with as much as possible precompiled
 * @author Jacob Salzberg (jssalzbe)
 * @file fst_fast.c
 */
#include <assert.h>
#include "fst_fast.h"
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
#include <memory.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))

// This is basically a header.
// The formal definition is
// (Q, Sigma, Gamma, I, F, Delta)
// Or (States, InputAlphabet, OutputAlphabet, InitialStates, FinalStates,
// TransitionRelation)

// HOWEVER, fastest matching is translate this to
// a directed graph, and store the outgoing edges
// in each vertex

// Vertex definition would look like:
// Header
// # States
// # Initial States
// # Final States
// Body:
// Final states as vector of ints
// Vector of states beginning with the inital states
// Each state contains a 256 long vector of (short, char, char)
// AKA Out-state, junk data, out-character
// The position in the 256-long vector is the in - character
// and the state's index is the state #
// This limits the largets possible FST to 0.5 GB
// The state list will always begin with the initial states.

/**
 * Whether the transition is valid.
 * Only matters for non-determinsitic fsts
 */
#define FST_FLAG_VALID (1 << 0)

/**
 * Whether the fst state is initial
 */
#define FST_FLAG_INITIAL (1 << 1)

/**
 * Whether the fst state is final
 */
#define FST_FLAG_FINAL (1 << 2)

// Compile the FST a:a
// AKA:
// -> (0) -a:a-> ((1))
//    ||| !a:0    ||| !?:0
//    \ /         |||
//     -          |||
//    (2)-|||?:0  |||
//     -     |||  |||
//    / \    |||  |||
//    |||    |||  |||
//    ||======||  |||
//    |============||
void create_a_to_a(unsigned char *outbuff) {
  for (int i = 0; i < 256; i++) {
    FstStateEntry fse;
    if (i == 'a') {
      fse.components.flags = 0;
      fse.components.flags |= FST_FLAG_VALID;
      fse.components.flags |= FST_FLAG_INITIAL;
      fse.components.out_state = 1;
      fse.components.outchar = 'a';
    } else {
      fse.components.flags = 0;
      fse.components.flags |= FST_FLAG_VALID;
      fse.components.flags |= FST_FLAG_INITIAL;
      fse.components.out_state = 2;
      fse.components.outchar = 0;
    }

    *((FstStateEntry *) outbuff) = fse;
    outbuff += sizeof(FstStateEntry);
  }

  for (int i = 0; i < 256; i++) {
    FstStateEntry fse;
    fse.components.flags = 0;
    fse.components.flags |= FST_FLAG_VALID;
    fse.components.flags |= FST_FLAG_FINAL;
    fse.components.out_state = 2;
    fse.components.outchar = 0;
    *((FstStateEntry *) outbuff) = fse;
    outbuff += sizeof(FstStateEntry);
  }

  for (int i = 0; i < 256; i++) {
    FstStateEntry fse;
    fse.components.flags = 0;
    fse.components.flags |= FST_FLAG_VALID;
    fse.components.out_state = 2;
    fse.components.outchar = 0;
    *((FstStateEntry *) outbuff) = fse;
    outbuff += sizeof(FstStateEntry);
  }
}

/*
 * Using the PEGREG:
 * A <- aa
 * B <- a
 * K <- ab
 * (A/B)K
 * Translated to the FST:
 * Q (states):
 * 0: {q0, a0, b0}
 * 1: {q1, b1, Fb, Kb0}
 * 2: {a1, Fa, Ka0, Kb1}
 * 3: {Ka1}
 * 4: {Ka2}
 * 5: {}
 * Transitions:
 * 0 -a:a-> 1
 * 1 -a:a-> 2
 * 2 -a:a-> 3
 * 3 -b:b-> 4
 * 4 -?:0-> 5
 */
void create_pegreg_abk(unsigned char *outbuff) {
  for (int i = 0; i < 256; i++) {
    FstStateEntry fse;
    if (i == 'a') {
      fse.components.flags = 0;
      fse.components.flags |= FST_FLAG_VALID;
      fse.components.flags |= FST_FLAG_INITIAL;
      fse.components.out_state = 1;
      fse.components.outchar = 'a';
    } else {
      fse.components.flags = 0;
      fse.components.flags |= FST_FLAG_VALID;
      fse.components.flags |= FST_FLAG_INITIAL;
      fse.components.out_state = 5;
      fse.components.outchar = 0;
    }

    *((FstStateEntry *) outbuff) = fse;
    outbuff += sizeof(FstStateEntry);
  }

  for (int i = 0; i < 256; i++) {
    FstStateEntry fse;
    if (i == 'a') {
      fse.components.flags = 0;
      fse.components.flags |= FST_FLAG_VALID;
      fse.components.out_state = 2;
      fse.components.outchar = 'a';
    } else {
      fse.components.flags = 0;
      fse.components.flags |= FST_FLAG_VALID;
      fse.components.out_state = 5;
      fse.components.outchar = 0;
    }

    *((FstStateEntry *) outbuff) = fse;
    outbuff += sizeof(FstStateEntry);
  }

  for (int i = 0; i < 256; i++) {
    FstStateEntry fse;
    if (i == 'a') {
      fse.components.flags = 0;
      fse.components.flags |= FST_FLAG_VALID;
      fse.components.out_state = 3;
      fse.components.outchar = 'a';
    } else {
      fse.components.flags = 0;
      fse.components.flags |= FST_FLAG_VALID;
      fse.components.out_state = 5;
      fse.components.outchar = 0;
    }

    *((FstStateEntry *) outbuff) = fse;
    outbuff += sizeof(FstStateEntry);
  }

  for (int i = 0; i < 256; i++) {
    FstStateEntry fse;
    if (i == 'b') {
      fse.components.flags = 0;
      fse.components.flags |= FST_FLAG_VALID;
      fse.components.out_state = 4;
      fse.components.outchar = 'b';
    } else {
      fse.components.flags = 0;
      fse.components.flags |= FST_FLAG_VALID;
      fse.components.out_state = 5;
      fse.components.outchar = 0;
    }

    *((FstStateEntry *) outbuff) = fse;
    outbuff += sizeof(FstStateEntry);
  }

  for (int i = 0; i < 256; i++) {
    FstStateEntry fse;
    fse.components.flags = 0;
    fse.components.flags |= FST_FLAG_VALID;
    fse.components.flags |= FST_FLAG_FINAL;
    fse.components.out_state = 5;
    fse.components.outchar = 0;
    *((FstStateEntry *) outbuff) = fse;
    outbuff += sizeof(FstStateEntry);
  }

  for (int i = 0; i < 256; i++) {
    FstStateEntry fse;
    fse.components.flags = 0;
    fse.components.flags |= FST_FLAG_VALID;
    fse.components.out_state = 5;
    fse.components.outchar = 0;
    *((FstStateEntry *) outbuff) = fse;
    outbuff += sizeof(FstStateEntry);
  }
}

void fse_clear_flag(FstStateEntry *fse) {
  fse->components.flags = (char) 0;
}

void fse_set_initial_flag(FstStateEntry *fse) {
  fse->components.flags |= FST_FLAG_INITIAL;
}

void fse_set_valid_flag(FstStateEntry *fse) {
  fse->components.flags |= FST_FLAG_VALID;
}

void fse_set_final_flags(InstructionTape *instrtape) {
  FstStateEntry *fse = (FstStateEntry *) instrtape->current;
  for (int i = 0; i < 256; i++) {
    fse->components.flags |= FST_FLAG_FINAL;
    fse += 1;
  }
}

void fse_set_initial_flags(InstructionTape *instrtape) {
  FstStateEntry *fse = (FstStateEntry *) instrtape->current;
  for (int i = 0; i < 256; i++) {
    fse->components.flags |= FST_FLAG_INITIAL;
    fse += 1;
  }
}

void fse_set_outchar(FstStateEntry *fse, char a) {
  FstStateEntry copy = *fse;
  copy.components.outchar = a;
  *fse = copy;
}

void fse_set_outstate(FstStateEntry *fse, unsigned short outstate) {
  fse->components.out_state = outstate;
}

/**
 * Initialize FSE tape
 */
void fse_initialize_tape(InstructionTape *instrtape) {
  instrtape->capacity = 10;
  instrtape->beginning = (unsigned char *) malloc(instrtape->capacity *
                                                  sizeof(FstStateEntry) * 256);
  if (!(instrtape->beginning)) {
    perror("Memory allocation failure");
    exit(1);
  }
  instrtape->current = instrtape->beginning;
  instrtape->length = 0;
}

/**
 * Grow instruction tape if neccessary
 */
void fse_grow(InstructionTape *instrtape, int targetlen) {
  if (instrtape->capacity <= targetlen) {
    instrtape->capacity = MAX(instrtape->capacity * 2, targetlen);
    instrtape->beginning =
        realloc(instrtape->beginning,
                instrtape->capacity * sizeof(FstStateEntry) * 256);
    if (!(instrtape->beginning)) {
      perror("Memory allocation failure");
      exit(1);
    }
  }
};

/**
 * Clear an entire instruction
 */
void fse_clear_instr(InstructionTape *instrtape, unsigned short errorstate) {
  fse_grow(instrtape, instrtape->length + 1);
  instrtape->length += 1;

  FstStateEntry *fse = (FstStateEntry *) instrtape->current;
  for (int i = 0; i < 256; i++) {
    fse_clear_flag(fse);
    fse_set_valid_flag(fse);
    fse_set_outchar(fse, 0);
    fse_set_outstate(fse, errorstate);
    fse += 1;
  }
}

/**
 * Get the outgoing edge on the character
 */
FstStateEntry *fse_get_outgoing(InstructionTape *instrtape, char c) {
  return ((FstStateEntry *) instrtape->current) + c;
}

/**
 * Finish the current FST vertex
 */
void fse_finish(InstructionTape *instrtape) {
  instrtape->current += sizeof(FstStateEntry) * 256;
}

/**
 * Free resources in instrbuff
 */
void instruction_tape_destroy(InstructionTape *instrbuff) {
  free(instrbuff->beginning);
}

/*
 * And now one where the path taken changes which capture is made
 * A <- aa
 * B <- ab
 * K <- x
 * (A/B)K
 * States
 * 0: {q0, a0, b0}
 * 1: {a1, b1}
 * 2: {Fa, Ka0}
 * 3: {Fka}
 * 4: {Fb, Kb0}
 * 5: {Fkb}
 * 6: {}
 * Transitions
 * 0 -a:a-> 1
 * 1 -a:a-> 2
 * 1 -b:b-> 4
 * 2 -x:x-> 3
 * 3 -?:0-> 6
 * 4 -x:x-> 5
 * 5 -?:0-> 6
 * 6 -?:0-> 6
 */
void create_pegreg_diffmatch(InstructionTape *instrtape) {
  /* State 0 */
  {
    fse_clear_instr(instrtape, 6);
    fse_set_initial_flags(instrtape);
    {
      FstStateEntry *fse = fse_get_outgoing(instrtape, 'a');
      fse_set_outstate(fse, 1);
      fse_set_outchar(fse, 'a');
    }
    fse_finish(instrtape);
  }

  /* State 1 */
  {
    fse_clear_instr(instrtape, 6);
    {
      FstStateEntry *fse = fse_get_outgoing(instrtape, 'a');
      fse_set_outstate(fse, 2);
      fse_set_outchar(fse, 'a');
    }
    {
      FstStateEntry *fse = fse_get_outgoing(instrtape, 'b');
      fse_set_outstate(fse, 4);
      fse_set_outchar(fse, 'b');
    }
    fse_finish(instrtape);
  }

  /* State 2 */
  {
    fse_clear_instr(instrtape, 6);
    {
      FstStateEntry *fse = fse_get_outgoing(instrtape, 'x');
      fse_set_outstate(fse, 3);
      fse_set_outchar(fse, 'x');
    }
    fse_finish(instrtape);
  }

  /* State 3 */
  {
    fse_clear_instr(instrtape, 6);
    fse_set_final_flags(instrtape);
    fse_finish(instrtape);
  }

  /* State 4 */
  {
    fse_clear_instr(instrtape, 6);
    {
      FstStateEntry *fse = fse_get_outgoing(instrtape, 'x');
      fse_set_outstate(fse, 5);
      fse_set_outchar(fse, 'x');
    }
    fse_finish(instrtape);
  }

  /* State 5 */
  {
    fse_clear_instr(instrtape, 6);
    fse_set_final_flags(instrtape);
    fse_finish(instrtape);
  }

  /* State 6 */
  {
    fse_clear_instr(instrtape, 6);
    fse_finish(instrtape);
  }
}

/*
 * Using the PEGREG:
 * A <- aa
 * B <- a
 * K <- ab
 * (B/A)K
 * Translated to the FST:
 * Q (states):
 * 0: {q0, a0, b0}
 * 1: {a1, Fb, Kb0}
 * 2: {Kb1}
 * 3: {FKb}
 * 4: {}
 * Transitions:
 * 0 -a:a-> 1
 * 1 -a:a-> 2
 * 2 -b:b-> 3
 * 3 -?:0-> 4
 * 4 -?:0-> 4
 */
void create_pegreg_bak(InstructionTape *it) {
  /* State 0 */
  {
    fse_clear_instr(it, 4);
    fse_set_initial_flags(it);
    {
      FstStateEntry *fse = fse_get_outgoing(it, 'a');
      fse_set_outstate(fse, 1);
      fse_set_outchar(fse, 'a');
    }
    fse_finish(it);
  }

  /* State 1 */
  {
    fse_clear_instr(it, 4);
    {
      FstStateEntry *fse = fse_get_outgoing(it, 'a');
      fse_set_outstate(fse, 2);
      fse_set_outchar(fse, 'a');
    }
    fse_finish(it);
  }

  /* State 2 */
  {
    fse_clear_instr(it, 4);
    {
      FstStateEntry *fse = fse_get_outgoing(it, 'b');
      fse_set_outstate(fse, 3);
      fse_set_outchar(fse, 'b');
    }
    fse_finish(it);
  }

  /* State 3 */
  {
    fse_clear_instr(it, 4);
    fse_set_final_flags(it);
    {
      FstStateEntry *fse = fse_get_outgoing(it, 'b');
      fse_set_outstate(fse, 4);
      fse_set_outchar(fse, 'b');
    }
    fse_finish(it);
  }

  /* State 3 */
  {
    fse_clear_instr(it, 4);
    {
      FstStateEntry *fse = fse_get_outgoing(it, 'a');
      fse_set_outstate(fse, 3);
      fse_set_outchar(fse, 'a');
    }
    fse_finish(it);
  }

  /* State 4 */
  {
    fse_clear_instr(it, 4);
    fse_finish(it);
  }
}

/**
 * Match a single character on a single state.
 * The FST must be deterministic.
 * @param input the input character
 * @param state_number the number that labels the current state
 * @param states the vector of states
 * @param current_state_start the start of the 256 entries that define the
 * current state
 * @param next_state stores the pointer to the state after this is matched
 */
void match_one_char(char input, char *output, int *state_number,
                    FstStateEntry *states, FstStateEntry *current_state_start,
                    FstStateEntry **next_state) {
  unsigned char uinput = input;
  *output = current_state_start[uinput].components.outchar;
  *state_number = current_state_start[uinput].components.out_state;
  *next_state = states + (*state_number) * 256;
}

/**
 * Returns a malloced (!) string,
 * and sets matched_states to a malloced (!)
 * array of shorts corresponding to what
 * was matched by what state.
 */
char *match_string(const char *input, InstructionTape *instrtape,
                   int *match_success, unsigned short **matched_states) {
  /* Output vector */
  unsigned char *instrbuff = instrtape->beginning;
  char *retval = malloc(sizeof(char) * 10);
  char *string_end = retval;
  int retval_size = 0;
  int retval_capacity = 10;

  /* Matched states */
  /* Size and capacity piggyback off of that of output vector */
  *matched_states = malloc(sizeof(unsigned short) * 10);
  unsigned short *matched_states_end = *matched_states;

  int state_number = 0;
  FstStateEntry *states = (FstStateEntry *) instrbuff;
  FstStateEntry *current_state_start = (FstStateEntry *) instrbuff;
  while (*input) {
    char output;
    match_one_char(*input, &output, &state_number, states, current_state_start,
                   &current_state_start);
    input += 1;
    if (output) {
      if (retval_size == retval_capacity) {
        retval_capacity *= 2;
        retval = realloc(retval, retval_capacity);
        if (!retval) {
          perror("Failed to allocate memory");
          exit(1);
        }
        matched_states = realloc(matched_states, retval_capacity);
        if (!matched_states) {
          perror("Failed to allocate memory");
          exit(1);
        }
      }
      *string_end = output;
      string_end += 1;
      *matched_states_end = state_number;
      matched_states_end += 1;
      retval_size += 1;
    }
  }

  if (retval_size == retval_capacity) {
    retval_capacity *= 2;
    retval = realloc(retval, retval_capacity);
    if (!retval) {
      perror("Failed to allocate memory");
      exit(1);
    }
  }

  *string_end = '\0';
  *matched_states_end = state_number;

  *match_success = !!(current_state_start->components.flags & FST_FLAG_FINAL);
  return retval;
}

static int c_swap(lua_State *L) {
  double arg1 = luaL_checknumber(L, 1);
  double arg2 = luaL_checknumber(L, 2);
  lua_pushnumber(L, arg2);
  lua_pushnumber(L, arg1);
  return 2;
}

static int l_get_instruction_tape(lua_State *L) {
  InstructionTape *it = malloc(sizeof(InstructionTape));
  fse_initialize_tape(it);
  lua_pushlightuserdata(L, (void *) it);
  return 1;
}

static int l_create_pegreg_diffmatch(lua_State *L) {
  InstructionTape *it = (InstructionTape *) lua_touserdata(L, 1);
  create_pegreg_diffmatch(it);
  return 0;
}

static int l_match_string(lua_State *L) {
  const char *input = luaL_checkstring(L, 1);
  InstructionTape *it = (InstructionTape *) lua_touserdata(L, 2);

  int match_success;
  unsigned short *matched_states;
  char *outstr = match_string(input, it, &match_success, &matched_states);

  lua_pushstring(L, outstr);

  lua_pushboolean(L, match_success);

  lua_newtable(L);
  int counter = 1;
  while (*outstr) {
    lua_pushnumber(L, counter);
    lua_pushnumber(L, matched_states[counter - 1]);
    lua_settable(L, -3);
    counter += 1;
    outstr += 1;
  }

  return 3;
}

static int l_instruction_tape_destroy(lua_State *L) {
  InstructionTape *it = (InstructionTape *) lua_touserdata(L, 1);
  instruction_tape_destroy(it);
  free(it);
  return 0;
}

static int l_fse_clear_instr(lua_State *L) {
  InstructionTape *it = (InstructionTape *) lua_touserdata(L, 1);
  int error_state = luaL_checkint(L, 2);
  fse_clear_instr(it, error_state);
  return 0;
}

static int l_fse_set_initial_flags(lua_State *L) {
  InstructionTape *it = (InstructionTape *) lua_touserdata(L, 1);
  fse_set_initial_flags(it);
  return 0;
}

static int l_fse_get_outgoing(lua_State *L) {
  InstructionTape *it = (InstructionTape *) lua_touserdata(L, 1);
  const char *c = luaL_checkstring(L, 2);
  if (strlen(c) != 1) {
    luaL_error(L, "Size of c parameter must be 1");
    return 0;
  }
  FstStateEntry *fse = fse_get_outgoing(it, *c);
  lua_pushlightuserdata(L, (void *) fse);
  return 1;
}

static int l_fse_set_outstate(lua_State *L) {
  FstStateEntry *fse = (FstStateEntry *) lua_touserdata(L, 1);
  int outstate = luaL_checkint(L, 2);
  fse_set_outstate(fse, outstate);
  return 0;
}

static int l_fse_set_outchar(lua_State *L) {
  FstStateEntry *fse = (FstStateEntry *) lua_touserdata(L, 1);
  const char *c = luaL_checkstring(L, 2);
  if (strlen(c) != 1) {
    luaL_error(L, "Size of c parameter must be 1");
    return 0;
  }
  fse_set_outchar(fse, *c);
  return 0;
}

static int l_fse_finish(lua_State *L) {
  InstructionTape *it = (InstructionTape *) lua_touserdata(L, 1);
  fse_finish(it);
  return 0;
}

static int l_fse_set_final_flags(lua_State *L) {
  InstructionTape *it = (InstructionTape *) lua_touserdata(L, 1);
  fse_set_final_flags(it);
  return 0;
}

static const struct luaL_Reg fst_fast_system[] = {
    {"c_swap", c_swap},
    {"get_instruction_tape", l_get_instruction_tape},
    {"create_pegreg_diffmatch", l_create_pegreg_diffmatch},
    {"match_string", l_match_string},
    {"instruction_tape_destroy", l_instruction_tape_destroy},
    {"fse_clear_instr", l_fse_clear_instr},
    {"fse_set_initial_flags", l_fse_set_initial_flags},
    {"fse_get_outgoing", l_fse_get_outgoing},
    {"fse_set_outstate", l_fse_set_outstate},
    {"fse_set_outchar", l_fse_set_outchar},
    {"fse_finish", l_fse_finish},
    {"fse_set_final_flags", l_fse_set_final_flags},
    {NULL, NULL}};

int luaopen_fst_fast_system(lua_State *L) {
  luaL_newlib(L, fst_fast_system);
  return 1;
}
