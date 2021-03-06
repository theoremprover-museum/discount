MODE PROOF, PARAMOD(2)

NAME COL069_1_rm_eq_rstfp_in

ORDERING XKBO
           apply:1 > l:1 > b:1 > f:1

EQUATIONS
           apply(apply(apply(b,X),Y),Z) = apply(X,apply(Y,Z))
           apply(apply(l,X),Y) = apply(X,apply(Y,Y))

CONCLUSION apply(Y,f(Y)) = apply(f(Y),apply(Y,f(Y)))
