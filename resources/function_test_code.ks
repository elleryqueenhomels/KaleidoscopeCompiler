extern printd(x)

def fibonacci(x)
    if x < 3 then
        1
    else
        fibonacci(x - 1) + fibonacci(x - 2)
    end
end

def test()
    for i = 1, i <= 10, 1 in
        printd(fibonacci(i))
    end
end

test()

# function declare (`extern`) or define (`def`) supports
# explicitly or implicitly using `,` to split multiple arguments
# following are all valid:
extern test1(a b c d)
extern test2(a,b,c,d)
extern test3(a, b, c, d)
extern test4(a, b c d)
extern test5(a, b c, d)

def sum(left, right)
    sum = 0
    for num = left, num <= right, 1 in
        sum = sum + num
    end
    sum
end

sum(1, 6)   # return 21
sum(1, 100) # return 5050
