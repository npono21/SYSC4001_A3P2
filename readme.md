# SYSC 4001 - Operating Systems - Assignment 3 Part 2

### Student: Nicholai Ponomarev 101182048

### Student: Arshiya Moallem 101324189

## Description

There is a variable number of Teaching Assistants (TA) assigned to mark exams. When they meet to mark,
they use a pile of exams written on paper.

There are two sets of documents, as in the figure below: (a) the pile of exams, and (b) an individual file
with the exam rubric. All the TAs can read the rubric concurrently, but if a TA detects a problem in the
rubric, and it needs to be changed, only one TA can modify the rubric at a given time. Exams can be read
or written by any TA at any time.

The TAs have two tasks at hand: they first pick an exam. Then, they check the rubric, and mark only one
exercise in the exam. Then, the TA writes the mark for that student in the exam file. If in the process of
marking an error is detected in the rubric, the TA modifies the rubric. All the TAs can be marking their
exams concurrently.

(Retrieved from Assignment Instructions)

## Getting Started

### Executing program

- Ensure you are in the main (parent) directory and execute the following
- 'number of TAs' is a command line argument supplied by you, it can be from minimum 2 to however many you want
- it cannot be negative, nor less than 2, otherwise, it will default to 2

```
gcc main.c -o main && ./main <number of TAs>
```

## Version History

- 0.1
  - Initial Version

## Acknowledgments

Assignment Instructions

- [SYSC 4001 Brightspace](https://brightspace.carleton.ca/d2l/le/content/372590/viewContent/4495273/View)
