Option Explicit
Call FixedArray
Call DynamicArray

Sub FixedArray
	Dim strCustomers(3)
	strCustomers(0) = "Abe"
	strCustomers(1) = "Ben"
	strCustomers(2) = "Chris"
	strCustomers(3) = "Dustin"
	Wscript.Echo "strCustomers(0) is "& strCustomers(0)
End Sub

Sub DynamicArray
	Dim strCustomersNew()
	Redim strCustomersNew(3)
	strCustomersNew(0) = "Abe"
	strCustomersNew(1) = "Ben"
	strCustomersNew(2) = "Chris"
	strCustomersNew(3) = "Dustin"
	Redim preserve strCustomersNew(5)
	strCustomersNew(4) = "Eddie"
	strCustomersNew(5) = "Fred"
	Dim i
	for i = 0 to UBound(strCustomersNew)
		WScript.Echo "The element "&i &" is "&strCustomersNew(i)
	Next
End Sub