MODE PROOF

NAME GRP023_2_rm_eq_rstfp_in

ORDERING XKBO
           multiply:1 > identity:1 > inverse:1

EQUATIONS
           multiply(identity,X) = X
           multiply(inverse(X),X) = identity
           multiply(multiply(X,Y),Z) = multiply(X,multiply(Y,Z))
           multiply(X,identity) = X
           multiply(X,inverse(X)) = identity

CONCLUSION inverse(identity) = identity
