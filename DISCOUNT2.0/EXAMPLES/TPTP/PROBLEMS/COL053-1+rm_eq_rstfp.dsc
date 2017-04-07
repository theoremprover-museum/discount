MODE PROOF, PARAMOD(2)

NAME COL053_1_rm_eq_rstfp_in

ORDERING XKBO
           response:1 > compose:1 > a:1 > b:1 > c:1 > f:1

EQUATIONS
           response(compose(X,Y),W) = response(X,response(Y,W))

CONCLUSION response(U,f(U)) = response(a,response(b,response(c,f(U))))
