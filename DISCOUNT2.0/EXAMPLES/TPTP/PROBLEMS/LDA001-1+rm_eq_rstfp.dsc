MODE PROOF

NAME LDA001_1_rm_eq_rstfp_in

ORDERING XKBO
           f:1 > u:1 > 1:1 > 2:1 > 3:1

EQUATIONS
           f(X,f(Y,Z)) = f(f(X,Y),f(X,Z))
           2 = f(1,1)
           3 = f(2,1)
           u = f(2,2)

CONCLUSION f(f(3,2),u) = f(f(u,u),u)
