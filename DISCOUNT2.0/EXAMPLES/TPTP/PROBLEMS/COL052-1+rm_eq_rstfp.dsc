MODE PROOF, PARAMOD(2)

NAME COL052_1_rm_eq_rstfp_in

ORDERING XKBO
           response:1 > compose:1 > common_bird:1 > c:1 > odd_bird:1 > a:1 > b:1

EQUATIONS
           response(compose(X,Y),W) = response(X,response(Y,W))
           response(c,common_bird(X)) = response(X,common_bird(X))
           c = compose(a,b)

CONCLUSION response(a,V) = response(odd_bird,V)
