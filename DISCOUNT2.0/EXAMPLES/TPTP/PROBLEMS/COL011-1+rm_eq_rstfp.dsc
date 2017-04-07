MODE PROOF, PARAMOD(2)

NAME COL011_1_rm_eq_rstfp_in

ORDERING XKBO
           apply:1 > q1:1 > o:1 > combinator:1

EQUATIONS
           apply(apply(o,X),Y) = apply(Y,apply(X,Y))
           apply(apply(apply(q1,X),Y),Z) = apply(X,apply(Z,Y))

CONCLUSION Y = apply(combinator,Y)
