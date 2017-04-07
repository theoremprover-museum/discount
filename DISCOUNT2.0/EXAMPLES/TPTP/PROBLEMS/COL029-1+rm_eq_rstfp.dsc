MODE PROOF, PARAMOD(2)

NAME COL029_1_rm_eq_rstfp_in

ORDERING XKBO
           apply:1 > u:1 > f:1

EQUATIONS
           apply(apply(u,X),Y) = apply(Y,apply(apply(X,X),Y))

CONCLUSION apply(Y,f(Y)) = apply(f(Y),apply(Y,f(Y)))
