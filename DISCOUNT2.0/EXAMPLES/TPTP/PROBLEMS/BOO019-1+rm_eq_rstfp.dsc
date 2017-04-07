MODE PROOF

NAME BOO019_1_rm_eq_rstfp_in

ORDERING XKBO
           multiply:1 > inverse:1 > x:1 > y:1

EQUATIONS
           multiply(multiply(V,W,X),Y,multiply(V,W,Z)) = multiply(V,W,multiply(X,Y,Z))
           multiply(X,X,Y) = X
           multiply(inverse(Y),Y,X) = X
           multiply(X,Y,inverse(Y)) = X

CONCLUSION multiply(y,x,x) = x
