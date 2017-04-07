MODE PROOF

NAME GRP188_2_rm_eq_rstfp_in

ORDERING XKBO
           greatest_lower_bound:1 > least_upper_bound:1 > multiply:1 > identity:1 > inverse:1 > a:1 > b:1

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
           inverse(identity) = identity
           inverse(inverse(X)) = X
           inverse(multiply(X,Y)) = multiply(inverse(Y),inverse(X))

CONCLUSION least_upper_bound(b,least_upper_bound(a,b)) = least_upper_bound(a,b)
