MODE PROOF

NAME LCL153_1_rm_eq_rstfp_in

ORDERING XKBO
           and_star:1 > or:1 > implies:1 > xor:1 > and:1 > false:1 > not:1 > true:1 > x:1

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
           xor(X,Y) = or(and(X,not(Y)),and(not(X),Y))
           xor(X,Y) = xor(Y,X)
           and_star(X,Y) = not(or(not(X),not(Y)))
           and_star(and_star(X,Y),Z) = and_star(X,and_star(Y,Z))
           and_star(X,Y) = and_star(Y,X)
           not(true) = false

CONCLUSION not(x) = xor(x,true)
