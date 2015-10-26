# Copyright (C) 2015 Lukasz Czerwinski

# Calc
Shell calculator in C++ boost::spirit  

# Website
https://github.com/wo3kie/Calc

# License
For license please refer to LICENSE file  

# Requirements
C++11  
boost  

# How to build it
make

# Features
## Operators
+, -,
\*, /  
^  
==, !=, <, <=, >, >=  
!, &&, ||  
()  

```{r, engine='bash'}
$./calc "1+2*3"
7

$./calc "2^3^2"
512

$./calc "1||(1/0)"
1
```

## Functions
sin, con, tan  
ln, log2, log10  
rad, deg  
abs  
e, pi  

```{r, engine='bash'}
$./calc "sin(rad(45))"
0.707107

$ ./calc "pi()*e()"
8.53973
```

## Placeholders
\_0 - number of arguments  
\_1, \_2, ... \_9 - placeholders  

```{r, engine='bash'}
$./calc "_1+_2" 1 2
3

./calc "(_0==3)&&(_1+_2+_3)" 5 10 15
30
```

