rockspec_format = "3.0"

package = "fst-fast-system"
version = "dev-1"
source = {
   url = "git+https://github.com/jsalzbergedu/fst-fast-system.git"
}
description = {
   homepage = "https://github.com/jsalzbergedu/fst-fast-system",
   license = "MIT License"
}
build = {
   type = "builtin",
   modules = {
           fst_fast_system = "src/fst_fast.c"
   }
}
test_dependencies = {
   "luaunit >= 3"
}
test = {
   type = "command",
   script = "test/test.lua"
}
