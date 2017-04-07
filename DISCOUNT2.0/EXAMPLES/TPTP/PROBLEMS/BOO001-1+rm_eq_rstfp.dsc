MODE PROOF

NAME BOO001_1_rm_eq_rstfp_in

ORDERING XKBO
           multiply:1 > inverse:1 > a:1

EQUATIONS
           multiply(multiply(V,W,X),Y,multiply(V,W,Z)) = multiply(V,W,multiply(X,Y,Z))
           multiply(Y,X,X) = X
           multiply(X,X,Y) = X
           multiply(inverse(Y),Y,X) = X
           multiply(X,Y,inverse(Y)) = X

CONCLUSION inverse(inverse(a)) = a
