MODE PROOF, PARAMOD(2)

NAME COL056_1_rm_eq_rstfp_in

ORDERING XKBO
           response:1 > compose:1 > a:1 > b:1 > c:1

EQUATIONS
           response(compose(X,Y),W) = response(X,response(Y,W))
           response(a,b) = c
           response(a,c) = b

CONCLUSION response(W,V) = V
