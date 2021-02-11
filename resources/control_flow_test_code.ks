extern printd(x)

def foo(x)
    if x < 3 then
        1
    else
        foo(x - 1) + foo(x - 2)
    end
end

for i = 1, i < 10, 1.0 in
    printd(foo(i))
end

for i = 1, i < 10, 1.0 in
    for j = 1, j < 5, 1.0 in
        for k = 1, k < 3, 1.0 in
            printd(k)
        end
    end
end

for i = 10, i > 0, -1 in
    printd(i)
end

def for_test()
    sum = 0
    for i = 10, i > 0, -1 in
        sum = sum + i
        printd(sum)
    end
    sum
end

def if_test(x)
    if x > 10 then
        if x > 15 then
            x = x + 15
            printd(x)
        else
            x = x + 10
            printd(x)
        end
    else
        if x > 5 then
            x = x + 5
            printd(x)
        else
            x = x + 1
            printd(x)
        end
    end
    x
end

for_test()   # return 55
if_test(18)  # return 33
if_test(12)  # return 22
if_test(7)   # return 12
if_test(2)   # return 3
