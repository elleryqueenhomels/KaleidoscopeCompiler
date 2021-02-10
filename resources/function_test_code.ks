def sum(left right)
    sum = 0
    for num = left, num <= right, 1 in
        sum = sum + num
    end
    sum
end

sum(1, 6)   # return 21
sum(1, 100) # return 5050
