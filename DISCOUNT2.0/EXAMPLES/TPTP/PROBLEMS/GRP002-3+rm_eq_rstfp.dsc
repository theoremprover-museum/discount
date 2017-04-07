MODE PROOF

NAME GRP002_3_rm_eq_rstfp_in

ORDERING XKBO
           multiply:1 > commutator:1 > identity:1 > inverse:1 > a:1 > b:1

EQUATIONS
           multiply(identity,X) = X
           multiply(inverse(X),X) = identity
           multiply(multiply(X,Y),Z) = multiply(X,multiply(Y,Z))
           commutator(X,Y) = multiply(X,multiply(Y,multiply(inverse(X),inverse(Y))))
           multiply(X,multiply(X,X)) = identity

CONCLUSION commutator(commutator(a,b),b) = identity
