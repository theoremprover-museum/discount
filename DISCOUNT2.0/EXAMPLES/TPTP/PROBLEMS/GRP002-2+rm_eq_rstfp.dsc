MODE PROOF

NAME GRP002_2_rm_eq_rstfp_in

ORDERING XKBO
           multiply:1 > h:1 > j:1 > k:1 > identity:1 > a:1 > b:1 > c:1 > inverse:1 > d:1

EQUATIONS
           multiply(identity,X) = X
           multiply(inverse(X),X) = identity
           multiply(multiply(X,Y),Z) = multiply(X,multiply(Y,Z))
           multiply(X,identity) = X
           multiply(X,inverse(X)) = identity
           multiply(X,multiply(X,X)) = identity
           multiply(a,b) = c
           multiply(c,inverse(a)) = d
           multiply(d,inverse(b)) = h
           multiply(h,b) = j
           multiply(j,inverse(h)) = k

CONCLUSION multiply(k,inverse(b)) = identity
