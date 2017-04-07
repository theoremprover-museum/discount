MODE PROOF

NAME LCL109_2_rm_eq_rstfp_in

ORDERING XKBO
           implies:1 > not:1 > true:1 > a:1 > b:1

EQUATIONS
           implies(true,X) = X
           implies(implies(X,Y),implies(implies(Y,Z),implies(X,Z))) = true
           implies(implies(X,Y),Y) = implies(implies(Y,X),X)
           implies(implies(not(X),not(Y)),implies(Y,X)) = true

CONCLUSION implies(implies(implies(a,b),implies(b,a)),implies(b,a)) = true
