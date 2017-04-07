MODE PROOF

NAME GRP010_4_rm_eq_rstfp_in

ORDERING XKBO
           multiply:1 > identity:1 > b:1 > c:1 > inverse:1

EQUATIONS
           multiply(multiply(X,Y),Z) = multiply(X,multiply(Y,Z))
           multiply(identity,X) = X
           multiply(inverse(X),X) = identity
           multiply(c,b) = identity

CONCLUSION multiply(b,c) = identity
