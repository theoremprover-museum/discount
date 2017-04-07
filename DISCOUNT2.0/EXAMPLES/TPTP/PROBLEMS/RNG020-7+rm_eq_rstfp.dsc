MODE PROOF

NAME RNG020_7_rm_eq_rstfp_in

ORDERING XKBO
           add:1 > multiply:1 > commutator:1 > associator:1 > additive_inverse:1 > additive_identity:1 > u:1 > v:1 > x:1 > y:1

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
           multiply(multiply(X,X),Y) = multiply(X,multiply(X,Y))
           associator(X,Y,Z) = add(multiply(multiply(X,Y),Z),additive_inverse(multiply(X,multiply(Y,Z))))
           commutator(X,Y) = add(multiply(Y,X),additive_inverse(multiply(X,Y)))
           multiply(additive_inverse(X),additive_inverse(Y)) = multiply(X,Y)
           multiply(additive_inverse(X),Y) = additive_inverse(multiply(X,Y))
           multiply(X,additive_inverse(Y)) = additive_inverse(multiply(X,Y))
           multiply(X,add(Y,additive_inverse(Z))) = add(multiply(X,Y),additive_inverse(multiply(X,Z)))
           multiply(add(X,additive_inverse(Y)),Z) = add(multiply(X,Z),additive_inverse(multiply(Y,Z)))
           multiply(additive_inverse(X),add(Y,Z)) = add(additive_inverse(multiply(X,Y)),additive_inverse(multiply(X,Z)))
           multiply(add(X,Y),additive_inverse(Z)) = add(additive_inverse(multiply(X,Z)),additive_inverse(multiply(Y,Z)))

CONCLUSION associator(x,add(u,v),y) = add(associator(x,u,y),associator(x,v,y))
