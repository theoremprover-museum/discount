MODE PROOF

NAME ROB024_1_rm_eq_rstfp_in

ORDERING XKBO
           add:1 > a:1 > negate:1 > b:1

EQUATIONS
           add(X,Y) = add(Y,X)
           add(add(X,Y),Z) = add(X,add(Y,Z))
           negate(add(negate(add(X,Y)),negate(add(X,negate(Y))))) = X
           negate(add(negate(add(a,add(a,b))),negate(add(a,negate(b))))) = a

CONCLUSION add(negate(add(a,negate(b))),negate(add(negate(a),negate(b)))) = b
