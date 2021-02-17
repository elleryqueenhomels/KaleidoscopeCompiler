# KaleidoscopeCompiler
A Compiler for Kaleidoscope based on LLVM

## Prerequisites
- C++ 17
- LLVM v11

## Compiler Features
- JIT inside
- Optimizer supported

## Kaleidoscope Code Sample
```
# support external function declare
# `printd` is a function defined in compiler
# we can declare it then invoke it within our code
extern printd(x)

# function definition
def fibonacci(x)
    if x < 3 then
        1
    else
        fibonacci(x - 1) + fibonacci(x - 2)
    end
end

# since we have already used `extern` to declare `printd` above
# now we can directly call it within our code
def test()
    for i = 1, i <= 10, 1 in
        printd(fibonacci(i))
    end
end

# call `test()`, it should print out the first 10 fibonacci terms
test()

# define a function to calculate sum
def sum(left, right)
    sum = 0
    for num = left, num <= right, 1 in
        sum = sum + num
    end
    sum
end

# support user-defined binary operator
def binary -> 1 (LHS RHS)
    RHS
end

# support user-defined unary operator
def unary $ (x)
    sum(1, x)
end

# use the self-defined operator, it should return 735078
    x = sum(1, 10) + fibonacci(6)
->  y = $50 - x
->  $y
```

## How to Use
- Install Prerequisites
- Build Kaleidoscope Compiler: `bash build-jit.sh`
- Use the compiler built above to compile your Kaleidoscope script: `./ksc-jit.app < your-script.ks`
