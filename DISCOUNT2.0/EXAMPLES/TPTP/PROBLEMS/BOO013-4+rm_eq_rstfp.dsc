MODE PROOF

NAME BOO013_4_rm_eq_rstfp_in

ORDERING XKBO
           add:1 > multiply:1 > a:1 > b:1 > inverse:1 > multiplicative_identity:1 > additive_identity:1

EQUATIONS
           add(X,Y) = add(Y,X)
           multiply(X,Y) = multiply(Y,X)
           add(X,multiply(Y,Z)) = multiply(add(X,Y),add(X,Z))
           multiply(X,add(Y,Z)) = add(multiply(X,Y),multiply(X,Z))
           add(X,additive_identity) = X
           multiply(X,multiplicative_identity) = X
           add(X,inverse(X)) = multiplicative_identity
           multiply(X,inverse(X)) = additive_identity
           add(a,b) = multiplicative_identity
           multiply(a,b) = additive_identity

CONCLUSION b = inverse(a)
