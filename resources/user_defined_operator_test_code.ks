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
end

1 & 1
1 & 0
0 & 1
0 & 0
