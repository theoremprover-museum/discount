MODE PROOF, PARAMOD(2)

NAME COL044_1_rm_eq_rstfp_in

ORDERING XKBO
           apply:1 > n:1 > b:1 > f:1

EQUATIONS
           apply(apply(apply(b,X),Y),Z) = apply(X,apply(Y,Z))
           apply(apply(apply(n,X),Y),Z) = apply(apply(apply(X,Z),Y),Z)

CONCLUSION apply(Y,f(Y)) = apply(f(Y),apply(Y,f(Y)))
