MODE PROOF, PARAMOD(2)

NAME COL071_1_rm_eq_rstfp_in

ORDERING XKBO
           apply:1 > n:1 > q:1 > f:1

EQUATIONS
           apply(apply(apply(n,X),Y),Z) = apply(apply(apply(X,Z),Y),Z)
           apply(apply(apply(q,X),Y),Z) = apply(Y,apply(X,Z))

CONCLUSION apply(Y,f(Y)) = apply(f(Y),apply(Y,f(Y)))
