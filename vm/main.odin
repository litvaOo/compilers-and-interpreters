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
    make([dynamic]Value, 0),
    0,
    0,
    make(map[string]int),
    make([dynamic]Value, 0),
    make([dynamic]Frame, 0)
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
      fmt.println(pop(&vm.stack))
    case "PRINT":
      fmt.print(pop(&vm.stack))
    case "JSR":
      num_args := int(pop(&vm.stack).(f64))
      base_pointer := len(&vm.stack) - num_args
      new_frame := Frame{vm.pc, base_pointer, args[0]}
      append(&vm.frames, new_frame)
      vm.pc = vm.labels[args[0]] or_else panic("No such function")
    case "RTS":
      res := vm.stack[len(&vm.stack)-1]
      for len(&vm.stack) > vm.frames[len(&vm.frames)-1].frame_pointer {
        pop(&vm.stack)
      }
      append(&vm.stack, res)
      vm.pc = vm.frames[len(&vm.frames)-1].ret_pc
      pop(&vm.frames)
    case "JMP":
      vm.pc = vm.labels[args[0]] or_else panic("No label found")
    case "POP":
      pop(&vm.stack)
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
      assign_at(&vm.globals, strconv.atoi(args[0]), x1)
    case "LOAD_LOCAL":
      offset := 0
      if len(&vm.frames) > 0 {
        offset = vm.frames[len(&vm.frames)-1].frame_pointer
      }
      // fmt.println(vm.stack, offset + strconv.atoi(args[0]))
      append(&vm.stack, vm.stack[strconv.atoi(args[0])+offset])
    case "STORE_LOCAL":
      // fmt.println("Stack is ", vm.stack)
      x1 := pop(&vm.stack)
      // fmt.println("X1 is ", x1)
      offset := 0
      if len(&vm.frames) > 0 {
        offset = vm.frames[len(&vm.frames)-1].frame_pointer
      }
      // fmt.println("offset is ", offset)
      // fmt.println("Assiging at ", strconv.atoi(args[0])+offset)
      assign_at(&vm.stack, strconv.atoi(args[0])+offset, x1)
      // fmt.println("Stack after store is ", vm.stack)
      // fmt.println("=================================")
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
