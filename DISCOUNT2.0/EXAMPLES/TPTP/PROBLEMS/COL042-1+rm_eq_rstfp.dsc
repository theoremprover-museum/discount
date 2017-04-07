MODE PROOF, PARAMOD(2)

NAME COL042_1_rm_eq_rstfp_in

ORDERING XKBO
           apply:1 > b:1 > w1:1 > f:1

EQUATIONS
           apply(apply(apply(b,X),Y),Z) = apply(X,apply(Y,Z))
           apply(apply(w1,X),Y) = apply(apply(Y,X),X)

CONCLUSION apply(Y,f(Y)) = apply(f(Y),apply(Y,f(Y)))
