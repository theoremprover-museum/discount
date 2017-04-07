MODE PROOF, PARAMOD(2)

NAME COL047_1_rm_eq_rstfp_in

ORDERING XKBO
           apply:1 > l:1 > q:1 > f:1

EQUATIONS
           apply(apply(l,X),Y) = apply(X,apply(Y,Y))
           apply(apply(apply(q,X),Y),Z) = apply(Y,apply(X,Z))

CONCLUSION apply(Y,f(Y)) = apply(f(Y),apply(Y,f(Y)))
