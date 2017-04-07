MODE PROOF, PARAMOD(2)

NAME COL073_1_rm_eq_rstfp_in

ORDERING XKBO
           apply:1 > n1:1 > b:1 > f:1

EQUATIONS
           apply(apply(apply(n1,X),Y),Z) = apply(apply(apply(X,Y),Y),Z)
           apply(apply(apply(b,X),Y),Z) = apply(X,apply(Y,Z))

CONCLUSION apply(Y,f(Y)) = apply(f(Y),apply(Y,f(Y)))
