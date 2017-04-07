MODE PROOF, PARAMOD(2)

NAME ROB006_2_rm_eq_rstfp_in

ORDERING XKBO
           add:1 > negate:1 > c:1 > d:1

EQUATIONS
           add(X,Y) = add(Y,X)
           add(add(X,Y),Z) = add(X,add(Y,Z))
           negate(add(negate(add(X,Y)),negate(add(X,negate(Y))))) = X
           add(c,d) = d

CONCLUSION add(X,X) = X
