Option Explicit

Dim strTime
strTime = Time

'IF block starts
'(1,strTime,"AM")
' "1" - start of the string
'"strTime" - string to be searched
'"AM" - character to be searched in the string
if InStr(1,strTime,"AM")<>0 Then
    MsgBox"The current time is "&strTime&". Good Morning!"

'Else block starts
Else
    MsgBox"The current time is "&strTime&". Good Night!"
End if