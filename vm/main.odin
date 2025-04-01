package vm

import "core:fmt"
import "core:os"
import "core:strings"
import "core:strconv"
import "core:math"
import "base:runtime"

Value :: union {
  f64, string
}

VM :: struct {
  stack: [dynamic]Value,
  pc: int,
  sp: int,
}

execute::proc(instructions: []string) -> runtime.Allocator_Error {
  vm := VM{
    make([dynamic]Value, 1024),
    0,
    0,
  }
  exec: for {
    instruction := strings.split(strings.trim_space(instructions[vm.pc]), " ") or_return
    vm.pc += 1
    opcode := instruction[0]
    args : []string
    if len(instruction) > 1 {
      args = instruction[1:]
    }
    switch opcode {
    case "HALT":
      break exec
    case "PUSH":
      append(&vm.stack, strconv.atof(args[0]))
    case "START":
    case "MUL":
      x1 := pop(&vm.stack)
      x2 := pop(&vm.stack)
      append(&vm.stack, x1.(f64)*x2.(f64))
    case "DIV":
      x1 := pop(&vm.stack)
      x2 := pop(&vm.stack)
      append(&vm.stack, x2.(f64)/x1.(f64))
    case "NEG":
      x1 := pop(&vm.stack)
      append(&vm.stack, x1.(f64)*-1)
    case "NE":
      x1 := pop(&vm.stack)
      x2 := pop(&vm.stack)
      append(&vm.stack, (x2 != x1))
    case "EQ":
      x1 := pop(&vm.stack)
      x2 := pop(&vm.stack)
      append(&vm.stack, (x2 == x1))
    case "LT":
      x1 := pop(&vm.stack)
      x2 := pop(&vm.stack)
      append(&vm.stack, (x2.(f64) < x1.(f64)))
    case "GT":
      x1 := pop(&vm.stack)
      x2 := pop(&vm.stack)
      append(&vm.stack, (x2.(f64) > x1.(f64)))
    case "LTE":
      x1 := pop(&vm.stack)
      x2 := pop(&vm.stack)
      append(&vm.stack, (x2.(f64) <= x1.(f64)))
    case "GTE":
      x1 := pop(&vm.stack)
      x2 := pop(&vm.stack)
      append(&vm.stack, (x2.(f64) >= x1.(f64)))
    case "ADD":
      x1 := pop(&vm.stack)
      x2 := pop(&vm.stack)
      append(&vm.stack, x1.(f64)+x2.(f64))
    case "SUB":
      x1 := pop(&vm.stack)
      x2 := pop(&vm.stack)
      append(&vm.stack, x2.(f64)-x1.(f64))
    case "EXP":
      x1 := pop(&vm.stack)
      x2 := pop(&vm.stack)
      append(&vm.stack, math.pow(x2.(f64), x1.(f64)))
    case "MOD":
      x1 := pop(&vm.stack)
      x2 := pop(&vm.stack)
      append(&vm.stack, math.remainder_f64(x2.(f64), x1.(f64)))
    case "AND":
      x1 := pop(&vm.stack)
      x2 := pop(&vm.stack)
      append(&vm.stack, f64(int(x2.(f64)) ~ int(x1.(f64))))
    case "OR":
      x1 := pop(&vm.stack)
      x2 := pop(&vm.stack)
      append(&vm.stack, f64(int(x2.(f64)) ~ int(x1.(f64))))
    case "XOR":
      x1 := pop(&vm.stack)
      x2 := pop(&vm.stack)
      append(&vm.stack, f64(int(x2.(f64)) ~ int(x1.(f64))))
    case "PRINTLN":
      fmt.println(vm.stack[len(vm.stack)-1])
    case "PRINT":
      fmt.print(vm.stack[len(vm.stack)-1])
    }
  }
  return nil
}

main::proc() {
  filepath := "code.vm"
  code, ok := os.read_entire_file(filepath, context.allocator)
  if !ok {
    fmt.eprintln("Failed to read entire file")
    return
  }
  defer delete(code, context.allocator)
  instructions, error := strings.split_lines(string(code), context.allocator)
  if error != runtime.Allocator_Error.None {
    fmt.eprintln("Failed to split into instructions")
    return
  }
  execute(instructions)
  return
}
