MODE PROOF

NAME RNG007_4_rm_eq_rstfp_in

ORDERING XKBO
           add:1 > multiply:1 > additive_inverse:1 > additive_identity:1 > a:1

EQUATIONS
           add(additive_identity,X) = X
           add(additive_inverse(X),X) = additive_identity
           multiply(X,add(Y,Z)) = add(multiply(X,Y),multiply(X,Z))
           multiply(add(X,Y),Z) = add(multiply(X,Z),multiply(Y,Z))
           additive_inverse(additive_identity) = additive_identity
           additive_inverse(additive_inverse(X)) = X
           multiply(X,additive_identity) = additive_identity
           multiply(additive_identity,X) = additive_identity
           additive_inverse(add(X,Y)) = add(additive_inverse(X),additive_inverse(Y))
           multiply(X,additive_inverse(Y)) = additive_inverse(multiply(X,Y))
           multiply(additive_inverse(X),Y) = additive_inverse(multiply(X,Y))
           add(add(X,Y),Z) = add(X,add(Y,Z))
           add(X,Y) = add(Y,X)
           multiply(multiply(X,Y),Z) = multiply(X,multiply(Y,Z))
           multiply(X,X) = X

CONCLUSION add(a,a) = additive_identity
