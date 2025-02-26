Option Explicit

Dim i
Call Loop1
'Call Loop2

Sub Loop1
For i= 1 to 5
	MsgBox"The current value of i is "&i, vbInformation,"For loop"

Next
End Sub

Sub Loop2
For i = 1 to 10 Step 2
	MsgBox"The Current value of i is "&i, vbInformation,"For loop"
	'if i=5 Then Exit For
Next
End Sub 