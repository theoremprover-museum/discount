MODE PROOF

NAME RNG027_5_rm_eq_rstfp_in

ORDERING XKBO
           add:1 > multiply:1 > commutator:1 > associator:1 > additive_inverse:1 > additive_identity:1 > cz:1 > cx:1 > cy:1

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

CONCLUSION multiply(cz,multiply(cx,multiply(cy,cx))) = multiply(multiply(multiply(cz,cx),cy),cx)
