MODE PROOF

NAME GRP178_1_rm_eq_rstfp_in

ORDERING XKBO
           greatest_lower_bound:1 > least_upper_bound:1 > multiply:1 > identity:1 > a:1 > b:1 > c:1 > inverse:1

EQUATIONS
           multiply(identity,X) = X
           multiply(inverse(X),X) = identity
           multiply(multiply(X,Y),Z) = multiply(X,multiply(Y,Z))
           greatest_lower_bound(X,Y) = greatest_lower_bound(Y,X)
           least_upper_bound(X,Y) = least_upper_bound(Y,X)
           greatest_lower_bound(X,greatest_lower_bound(Y,Z)) = greatest_lower_bound(greatest_lower_bound(X,Y),Z)
           least_upper_bound(X,least_upper_bound(Y,Z)) = least_upper_bound(least_upper_bound(X,Y),Z)
           least_upper_bound(X,X) = X
           greatest_lower_bound(X,X) = X
           least_upper_bound(X,greatest_lower_bound(X,Y)) = X
           greatest_lower_bound(X,least_upper_bound(X,Y)) = X
           multiply(X,least_upper_bound(Y,Z)) = least_upper_bound(multiply(X,Y),multiply(X,Z))
           multiply(X,greatest_lower_bound(Y,Z)) = greatest_lower_bound(multiply(X,Y),multiply(X,Z))
           multiply(least_upper_bound(Y,Z),X) = least_upper_bound(multiply(Y,X),multiply(Z,X))
           multiply(greatest_lower_bound(Y,Z),X) = greatest_lower_bound(multiply(Y,X),multiply(Z,X))
           least_upper_bound(identity,a) = a
           least_upper_bound(identity,b) = b
           least_upper_bound(identity,c) = c
           greatest_lower_bound(a,b) = identity

CONCLUSION greatest_lower_bound(a,multiply(b,c)) = greatest_lower_bound(a,c)
