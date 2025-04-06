package vm

import "core:fmt"
import "core:os"
import "core:strings"
import "core:strconv"
import "core:math"
import "base:runtime"

Value :: union {
  f64, string, bool
}

VM :: struct {
  stack: [dynamic]Value,
  pc: int,
  sp: int,
  labels: map[string]int,
  globals: [dynamic]Value,
  frames: [dynamic]Frame
}

Frame :: struct {
  ret_pc: int,
  frame_pointer: int,
  name: string
}

execute::proc(instructions: []string) -> runtime.Allocator_Error {
  vm := VM{
    make([dynamic]Value, 1024),
    0,
    0,
    make(map[string]int),
    make([dynamic]Value, 1024),
    make([dynamic]Frame, 1024)
  }
  label_scan: for {
    instruction := strings.split(strings.trim_space(instructions[vm.pc]), " ") or_return
    vm.pc += 1
    opcode := instruction[0]
    if opcode == "LABEL" {
      vm.labels[instruction[1][:len(instruction[1])-1]] = vm.pc
    }
    if opcode == "HALT" {
      break label_scan
    }
  }
  vm.pc = 0
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
      var, ok := strconv.parse_f64(args[0])
      if ok {
        append(&vm.stack, var)
      }
      else {
        append(&vm.stack, strings.concatenate(args))
      }
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
    case "LE":
      x1 := pop(&vm.stack)
      x2 := pop(&vm.stack)
      append(&vm.stack, (x2.(f64) <= x1.(f64)))
    case "GE":
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
    case "JSR":
      new_frame := Frame{vm.pc, len(&vm.stack)-1, args[0]}
      append(&vm.frames, new_frame)
      fmt.println(args[0])
      vm.pc = vm.labels[args[0]] or_else panic("No such function")
    case "RTS":
      vm.pc = vm.frames[len(&vm.frames)-1].ret_pc
      pop(&vm.frames)
    case "JMP":
      vm.pc = vm.labels[args[0]] or_else panic("No label found")
    case "JMPZ":
      x1 := pop(&vm.stack)
      #partial switch v in x1 {
      case bool:
        if x1.(bool) == false {
          vm.pc = vm.labels[args[0]] or_else panic("No label found")
        }
      case f64:
        if x1.(f64) == 0 {
          vm.pc = vm.labels[args[0]] or_else panic("No label found")
        }

      }
    case "LOAD_GLOBAL":
      append(&vm.stack, vm.globals[strconv.atoi(args[0])])
    case "STORE_GLOBAL":
      x1 := pop(&vm.stack)
      vm.globals[strconv.atoi(args[0])] = x1
    case "LOAD_LOCAL":
      append(&vm.stack, vm.stack[strconv.atoi(args[0])])
    case "STORE_LOCAL":
      x1 := pop(&vm.stack)
      vm.stack[strconv.atoi(args[0])] = x1
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
