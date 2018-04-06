\ENCODING=ISO-8859-1
\Problem name: toy

Minimize
 obj: x1 + 2 x2 + 3 x3 + 4 x4 + 5 x5 + 6 x6 + [ x1 ^2 + 4 x1 * x2 + 6 x1 * x3
      + 4 x1 * x4 + 8 x1 * x5 + 12 x1 * x6 + x2 ^2 + 4 x2 * x3 + 8 x2 * x4
      + 4 x2 * x5 + 8 x2 * x6 + x3 ^2 + 12 x3 * x4 + 8 x3 * x5 + 4 x3 * x6
      + x4 ^2 + 4 x4 * x5 + 6 x4 * x6 + x5 ^2 + 4 x5 * x6 + x6 ^2 ] / 2
Subject To
 c1: x1 + x4  = 1
 c2: x2 + x5  = 1
 c3: x3 + x6  = 1
 c4: 2 x1 + 2 x2 + 2 x3 <= 4
 c5: 3 x4 + 3 x5 + 3 x6 <= 5
Bounds
 0 <= x1 <= 1
 0 <= x2 <= 1
 0 <= x3 <= 1
 0 <= x4 <= 1
 0 <= x5 <= 1
 0 <= x6 <= 1
Generals
 x1  x2  x3  x4  x5  x6 
End
