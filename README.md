# Plaid Shell (plaidsh)

Plaid Shell is a custom Linux shell program implemented in C. 

## Features

- Custom tokenizer for parsing input 
- Supports pipelines using |
- Input/output redirection with < and >
- Built-in commands:
  - exit
  - quit
  - author
  - cd
  - pwd
- Executes other programs as child processes
- Displays error messages for failed child processes
- Uses readline for interactive editing and history

## Building

To build plaidsh:

```
make all
```

This compiles the source code and generates the plaidsh executable.

## Running

To run plaidsh after building:

```
./plaidsh
```

This will start the interactive plaid shell. Type `exit` or `quit` to exit.

## Testing

An automated test suite is included to validate the functionality. To run:

```
./ps_test
```

## Clean

Run the following command on the terminal to clean up the program generated files.

```
make clean
```

## Implementation

Plaid shell is implemented in C using concepts like:

- Process creation and management (fork, exec)
- Piping
- Signals
- Abstract syntax trees

The code is split across modules for tokenization, parsing, evaluation etc.

## Acknowledgement

This assignment drew heavy inspiration from the amazing work done by exceptional programmers on various flavors of what we know as a shell program today(i.e. ksh, bash, csh etc).

Many thanks to my instructor, @howdypierce and TA @tsega-1 for their exceptional tutorship and mentorship.