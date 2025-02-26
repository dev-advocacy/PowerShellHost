Option Explicit
Dim i
i=1
Loop1
Loop2

Sub Loop1
Do While i<=5
	MsgBox"The current value of i is "&i, vbInformation,"For Loop"
	i=i+1
Loop
End Sub

Sub Loop2
Do
	MsgBox"The current value of i is "&i, vbInformation,"For Loop"
	i=i+1
Loop While i<=5
End Sub