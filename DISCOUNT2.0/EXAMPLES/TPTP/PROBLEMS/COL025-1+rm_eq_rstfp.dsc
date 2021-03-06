MODE PROOF, PARAMOD(2)

NAME COL025_1_rm_eq_rstfp_in

ORDERING XKBO
           apply:1 > w:1 > b:1 > combinator:1

EQUATIONS
           apply(apply(apply(b,X),Y),Z) = apply(X,apply(Y,Z))
           apply(apply(w,X),Y) = apply(apply(X,Y),Y)

CONCLUSION Y = apply(combinator,Y)
