Option Explicit
'This is to run the script even there is an error
On Error Resume Next

Dim ObjFSO,folder,file

set ObjFSO= CreateObject("Scripting.FileSystemObject")
set folder = objFSO.GetFolder("D:\Executable\Windows\VBS")

For Each file in folder.files
	MsgBox File.Name
Next