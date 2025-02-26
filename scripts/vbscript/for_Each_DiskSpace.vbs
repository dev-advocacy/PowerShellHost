
Option Explicit
Dim objWMIService, objHardDisk, ObjDrive
Dim objFSO, objFile

Const ForReading=1, ForWriting=2,ForAppending=8

Set objWMIService = GetObject("winmgmts:\\"&"my-pc"&"\root\cimv2")
Set objHardDisk = objWMIService.ExecQuery("Select * from Win32_LogicalDisk")
Set objFSO = CreateObject("Scripting.FileSystemObject")
Set objFile = objFSO.OpenTextFile("D:\Executable\Windows\VBS\Disk.txt",ForAppending)

Dim intTotalSpace, intFreeSpace, strDriveName
For Each objDrive in objHardDisk
	intTotalSpace = Round(objDrive.Size/1073741824,0)
	intFreeSpace = Round(objDrive.FreeSpace/1073741824,0)
	strDriveName = objDrive.DeviceID
	objFile.WriteLine Now & " the "&strDriveName& " hard disk drive has a total of "& intTotalSpace &_
	"GB and free space of "& intFreeSpace&" GB."

Next
objFile.WriteLine"----------------------------------------------------------------------------"	
