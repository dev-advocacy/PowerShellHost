Option Explicit

Dim inputWeight
'"cdbl" convet number to double
inputWeight=cDbl(InputBox("Enter the weight in kilos : "))
call ConvertWeight(inputWeight)
' or 
'ConvertWeight inputWeight
call EndScript

'Sub procedure starts
Sub ConvertWeight(kilos)
    Dim Pounds
    Pounds=2.205*kilos
    MsgBox"The weight of "&kilos&" kg is equivalent to "&_
    Pounds&"lbs."

'Sub procedure ends
end Sub

Sub EndScript
    MsgBox"This is the end script"
end Sub