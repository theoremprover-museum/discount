MODE PROOF

NAME LDA002_1_rm_eq_rstfp_in

ORDERING XKBO
           f:1 > uu:1 > u:1 > v:1 > a:1 > b:1 > u1:1 > 1:1 > u2:1 > 2:1 > u3:1 > 3:1

EQUATIONS
           f(X,f(Y,Z)) = f(f(X,Y),f(X,Z))
           2 = f(1,1)
           3 = f(2,1)
           u = f(2,2)
           u1 = f(u,1)
           u2 = f(u,2)
           u3 = f(u,3)
           uu = f(u,u)
           a = f(f(3,2),u2)
           b = f(u1,u3)
           v = f(uu,uu)

CONCLUSION f(a,v) = f(b,v)
