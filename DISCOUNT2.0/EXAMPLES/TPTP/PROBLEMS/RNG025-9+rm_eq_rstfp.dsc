MODE PROOF

NAME RNG025_9_rm_eq_rstfp_in

ORDERING XKBO
           add:1 > multiply:1 > commutator:1 > associator:1 > additive_inverse:1 > additive_identity:1 > a:1 > b:1 > c:1

EQUATIONS
           multiply(additive_inverse(X),additive_inverse(Y)) = multiply(X,Y)
           multiply(additive_inverse(X),Y) = additive_inverse(multiply(X,Y))
           multiply(X,additive_inverse(Y)) = additive_inverse(multiply(X,Y))
           multiply(X,add(Y,additive_inverse(Z))) = add(multiply(X,Y),additive_inverse(multiply(X,Z)))
           multiply(add(X,additive_inverse(Y)),Z) = add(multiply(X,Z),additive_inverse(multiply(Y,Z)))
           multiply(additive_inverse(X),add(Y,Z)) = add(additive_inverse(multiply(X,Y)),additive_inverse(multiply(X,Z)))
           multiply(add(X,Y),additive_inverse(Z)) = add(additive_inverse(multiply(X,Z)),additive_inverse(multiply(Y,Z)))
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
           multiply(multiply(X,X),Y) = multiply(X,multiply(X,Y))
           associator(X,Y,add(U,V)) = add(associator(X,Y,U),associator(X,Y,V))
           associator(X,add(U,V),Y) = add(associator(X,U,Y),associator(X,V,Y))
           associator(add(U,V),X,Y) = add(associator(U,X,Y),associator(V,X,Y))
           commutator(X,Y) = add(multiply(Y,X),additive_inverse(multiply(X,Y)))

CONCLUSION add(associator(a,b,c),associator(a,c,b)) = additive_identity
