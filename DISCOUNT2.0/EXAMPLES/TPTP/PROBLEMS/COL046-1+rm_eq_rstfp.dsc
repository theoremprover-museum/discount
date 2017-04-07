MODE PROOF, PARAMOD(2)

NAME COL046_1_rm_eq_rstfp_in

ORDERING XKBO
           apply:1 > m:1 > b:1 > s:1 > f:1

EQUATIONS
           apply(apply(apply(s,X),Y),Z) = apply(apply(X,Z),apply(Y,Z))
           apply(apply(apply(b,X),Y),Z) = apply(X,apply(Y,Z))
           apply(m,X) = apply(X,X)

CONCLUSION apply(Y,f(Y)) = apply(f(Y),apply(Y,f(Y)))
