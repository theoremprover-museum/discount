MODE PROOF

NAME RNG010_5_rm_eq_rstfp_in

ORDERING XKBO
           add:1 > multiply:1 > commutator:1 > s:1 > associator:1 > additive_inverse:1 > additive_identity:1 > a:1 > b:1 > c:1 > d:1

EQUATIONS
           add(X,Y) = add(Y,X)
           add(add(X,Y),Z) = add(X,add(Y,Z))
           add(X,additive_identity) = X
           add(additive_identity,X) = X
           add(X,additive_inverse(X)) = additive_identity
           add(additive_inverse(X),X) = additive_identity
           additive_inverse(additive_identity) = additive_identity
           add(X,add(additive_inverse(X),Y)) = Y
           additive_inverse(add(X,Y)) = add(additive_inverse(X),additive_inverse(Y))
           additive_inverse(additive_inverse(X)) = X
           multiply(X,additive_identity) = additive_identity
           multiply(additive_identity,X) = additive_identity
           multiply(additive_inverse(X),additive_inverse(Y)) = multiply(X,Y)
           multiply(X,additive_inverse(Y)) = additive_inverse(multiply(X,Y))
           multiply(additive_inverse(X),Y) = additive_inverse(multiply(X,Y))
           multiply(X,add(Y,Z)) = add(multiply(X,Y),multiply(X,Z))
           multiply(add(X,Y),Z) = add(multiply(X,Z),multiply(Y,Z))
           multiply(multiply(X,Y),Y) = multiply(X,multiply(Y,Y))
           associator(X,Y,Z) = add(multiply(multiply(X,Y),Z),additive_inverse(multiply(X,multiply(Y,Z))))
           commutator(X,Y) = add(multiply(Y,X),additive_inverse(multiply(X,Y)))
           multiply(multiply(associator(X,X,Y),X),associator(X,X,Y)) = additive_identity
           multiply(multiply(X,X),Y) = multiply(X,multiply(X,Y))
           s(W,X,Y,Z) = add(add(associator(multiply(W,X),Y,Z),additive_inverse(multiply(X,associator(W,Y,Z)))),additive_inverse(multiply(associator(X,Y,Z),W)))
           multiply(X,multiply(Y,multiply(Z,Y))) = multiply(commutator(multiply(X,Y),Z),Y)
           multiply(multiply(Y,multiply(Z,Y)),X) = multiply(Y,commutator(Z,multiply(Y,X)))
           multiply(multiply(X,Y),multiply(Z,X)) = multiply(multiply(X,multiply(Y,Z)),X)

CONCLUSION s(a,b,c,d) = additive_inverse(s(b,a,c,d))
