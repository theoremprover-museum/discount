MODE PROOF

NAME LCL165_1_rm_eq_rstfp_in

ORDERING XKBO
           or:1 > implies:1 > and:1 > not:1 > true:1 > x:1

EQUATIONS
           implies(true,X) = X
           implies(implies(X,Y),implies(implies(Y,Z),implies(X,Z))) = true
           implies(implies(X,Y),Y) = implies(implies(Y,X),X)
           implies(implies(not(X),not(Y)),implies(Y,X)) = true
           or(X,Y) = implies(not(X),Y)
           or(or(X,Y),Z) = or(X,or(Y,Z))
           or(X,Y) = or(Y,X)
           and(X,Y) = not(or(not(X),not(Y)))
           and(and(X,Y),Z) = and(X,and(Y,Z))
           and(X,Y) = and(Y,X)

CONCLUSION not(or(and(x,or(x,x)),and(x,x))) = and(not(x),or(or(not(x),not(x)),and(not(x),not(x))))
