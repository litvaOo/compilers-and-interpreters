package vm

import "core:fmt"
import "core:os"
import "core:strings"
import "base:runtime"

VM :: struct {
  stack: []int,
  pc: int
}

execute::proc(instructions: []string) {
  for it in instructions {
    res, alloced := strings.replace(it, "\t", "", -1)
    fmt.println(res)
  }
}

main::proc() {
  filepath := "code.vm"
  code, ok := os.read_entire_file(filepath, context.allocator)
  if !ok {
    fmt.eprintln("Failed to read entire file")
    return
  }
  defer delete(code, context.allocator)
  instructions, error := strings.split(string(code), "\n", context.allocator)
  if error != runtime.Allocator_Error.None {
    fmt.eprintln("Failed to split into instructions")
    return
  }
  execute(instructions)
  return
}
