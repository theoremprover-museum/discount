MODE PROOF

NAME RNG035_7_rm_eq_rstfp_in

ORDERING XKBO
           add:1 > multiply:1 > additive_inverse:1 > a:1 > b:1 > c:1 > additive_identity:1

EQUATIONS
           add(additive_identity,X) = X
           add(X,additive_identity) = X
           add(additive_inverse(X),X) = additive_identity
           add(X,additive_inverse(X)) = additive_identity
           add(X,add(Y,Z)) = add(add(X,Y),Z)
           add(X,Y) = add(Y,X)
           multiply(X,multiply(Y,Z)) = multiply(multiply(X,Y),Z)
           multiply(X,add(Y,Z)) = add(multiply(X,Y),multiply(X,Z))
           multiply(add(X,Y),Z) = add(multiply(X,Z),multiply(Y,Z))
           multiply(X,multiply(X,multiply(X,X))) = X
           multiply(a,b) = c

CONCLUSION multiply(b,a) = c
