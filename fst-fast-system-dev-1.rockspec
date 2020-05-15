rockspec_format = "3.0"

package = "fst-fast-system"
version = "dev-1"
source = {
   url = "*** please add URL for source tarball, zip or repository here ***"
}
description = {
   homepage = "*** please enter a project homepage ***",
   license = "MIT License"
}
build = {
   type = "builtin",
   modules = {
           fst_fast = "src/fst_fast.c"
   }
}

test_dependencies = {
   "luaunit"
}

test = {
   type = "command",
   script = "test/test.lua"
}
