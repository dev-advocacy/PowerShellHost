Option Explicit

Dim intDayOfWeek
intDayOfWeek = Weekday(Date)

Dim strMessage
if intDayOfWeek=1 Then 'insated of "1" we can also use "vbSunday"
	strMessage = "Sunday"
elseif intDayOfWeek=2 Then 'insated of "2" we can also use "vbMonday"
	strMessage = "Monday"
elseif intDayOfWeek=3 Then
	strMessage = "Tuesday"
elseif intDayOfWeek=4 Then
	strMessage = "Wednesday"
elseif intDayOfWeek=5 Then
	strMessage = "Thursday"
elseif intDayOfWeek=6 Then
	strMessage = "Friday"
elseif intDayOfWeek=7 Then
	strMessage = "Satruday"
else
	strMessage = "Unknown Day"
End if
MsgBox"Have a happy -"&strMessage