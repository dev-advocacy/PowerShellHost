Option Explicit
Dim objFSO,objFile
Set objFSO = CreateObject("Scripting.FileSystemObject")
set objFile = objFSO.createTextFile("D:\Executable\Windows\VBS\"&Date&".html",True) 'True means overwrite if the file already exists
WriteHead
WriteBody

Sub WriteHead
	With objFile
		.WriteLine("<html>")
		.WriteLine("<head>")
		.WriteLine("<title>Report</title>")
		.WriteLine("<style>")
		.WriteLine("boby {color:red;font-size:30px;background-color:honeydew}")
		.WriteLine("</style>")
		.WriteLine("</head>")
	End With
End Sub

Sub WriteBody
	With objFile
		.WriteLine("<body>")
		.WriteLine("<h3 aign='center'>Report Generated on "&Date&"</h3>")
		.WriteLine("<p>Orders placed</p>")
		.WriteLine("<table border='5' height='100px'>")
		.WriteLine("<tr><td>Number of Customers</td><td>10</td></tr>")
		.WriteLine("<tr><td>Total Order value</td><td>$1000</td></tr>")
		.WriteLine("</body>")
		.WriteLine("</html>")
	End With
End Sub