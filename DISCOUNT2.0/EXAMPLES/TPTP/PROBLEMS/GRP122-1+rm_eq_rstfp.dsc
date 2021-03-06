MODE PROOF

NAME GRP122_1_rm_eq_rstfp_in

ORDERING XKBO
           multiply:1 > identity:1 > a:1 > b:1 > c:1

EQUATIONS
           multiply(Y,multiply(multiply(Y,multiply(multiply(Y,Y),multiply(X,Z))),multiply(Z,multiply(Z,Z)))) = X
           multiply(identity,identity) = identity

CONCLUSION multiply(multiply(a,b),c) = multiply(a,multiply(b,c))
