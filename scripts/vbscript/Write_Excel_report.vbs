' VBScript code
' This script is just an example.
' Important: Write your own script, test it on a non-production machine and use it at your own risk.
Option Explicit
Dim strFile
' Give a folder that exists on our computer.
strFile = "D:\Executable\Windows\VBS\" & Date & ".xlsx"
FileDelete
WriteExcelReport

Sub WriteExcelReport
    Dim objExcelApp, objRange
    Set objExcelApp = CreateObject("Excel.Application")
    With objExcelApp
        .Application.Visible = True
        .Workbooks.Add
        .Cells(1, 1).Value = "Report generated on " & Date
        .Cells(1, 1).Font.ColorIndex = 30
        .Cells(2,1).Value = "Orders Placed"
        .Cells(3,1).Value = "Number of Customers"
        .Cells(3,2).Value = 10
        .Cells(3,2).Font.ColorIndex = 50
        .Cells(4,1).Value = "Total Order value"   
        Set objRange = .ActiveCell.EntireColumn
        With objRange
            .Font.Size = 12
            .AutoFit()
        End With
        .Cells(4,2).Value= 10000
        .Cells(4,2).Font.ColorIndex = 50       
        .ActiveWorkbook.SaveAs strFile
        .ActiveWorkbook.Close
        .Application.Quit
    End With
End Sub

Sub FileDelete
    Dim objFSO
    Set objFSO = CreateObject("Scripting.FileSystemObject")
    If objFSO.FileExists(strFile) Then objFSO.DeleteFile(strFile)
End Sub
