# define function to calulate fibonacci
def fib(x)
    if x < 3 then
        1
    else
        fib(x - 1) + fib(x - 2)

fib(40)

# function declare
extern sin(arg)
extern cos(arg)
extern atan2(arg1 arg2)

# declared function can be invoked
atan2(sin(.4), cos(42))
