MODE PROOF

NAME LCL136_1_rm_eq_rstfp_in

ORDERING XKBO
           implies:1 > x:1 > y:1 > z:1 > not:1 > true:1

EQUATIONS
           implies(true,X) = X
           implies(implies(X,Y),implies(implies(Y,Z),implies(X,Z))) = true
           implies(implies(X,Y),Y) = implies(implies(Y,X),X)
           implies(implies(not(X),not(Y)),implies(Y,X)) = true
           implies(x,y) = implies(y,z)

CONCLUSION implies(x,z) = true
