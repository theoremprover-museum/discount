MODE PROOF, PARAMOD(2)

NAME COL016_1_rm_eq_rstfp_in

ORDERING XKBO
           apply:1 > l:1 > m:1 > b:1 > combinator:1

EQUATIONS
           apply(apply(apply(b,X),Y),Z) = apply(X,apply(Y,Z))
           apply(apply(l,X),Y) = apply(X,apply(Y,Y))
           apply(m,X) = apply(X,X)

CONCLUSION Y = apply(combinator,Y)
