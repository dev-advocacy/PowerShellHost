Option Explicit

Dim firstnumber
firstnumber=InputBox("Please enter a number to add : ", "First Number")
firstnumber=CInt(firstnumber)

Dim secondnumber
' "vbcrlf" - similar to "\n"
' "0" in the end is default value to add 
secondnumber=cint(InputBox("Please enter the second number to add"&vbcrlf&"to the sum","Second Number",0))

Dim sum
sum = firstnumber+secondnumber

MsgBox"The sum is "&sum