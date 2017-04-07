MODE PROOF, PARAMOD(2)

NAME COL031_1_rm_eq_rstfp_in

ORDERING XKBO
           apply:1 > l:1 > o:1 > f:1

EQUATIONS
           apply(apply(l,X),Y) = apply(X,apply(Y,Y))
           apply(apply(o,X),Y) = apply(Y,apply(X,Y))

CONCLUSION apply(Y,f(Y)) = apply(f(Y),apply(Y,f(Y)))
