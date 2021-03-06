MODE PROOF

NAME BOO007_2_rm_eq_rstfp_in

ORDERING XKBO
           add:1 > multiply:1 > additive_identity:1 > multiplicative_identity:1 > inverse:1 > a:1 > b:1 > c:1

EQUATIONS
           add(X,Y) = add(Y,X)
           multiply(X,Y) = multiply(Y,X)
           add(multiply(X,Y),Z) = multiply(add(X,Z),add(Y,Z))
           add(X,multiply(Y,Z)) = multiply(add(X,Y),add(X,Z))
           multiply(add(X,Y),Z) = add(multiply(X,Z),multiply(Y,Z))
           multiply(X,add(Y,Z)) = add(multiply(X,Y),multiply(X,Z))
           add(X,inverse(X)) = multiplicative_identity
           add(inverse(X),X) = multiplicative_identity
           multiply(X,inverse(X)) = additive_identity
           multiply(inverse(X),X) = additive_identity
           multiply(X,multiplicative_identity) = X
           multiply(multiplicative_identity,X) = X
           add(X,additive_identity) = X
           add(additive_identity,X) = X

CONCLUSION multiply(a,multiply(b,c)) = multiply(multiply(a,b),c)
