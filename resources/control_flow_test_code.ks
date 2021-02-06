extern printd(x)

def foo(x)
    if x < 3 then
        1
    else
        foo(x - 1) + foo(x - 2)

for i = 1, i < 10, 1.0 in 
    printd(foo(i))
