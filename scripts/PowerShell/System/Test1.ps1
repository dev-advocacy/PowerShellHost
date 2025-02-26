#sample Test
#enum all %temp% file and print to the console

$TempPath = [System.IO.Path]::GetTempPath()

# print to the console
Write-Output $TempPath
Get-ChildItem -Path $TempPath -File | ForEach-Object { Write-Output $_.FullName }





