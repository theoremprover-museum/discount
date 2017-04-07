MODE PROOF

NAME RNG010_6_rm_eq_rstfp_in

ORDERING XKBO
           add:1 > multiply:1 > commutator:1 > s:1 > associator:1 > additive_inverse:1 > additive_identity:1 > a:1 > b:1 > c:1 > d:1

EQUATIONS
           add(X,Y) = add(Y,X)
           add(X,add(Y,Z)) = add(add(X,Y),Z)
           add(additive_identity,X) = X
           add(X,additive_identity) = X
           multiply(additive_identity,X) = additive_identity
           multiply(X,additive_identity) = additive_identity
           add(additive_inverse(X),X) = additive_identity
           add(X,additive_inverse(X)) = additive_identity
           multiply(X,add(Y,Z)) = add(multiply(X,Y),multiply(X,Z))
           multiply(add(X,Y),Z) = add(multiply(X,Z),multiply(Y,Z))
           additive_inverse(additive_inverse(X)) = X
           multiply(multiply(X,Y),Y) = multiply(X,multiply(Y,Y))
           associator(X,Y,Z) = add(multiply(multiply(X,Y),Z),additive_inverse(multiply(X,multiply(Y,Z))))
           commutator(X,Y) = add(multiply(Y,X),additive_inverse(multiply(X,Y)))
           s(W,X,Y,Z) = add(add(associator(multiply(W,X),Y,Z),additive_inverse(multiply(X,associator(W,Y,Z)))),additive_inverse(multiply(associator(X,Y,Z),W)))
           multiply(X,associator(Y,Z,Y)) = multiply(commutator(multiply(X,Y),Z),Y)
           multiply(associator(Y,Z,Y),X) = multiply(Y,commutator(Z,multiply(Y,X)))
           multiply(multiply(X,Y),multiply(Z,X)) = multiply(multiply(X,multiply(Y,Z)),X)

CONCLUSION s(a,b,c,d) = s(b,a,c,d)
