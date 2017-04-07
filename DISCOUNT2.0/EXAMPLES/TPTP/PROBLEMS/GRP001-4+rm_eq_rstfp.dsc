MODE PROOF

NAME GRP001_4_rm_eq_rstfp_in

ORDERING XKBO
           multiply:1 > identity:1 > a:1 > b:1 > c:1

EQUATIONS
           multiply(multiply(X,Y),Z) = multiply(X,multiply(Y,Z))
           multiply(identity,X) = X
           multiply(X,X) = identity
           multiply(a,b) = c

CONCLUSION multiply(b,a) = c
