MODE PROOF

NAME RNG009_5_rm_eq_rstfp_in

ORDERING XKBO
           add:1 > multiply:1 > additive_inverse:1 > additive_identity:1 > a:1 > b:1

EQUATIONS
           add(X,additive_identity) = X
           add(X,additive_inverse(X)) = additive_identity
           multiply(X,add(Y,Z)) = add(multiply(X,Y),multiply(X,Z))
           multiply(add(X,Y),Z) = add(multiply(X,Z),multiply(Y,Z))
           add(add(X,Y),Z) = add(X,add(Y,Z))
           add(X,Y) = add(Y,X)
           multiply(multiply(X,Y),Z) = multiply(X,multiply(Y,Z))
           multiply(X,multiply(X,X)) = X

CONCLUSION multiply(a,b) = multiply(b,a)
