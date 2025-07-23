# Mathematical Expression Calculator

## Description
This project is a console-based calculator that evaluates mathematical expressions. The calculator supports basic arithmetic operations, trigonometric functions, logarithms, and other mathematical operations.

## Features
- Basic operations: +, -, *, /

- Unary minus (e.g., -5)

- Trigonometric functions: sin, cos, tan, ctg

- Logarithms and roots: ln, sqrt

- Parentheses for operation precedence

- Error handling (syntax errors, math errors)

- Conversion to Reverse Polish Notation (RPN)

## Requirements
- C compiler (gcc, clang, etc.)

- Standard C library (including math.h)

## Usage
Run the program and enter mathematical expressions. Type q to exit.

## Examples:

text
> 2 + 2 * 2

Result: 6.000000

> sin(0.5) + cos(0.5)

Result: 1.357008

> sqrt(9) + ln(1)

Result: 3.000000

## Expression Format
The program understands the following elements:

- Numbers: integers (5) and decimals (3.14)

- Operators: +, -, *, /

- Functions: sin(x), cos(x), tan(x), ctg(x), ln(x), sqrt(x)

- Parentheses: (, )

## Error Handling
The program reports different error types:

- Error: Invalid expression - syntax error

- Error: Math error - math error (division by zero, etc.)

## Limitations
- Maximum expression length: 255 characters

- Maximum token length: 15 characters

- Function names are case-sensitive (sin is valid, SIN is not)

## Implementation Details
The calculator works in several stages:

- Tokenization - breaks input into tokens

- Validation - checks expression syntax

- Conversion to RPN - transforms to postfix notation

- Evaluation - calculates the result