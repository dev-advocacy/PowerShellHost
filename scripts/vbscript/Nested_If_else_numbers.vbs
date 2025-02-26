Option Explicit

Dim a,b,c,biggest

a = 10
b = 200
c = 30

if a>b Then
	if a>c Then
		biggest = a
	Else
		biggest = c
	end if
	
Else
	if b>c Then
		biggest=b
	Else
		biggest = c
	end if

end If
MsgBox"The bigest number is "& biggest