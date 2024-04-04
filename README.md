# Nano Post Machine

<details>
  <summary>Languages</summary>

  - [Русский](README.ru.md)
</details>

The Nano Post Machine is a simple digital machine that emulates the functionality of a Post–Turing machine, using Arduino Nano. It is designed to be compact and easy to use, making it suitable for educational purposes and small-scale automation tasks. The machine is controlled using a set of buttons and features a 20x4 LCD screen for displaying the current state of the machine.

The machine operates on a fixed-length tape with configurable cell values, allowing users to input and manipulate data. It executes a set of predefined instructions, each represented by a single character, and supports basic operations such as reading and writing to the tape, moving the head left or right, and conditional branching. The program can be loaded into the machine's memory, and the execution can be controlled manually or automatically.

The machine is open-source, allowing users to customize and extend its functionality to suit their specific needs. The Nano Post Machine offers a simple yet powerful platform for exploring the principles of computational machines and experimenting with basic automation tasks.

## Instructions

All jump addresses are program code addresses, starting from 0 as the first instruction. A program can contain up to 256 instructions.

| Instruction | 2 operands | Description |
| :-- | :-- | :-- |
| `1` | jump address, unused | Marking the cell pointed to by the head if it is empty |
| `0` | jump address, unused | Erasing the mark in the cell pointed to by the head, if it is marked |
| `>` | jump address, unused | Moving the head to the cell on the right of it |
| `<` | jump address, unused | Moving the head to the cell to the left of it |
| `?` | Jump addresses: go to first jump address if the cell is marked, otherwise go to the second jump address | Determining whether the cell pointed to by the head is marked or not |
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

- non-programmable (program is hard-coded)
- start hard-coded program
- pause execution
- changing data on the tape before program execution and during a pause
- tape zeroing option
- saving/loading the tape into the microcontroller RAM
- return to the initial position of the head when the "Stop" button is pressed
- simple audio as feedback

## Planned features

- programmability (maybe a numeric keyboard will be used) with a syntax similar to [Konstantin Polyakov](https://kpolyakov.spb.ru/prog/post.htm)'s
- program listing with vertical scrolling
- step-by-step execution
- using multiple button presses to invoke menus
- scrolling of the tape (overcoming display column width limitation) with a scroll bar in place: arrows for each side show if there is more tape
- improved user interface
- documentation

## Sources
- https://en.wikipedia.org/wiki/Post–Turing_machine: Used the paragraph "1936: Post model" to write [descriptions for instructions](#instructions)
