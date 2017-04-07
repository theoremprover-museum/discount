MODE PROOF

NAME GRP011_4_rm_eq_rstfp_in

ORDERING XKBO
           multiply:1 > identity:1 > b:1 > c:1 > inverse:1 > d:1

EQUATIONS
           multiply(multiply(X,Y),Z) = multiply(X,multiply(Y,Z))
           multiply(identity,X) = X
           multiply(inverse(X),X) = identity
           multiply(b,c) = multiply(d,c)

CONCLUSION b = d
