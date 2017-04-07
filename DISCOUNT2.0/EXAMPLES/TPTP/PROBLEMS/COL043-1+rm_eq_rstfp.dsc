MODE PROOF, PARAMOD(2)

NAME COL043_1_rm_eq_rstfp_in

ORDERING XKBO
           apply:1 > h:1 > b:1 > f:1

EQUATIONS
           apply(apply(apply(b,X),Y),Z) = apply(X,apply(Y,Z))
           apply(apply(apply(h,X),Y),Z) = apply(apply(apply(X,Y),Z),Y)

CONCLUSION apply(Y,f(Y)) = apply(f(Y),apply(Y,f(Y)))
