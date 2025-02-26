# Get the script folder
$scriptPath = Split-Path -parent $MyInvocation.MyCommand.Definition
#concat with Interop.Microsoft.Office.Interop.Word.dll
$comInterOp = $scriptPath + "\Interop.Microsoft.Office.Interop.Word.dll"
Add-Type -LiteralPath $comInterOp;

try {
    $application = New-Object -ComObject word.application;
    $application.Visible = $true;
    $document = $application.Documents.Add();
   

    # create an array of 1000 random strings
    $randomStrings = 1..1000 | ForEach-Object { [System.Guid]::NewGuid().ToString() };

    #insert the strings into the document
    $randomStrings | ForEach-Object { $document.Content.InsertAfter($_) };
    # Get the desktop folder
    $folder = [System.Environment]::GetFolderPath("Desktop");
    $document.SaveAs($folder + "\HelloWorld.docx");
}
catch {
    Write-Error "An error occurred: $_"
}
finally {
    if ($application) {
        $application.Quit()
    }
}