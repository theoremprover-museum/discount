MODE PROOF, PARAMOD(2)

NAME COL018_1_rm_eq_rstfp_in

ORDERING XKBO
           apply:1 > w:1 > l:1 > q:1 > combinator:1

EQUATIONS
           apply(apply(l,X),Y) = apply(X,apply(Y,Y))
           apply(apply(w,X),Y) = apply(apply(X,Y),Y)
           apply(apply(apply(q,X),Y),Z) = apply(Y,apply(X,Z))

CONCLUSION Y = apply(combinator,Y)
