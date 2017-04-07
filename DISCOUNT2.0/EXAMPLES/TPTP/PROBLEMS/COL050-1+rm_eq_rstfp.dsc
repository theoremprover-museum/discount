MODE PROOF, PARAMOD(2)

NAME COL050_1_rm_eq_rstfp_in

ORDERING XKBO
           response:1 > compose:1 > mocking_bird:1 > a:1

EQUATIONS
           response(mocking_bird,Y) = response(Y,Y)
           response(compose(X,Y),W) = response(X,response(Y,W))

CONCLUSION response(a,Y) = Y
