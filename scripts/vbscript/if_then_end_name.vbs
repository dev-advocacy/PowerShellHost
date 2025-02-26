Option Explicit

Dim name
'"TRIM" to remove spaces
name=Trim(inputBox("Please enter your name","Name Entry","Example: John Doe"))

if Len(name)>5 Then 
    MsgBox"This name is "&Ucase(name)&"."
    MsgBox "The length of the name is "&len(name)&" characters"
End if