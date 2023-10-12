# Nano Post Machine

<details>
  <summary>In other languages</summary>

  - [Русский](README.ru.md)
</details>

The Nano Post Machine is a simple digital machine that emulates the functionality of a Post–Turing machine, using Arduino Nano. It is designed to be compact and easy to use, making it suitable for educational purposes and small-scale automation tasks. The machine is controlled using a set of buttons and features a 20x4 LCD screen for displaying the current state of the machine.

The machine operates on a fixed-length tape with configurable cell ("box") values, allowing users to input and manipulate data. It executes a set of predefined instructions, each represented by a single character, and supports basic operations such as reading and writing to the tape, moving the head left or right, and conditional branching. The program can be loaded into the machine's memory, and the execution can be controlled manually or automatically.

The machine is open-source, allowing users to customize and extend its functionality to suit their specific needs. The Nano Post Machine offers a simple yet powerful platform for exploring the principles of computational machines and experimenting with basic automation tasks.

## Instructions

All jump addresses are program code addresses, starting from 0 as the first instruction. A program can contain up to 256 instructions.

| Instruction | 2 operands | Description |
| :-- | :-- | :-- |
| `1` | jump address, unused | Marking the box it is in, if it is empty |
| `0` | jump address, unused | Erasing the mark in the box it is in, if it is marked |
| `>` | jump address, unused | Moving to the box on its right |
| `<` | jump address, unused | Moving to the box on its left |
| `?` | Jump addresses: go to first jump address if the cell ("box") is marked, otherwise go to the second jump address | Determining whether the box it is in, is or is not marked |
| `.` | unused, unused | Terminate the program |

### Example

```cpp
const Instruction program[] = {
  {'?', 1, 3}, // "?2,4"
  {'1', 2, 0}, // "1"
  {'>', 0, 0}, // ">1"
  {'.', 0, 0}  // "."
};
```

## Current features

- non-programmable (a program is hard coded)
- running the hard coded program
- pausing execution
- changing tape data before running the program and during a pause
- ability to zero the tape
- saving/loading the tape to RAM of the microcontroller
- when stop button is pressed, returns to the initial head position
- simple sound accompaniment as a feedback

## Planned features

- programmability (keypad may be used) with [Konstantin Polyakov](https://kpolyakov.spb.ru/prog/post.htm)-like syntax
- showing listing of a program with vertical scrolling
- step-by-step execution
- use of multi-button press to call menus
- tape scrolling (breaking the limit of column width of a display) with in-place scrollbar: using arrows for each side to show if there is more tape
- improved user interface
- documentation

## Sources
- https://en.wikipedia.org/wiki/Post–Turing_machine: Used the paragraph "1936: Post model" to write [descriptions for instructions](#instructions)
