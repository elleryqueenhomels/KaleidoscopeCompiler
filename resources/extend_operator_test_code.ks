def binary|| 5 (LHS RHS)
  if LHS then
    1
  else if RHS then
    1
  else
    0

def binary&& 5 (LHS RHS)
  if LHS then
    if RHS then
      1
    else
      0
  else
    0

def test(x y z)
    if x < y && y > z then
        y
    else if x + y <= z then
        z
    else if x > y || x > z then
        x + y + z
    else
        x

test(1, 2, 1) # return 2
test(1, 2, 4) # return 4
test(5, 4, 4) # return 13
test(6, 7, 8) # return 6
