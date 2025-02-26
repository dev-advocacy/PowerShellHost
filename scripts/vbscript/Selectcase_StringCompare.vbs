Option Explicit

call Main

Sub Main
	Dim stringA,stringB
	stringA=Wscript.Arguments.Item(0)
	stringB=Wscript.Arguments.Item(1)
	Select Case StrComp(stringA,stringB, vbTextCompare)
		Case 0
			MsgBox"Both the strings are the same ", vbInformation," Success"
		Case 1
			MsgBox stringA& " is greater than "& stringB, vbQuestion, "Warning"
		Case -1
			MsgBox stringA& " is less than "& stringB, vbQuestion, "Warning"
		Case Else
			MsgBox "There is an error in string comparison"& vbCritical, "Error"
	End Select
end Sub