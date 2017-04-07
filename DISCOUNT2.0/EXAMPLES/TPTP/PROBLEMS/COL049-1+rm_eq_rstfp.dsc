MODE PROOF, PARAMOD(2)

NAME COL049_1_rm_eq_rstfp_in

ORDERING XKBO
           apply:1 > w:1 > m:1 > b:1 > f:1

EQUATIONS
           apply(apply(apply(b,X),Y),Z) = apply(X,apply(Y,Z))
           apply(apply(w,X),Y) = apply(apply(X,Y),Y)
           apply(m,X) = apply(X,X)

CONCLUSION apply(Y,f(Y)) = apply(f(Y),apply(Y,f(Y)))
