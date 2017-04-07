MODE PROOF, PARAMOD(2)

NAME COL026_1_rm_eq_rstfp_in

ORDERING XKBO
           apply:1 > b:1 > w1:1 > combinator:1

EQUATIONS
           apply(apply(apply(b,X),Y),Z) = apply(X,apply(Y,Z))
           apply(apply(w1,X),Y) = apply(apply(Y,X),X)

CONCLUSION Y = apply(combinator,Y)
