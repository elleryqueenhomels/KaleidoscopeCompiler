def binary** 60 (LHS RHS)
  LHS * RHS + LHS + RHS

7 + 6 ** (5 + 7 * 2)

5.0 == 5.0
5.0 == 5.0001
3.0 != 3.0
3.0 != 3.0001
2.0 >= 2.0
2.0 >= 1.9
2.0 <= 2.0
2.0 <= 2.1

def test(x y z)
  if x < y && y > z then
    y
  else
    if x < y || x < z then
      x
    else
      z
    end
  end

test(1, 3, 2) # return 3
test(1, 2, 2) # return 1
test(5, 4, 2) # return 2

# test unary operator: `-`
(-5) * 3 * (-2) * (-4)
2 + 7 * (-11)

def test(x y z)
  if !(x < y && y > z) then
    y
  else
    if !(x > y || x < z) then
      x
    else
      z
    end
  end

test(5, 4, 2)  # return 4
test(3, 4, 1)  # return 3
test(1, 3, 2)  # return 2
test(-5,-1,-2) # return -2

!(-5 >= 2 && -4 >= 3)

def unary%% (x)
  x * x

%%7 + %%9 # return 130
%%7 - %%9 # return -32

6 * -5 + 7
6 * !1 + 0


def fibo(n)
  if n < 3 then
    1
  else
    fibo(n - 1) + fibo(n - 2)
  end

def unary$ (x)
  fibo(x)

$6
