MODE PROOF

NAME LCL111_2_rm_eq_rstfp_in

ORDERING XKBO
           implies:1 > not:1 > true:1 > x:1 > y:1 > z:1

EQUATIONS
           implies(true,X) = X
           implies(implies(X,Y),implies(implies(Y,Z),implies(X,Z))) = true
           implies(implies(X,Y),Y) = implies(implies(Y,X),X)
           implies(implies(not(X),not(Y)),implies(Y,X)) = true

CONCLUSION implies(implies(x,y),implies(implies(z,x),implies(z,y))) = true
