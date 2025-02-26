Option Explicit

Dim name
name=inputBox("Please enter your name","Name Entry","Example: John Doe")
if Len(name)>5 Then MsgBox"This name is "&Ucase(name)&" and its length is "&len(name)&" characters"