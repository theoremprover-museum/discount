MODE PROOF, PARAMOD(2)

NAME COL034_1_rm_eq_rstfp_in

ORDERING XKBO
           apply:1 > m:1 > b:1 > t:1 > f:1

EQUATIONS
           apply(apply(apply(b,X),Y),Z) = apply(X,apply(Y,Z))
           apply(m,X) = apply(X,X)
           apply(apply(t,X),Y) = apply(Y,X)

CONCLUSION apply(Y,f(Y)) = apply(f(Y),apply(Y,f(Y)))
