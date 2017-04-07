MODE PROOF

NAME BOO002_1_rm_eq_rstfp_in

ORDERING XKBO
           multiply:1 > inverse:1 > a:1 > b:1

EQUATIONS
           multiply(multiply(V,W,X),Y,multiply(V,W,Z)) = multiply(V,W,multiply(X,Y,Z))
           multiply(Y,X,X) = X
           multiply(X,X,Y) = X
           multiply(inverse(Y),Y,X) = X

CONCLUSION multiply(a,inverse(a),b) = b
