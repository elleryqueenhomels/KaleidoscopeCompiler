# self-defined binary operator `|`, precedence is 5
def binary| 5 (LHS RHS)
  if LHS then
    1
  else
    if RHS then
      1
    else
      0
    end
  end

1 | 0
0 | 1
0 | 0
1 | 1

# self-defined binary operator `&`, precedence is 5
def binary& 5 (LHS RHS)
  if LHS then
    if RHS then
      1
    else
      0
    end
  else
    0
  end

1 & 1
1 & 0
0 & 1
0 & 0

# self-defined binary operator `=`, precedence is 9
def binary= 9 (LHS RHS)
  if LHS < RHS then
    0
  else
    if LHS > RHS then
      0
    else
      1
    end
  end

5.0 = 5.0
5.0 = 5.00001
2.3 = 2.3
2.3 = 2.31
