def foo(x)
    if x < 3 then
        1
    else
        foo(x - 1) + foo(x - 2)

foo(1)
foo(2)
foo(3)
foo(4)
foo(5)
