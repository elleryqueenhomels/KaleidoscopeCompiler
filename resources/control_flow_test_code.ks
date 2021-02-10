extern printd(x)

def foo(x)
    if x < 3 then
        1
    else
        foo(x - 1) + foo(x - 2)
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
