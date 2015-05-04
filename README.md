# Compiler Final Hand-in
This is my final hand-in for compiler optimization.  Below is a detailed
analysis of the performance of my compiler and what cases my optimizations
work under.

## Benchmarks
Below is the output of my profiling script.  This can be run with
`./build-all.sh profile "$CFLAGS"`.  There are two different tables
because the cheating optimization removes so many instructions, the
other optimizations are unable to do anything.

### Without Cheating Enabled (i.e. `-p -t -i`)
program       |  before   | after     | nops after | real after | pct gone  
--------------|-----------|-----------|------------|------------|------------
blank         | 65       |  40       | 3         | 37         | 43.08
block         | 65       |  40       | 3         | 37         | 43.08
call          | 83       |  59       | 6         | 53         | 36.14
fact_rec      | 12320    |  80       | 12        | 68         | 99.45
fact_tr       | 13535    |  9925     | 2694      | 7231       | 46.58
fib_rec       | 59906869 |  811      | 180       | 631        | 100.00
fib_tr        | 1455     |  1127     | 255       | 872        | 40.07
for_test      | 1224       |  1224     | 252       | 972        | 0.00
hello         | 36       |  22       | 0         | 22         | 38.89
hi            | 28       |  19       | 0         | 19         | 32.14
inline        | 1023     |  663      | 141       | 522        | 48.97
rec           | 208      |  95       | 12        | 83         | 60.10
RegisterSetup | 77       |  51       | 0         | 51         | 33.77
ret           | 39       |  31       | 0         | 31         | 20.51
sample        | 73       |  43       | 3         | 40         | 45.21
selsort       | 4734     |  2697     | 546       | 2151       | 54.56
tailcall      | 304057   |  160042   | 48003     | 112039     | 63.15
while_in_while| 2403       |  2249     | 711       | 1538       | 6.41
while         | 406       |  323      | 90        | 233        | 19.85

### With Cheating Enabled (i.e. `-p -t -i -c`)
program         | before     | after      |nops after |real after |pct gone  
----------------|-------------|------------|-----------|----------|---------------
blank           | 65         | 3          | 0          | 3          | 95.38
block           | 65         | 3          | 0          | 3          | 95.38
call            | 83         | 3          | 0          | 3          | 96.39
fact_rec        | 12320      | 3          | 0          | 3          | 99.98
fact_tr         | 13535      | 3          | 0          | 3          | 99.98
fib_rec         | 59906869   | 3          | 0          | 3          | 100.00
fib_tr          | 1455       | 3          | 0          | 3          | 99.79
for_test        | 1224         | 51         | 0          | 51         | 95.83
hello           | 36         | 3          | 0          | 3          | 91.67
hi              | 28         | 3          | 0          | 3          | 89.29
inline          | 1023       | 5          | 0          | 5          | 99.51
rec             | 208        | 5          | 0          | 5          | 97.60
RegisterSetup   | 77         | 3          | 0          | 3          | 96.10
ret             | 39         | 3          | 0          | 3          | 92.31
sample          | 73         | 3          | 0          | 3          | 95.89
selsort         | 4734       | 5          | 0          | 5          | 99.89
tailcall        | 304057     | 3          | 0          | 3          | 100.00
while_in_while  | 2403         | 29         | 0          | 29         | 98.80
while           | 406         | 15         | 0          | 15         | 96.31


## Enabling Optimizations
To enable each of the optimizations add the flag separated by a space.  For
example, if you want to enable all optimizations write

```BASH
./compiler/cm -p -c -t -i <$FILE.cm >$FILE.tm
```

## `-p` Peephole Optimization
Looks at the instruction output and attempts to find combinations of
instructions that can be simplified.  My optimizer only replaces the
instructions with NOPs.  Every instance listed below is identified and
optimized inside `./compiler/peephole.c`.  To enable peepholing, add
`-p` to the compiler flags.

### The peephole patterns I identified and optimized were:
1. Load and immediately store the same register.
2. Store then immediately load the same register.
3. Store the same register twice.
4. Load the same register twice.
5. Add then immediately subtract one of the two numbers added.
6. Subtract then immediately add the number subtracted.
7. Multiply then immediately divide one of the numbers multiplied.
8. Divide then immediately multiply the divisor.

### The redundant PUSH/POP combinations optimized were:
1. Pushing a register to the stack, not modifying it, and then popping
   into the same register.
2. Pushing a register to the stack and popping it into a different register
   only if that register isn't modified over the duration that the original
   variable is on the stack.

## `-i` Inlining Optimizations
Inlining is taking a funciton call and pasting the actual function code
in its place.  Below is a list of reasons to not inline a function, this
means any function without any of these critera is going to be inlined.  
This also means that there is a system in the compiler to resolve function
locals that are inside inlined portions of code. To enable inline, add
`-i` to the compiler flags.

### Reasons to not inline function:
1. Function is named `main`
2. Return statements not at the very end of execution (i.e. inside an if)

### Reasons to not inline a call:
1. Call is in the expression of an if statement.
2. Call is in the expression of a while loop.
3. Call is inside the initializer or expression of a for-loop.
4. Call is in an arithmetic expression.
5. Call is inside arguments for another call.

## `-t` Tree-based Optimizations
There are two different tree-based optimizations: constant folding and
the tail call optimization.  They are both turned on with the `-t` flag.

### Constant Folding
Constant folding works on three cases below.  These cases bubble out,
allowing for complex expressions like `3 * 4 - 5 * 9` to be simplified.

#### Constant folding cases:
1. Adding / subtracting two constants together.
2. Multiplying / dividing two constants together. *
3. Parenthesis around constant expressions. *

### Tail Call Optimization
Executes a called function in the current scope, clobbering all locals.
Only the first of the cases is actually optimized but all three cases
are profiled.

#### Tail Call Cases:
1. Simple: Call in a return statement (i.e. `return call()`)
2. Inline: Math expressions on the same line as (i.e. `return call() * 3`).
3. Arithmetic: More complicated combinations on same line
   (i.e. `return call() * call()`).

## `-c` Cheating Optimizations
The cheating optimization compiles the program once, runs it to check for
any output result, and then emits OUT instructions that emulate the result.
This drastically reduces the number of instructions run but can be
considered cheating since the "slower" program is run during compilation.
The only case when this optimization doesn't work is if `input()` is called.
Since this takes user input, the output will be unknown at compile time.
To run this properly, you must run the compiler in the root directory of
the project and not in the `compiler/` folder.

## Etc. Features
Here is a small list of extra features / optimizations that I added or
started optimizing.

### Register Allocator
I started implementing a graph-based register allocator
(see [this pdf](http://web.cecs.pdx.edu/~mperkows/temp/register-allocation.pdf))
but never had time to finish it.  It is present in the `regalloc` branch
in the project repository.  As part of this optimization, I wrote a graph library, and dataflow analyzer.  I got all the way up to debugging the
dataflow analyzer.

### For-loop
I also added a for-loop into the compiler.  It behaves like the C89 standard.
There is a handout program with an example.

### While loop optimization
Added the flowchart optimization for the while loop as part of an activity.
It is hard-coded to be enabled always.

### Floating point
There is also optional floating point support enabled with the `-f` flag.
Note: optimizaitons no longer work when floating point is enabled, the
compiler is broken when any are turned on with floating point support.
