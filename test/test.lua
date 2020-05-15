local luaunit = require("luaunit")
local fst_fast = require("fst_fast")

-- Simplest case

function testFromC()
   local instruction_tape = fst_fast.get_instruction_tape()

   fst_fast.create_pegreg_diffmatch(instruction_tape)

   local outstr, match_success, matched_states = fst_fast.match_string("aax", instruction_tape);

   fst_fast.instruction_tape_destroy(instruction_tape)

   luaunit.assertTrue(match_success)

   luaunit.assertEquals(outstr, "aax")

   luaunit.assertEquals(matched_states, {1, 2, 3})

end




function testFromLua()
   --- Create the same little guy

   local instrtape = fst_fast.get_instruction_tape()

   -- State 0
   fst_fast.fse_clear_instr(instrtape, 6)
   fst_fast.fse_set_initial_flags(instrtape)

   local fse = fst_fast.fse_get_outgoing(instrtape, 'a')
   fst_fast.fse_set_outstate(fse, 1)
   fst_fast.fse_set_outchar(fse, 'a')

   fst_fast.fse_finish(instrtape)

   -- State 1
   fst_fast.fse_clear_instr(instrtape, 6)

   local fse = fst_fast.fse_get_outgoing(instrtape, 'a')
   fst_fast.fse_set_outstate(fse, 2)
   fst_fast.fse_set_outchar(fse, 'a')


   local fse = fst_fast.fse_get_outgoing(instrtape, 'b')
   fst_fast.fse_set_outstate(fse, 4)
   fst_fast.fse_set_outchar(fse, 'b')

   fst_fast.fse_finish(instrtape)

   -- State 2
   fst_fast.fse_clear_instr(instrtape, 6);

   local fse = fst_fast.fse_get_outgoing(instrtape, 'x')
   fst_fast.fse_set_outstate(fse, 3)
   fst_fast.fse_set_outchar(fse, 'x')

   fst_fast.fse_finish(instrtape)

   -- State 3
   fst_fast.fse_clear_instr(instrtape, 6)
   fst_fast.fse_set_final_flags(instrtape)

   fst_fast.fse_finish(instrtape)


   -- State 4
   fst_fast.fse_clear_instr(instrtape, 6)

   local fse = fst_fast.fse_get_outgoing(instrtape, 'x')
   fst_fast.fse_set_outstate(fse, 5)
   fst_fast.fse_set_outchar(fse, 'x')

   fst_fast.fse_finish(instrtape)


   -- State 5
   fst_fast.fse_clear_instr(instrtape, 6)
   fst_fast.fse_set_final_flags(instrtape)

   fst_fast.fse_finish(instrtape)


   -- State 6
   fst_fast.fse_clear_instr(instrtape, 6)

   fst_fast.fse_finish(instrtape)

   local outstr, match_success, matched_states = fst_fast.match_string("aax", instrtape)

   luaunit.assertTrue(match_success)

   luaunit.assertEquals(outstr, "aax")

   luaunit.assertEquals(matched_states, {1, 2, 3})

   fst_fast.instruction_tape_destroy(instrtape)
end

os.exit(luaunit.LuaUnit.run())
