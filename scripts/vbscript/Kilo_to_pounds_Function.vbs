Option Explicit

Dim inputWeight
inputWeight=cDbl(InputBox("Enter the weight in kilos : "))
MsgBox"The weight of"&inputWeight&" kg is equivalent to "&_
    Pounds(inputWeight)&"lbs."
MsgBox ExitMessage()

'Function procedure starts
Function Pounds(kilos)
    Pounds=2.205*kilos
'Function procedure ends
end Function

Function ExitMessage()
    ExitMessage= "This is the end script"
end Function