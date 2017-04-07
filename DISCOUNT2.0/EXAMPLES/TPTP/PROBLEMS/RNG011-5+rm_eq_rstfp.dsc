MODE PROOF

NAME RNG011_5_rm_eq_rstfp_in

ORDERING XKBO
           add:1 > multiply:1 > commutator:1 > associator:1 > additive_inverse:1 > additive_identity:1 > a:1 > b:1

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

CONCLUSION multiply(multiply(associator(a,a,b),a),associator(a,a,b)) = additive_identity
