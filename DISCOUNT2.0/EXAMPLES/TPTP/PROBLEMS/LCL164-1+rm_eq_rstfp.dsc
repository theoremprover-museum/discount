MODE PROOF

NAME LCL164_1_rm_eq_rstfp_in

ORDERING XKBO
           and_star:1 > implies:1 > xor:1 > false:1 > not:1 > true:1 > x:1 > y:1

EQUATIONS
           not(X) = xor(X,true)
           xor(X,false) = X
           xor(X,X) = false
           and_star(X,true) = X
           and_star(X,false) = false
           and_star(xor(true,X),X) = false
           xor(X,xor(true,Y)) = xor(xor(X,true),Y)
           and_star(xor(and_star(xor(true,X),Y),true),Y) = and_star(xor(and_star(xor(true,Y),X),true),X)
           xor(X,Y) = xor(Y,X)
           and_star(and_star(X,Y),Z) = and_star(X,and_star(Y,Z))
           and_star(X,Y) = and_star(Y,X)
           not(true) = false
           implies(X,Y) = xor(true,and_star(X,xor(true,Y)))

CONCLUSION implies(implies(not(x),not(y)),implies(y,x)) = true
