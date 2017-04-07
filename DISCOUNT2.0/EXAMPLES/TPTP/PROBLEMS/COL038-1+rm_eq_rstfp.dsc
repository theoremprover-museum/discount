MODE PROOF, PARAMOD(2)

NAME COL038_1_rm_eq_rstfp_in

ORDERING XKBO
           apply:1 > v:1 > m:1 > b:1 > f:1

EQUATIONS
           apply(apply(apply(b,X),Y),Z) = apply(X,apply(Y,Z))
           apply(m,X) = apply(X,X)
           apply(apply(apply(v,X),Y),Z) = apply(apply(Z,X),Y)

CONCLUSION apply(Y,f(Y)) = apply(f(Y),apply(Y,f(Y)))
