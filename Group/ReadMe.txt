Starting the Assignment 02 project:

 - The GEDGame_VS2017 folder is the correct project folder
 - The folder "baufeld" should be in the same directory as "external"
 - The x64 Debug & Release version, both are working

Assignment 04:
Questions:

Q1. If the terrain geometry should be rendered using “Draw()” 
instead of “DrawIndexed()”, what changes would be necessary to the vertex buffer?

A1: With "DrawIndexed()" the vertex buffer is only the collection of existing vertices and the triangles are defined via the index buffer.
If you want to use "Draw()" you have to refill the vertex buffer (actually you will need to use a temporary help vertex array):
Go through the Index buffer (i the current index): The new vertex value is the vertex in the old vertex array at position i.
After going through the index buffer you will have the changed vertex buffer ready to be used for "Draw()".

Q2:
Given is a triangle by the vertices A = (3, 0, 6); B = (0, 2, 0); C = (6, 0, 0).
 The following RGB-colors are assigned at each vertex: Ca = (1, 0, 0); Cb = (0, 1, 1); Cc = (1, 0, 1).
 Calculate the color Cp at point P = (2, 1, 2) by using barycentric interpolation.
 You can assume that P lies inside the given triangle.
 Hint: To calculate the area of a triangle XYZ you can use the formula 1/2 || XY x XZ ||.

A2:
area(ABC) = 1/2 * ||AB x AC|| = 1/2 *||((-3,2,-6) x (3,0,-6))||	= 1/2 * ||((2 * (-6) - (-6) * 0),((-6) * 3 - (-3) * (-6)), 3 * 0 - 2 * 3)||	= 1/2 * ||(-12,-36,-6)||= 1/2 * Root((-12) * (-12) + (-36) * (-36) + (-6) * (-6))	= 1/2 * Root(1476)	= 1/2 * 38.419
 = 19.209
area(ABP) = 1/2 * ||AB x AP|| = 1/2 *||((-3,2,-6) x (-1,1,-4))||= ||((2 * (-4) - (-6) * 1),((-6) * (-1) - (-3) * (-4)),((-3) * 1 - 2 * (-1)))||	= 1/2 * ||(-2,-18,-1)||	= 1/2 * Root((-2) * (-2) + (-18) * (-18) + (-1) * (-1))		= 1/2 * Root(329)	= 1/2 * 18.138
 = 9.069
area(ACP) = 1/2 * ||AC x AP|| = 1/2 *||((3,0,-6) x (-1,1,-4))||	= 1/2 *||((0 * (-4) - (-6) * 1),((-6) * (-1) - 3 * (-4)),(3 * 1 - 0 * (-1)))||	= 1/2 * ||(6,-6,3)||	= 1/2 * Root(6 * 6 + (-6) * (-6) + 3 * 3)			= 1/2 * Root(81)	= 1/2 * 9
 = 4.5
area(BCP) = 1/2 * ||BC x BP|| = 1/2 *||((6,-2,0) x (2,-1,2))||	= 1/2 *||(((-2) * 2 - 0 * (-1)),(0 * 2 - 6 * 2),(6 * (-1) - (-2) * 2))||	= 1/2 * ||(-4,-12,2)||	= 1/2 * Root((-4) * (-4) + (-12) * (-12) + 2 * 2)		= 1/2 * Root(164)	= 1/2 * 12.806
 = 6.403

a1 = area(BCP) / area(ABC) = 6.403 / 19.209 	= 0.333

a2 = area(ACP) / area(ABC) = 4.5 / 19.209	= 0.234

a3 = area(ABP) / area(ABC) = 9.069 / 19.209	= 0.472

Cp = a1 * C1 + a2 * C2 + a3 * C3 = 0.333 * (1,0,0) + 0.234 * (0,1,1) + 0.472 * (1,0,1) = (0.333,0,0) + (0,0.234,0.234) + (0.472,0,0.472) = (0.805,0.234,0.706)
 = (0.805,0.234,0.706)



Assignment 05:
Questions:

See: ...\baufeld\ged_ass05_questions.pdf for answers for the questions


Assignment 06:
Questions:

See: ...\baufeld\GED_Ass6_Questions.pdf for the answers to the questions

Assignment 07:
Questions:

See ...\baufeld\GED_Ass7_Questions.pdf for the answers to the questions


Shooting With the Guns:
 - Plasma:	Press the '1'-key
 - Gatling: 	Press the '3'-key