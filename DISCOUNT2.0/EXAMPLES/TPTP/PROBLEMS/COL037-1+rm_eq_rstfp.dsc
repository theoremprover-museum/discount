MODE PROOF, PARAMOD(2)

NAME COL037_1_rm_eq_rstfp_in

ORDERING XKBO
           apply:1 > b:1 > c:1 > s:1 > f:1

EQUATIONS
           apply(apply(apply(s,X),Y),Z) = apply(apply(X,Z),apply(Y,Z))
           apply(apply(apply(b,X),Y),Z) = apply(X,apply(Y,Z))
           apply(apply(apply(c,X),Y),Z) = apply(apply(X,Z),Y)

CONCLUSION apply(Y,f(Y)) = apply(f(Y),apply(Y,f(Y)))
