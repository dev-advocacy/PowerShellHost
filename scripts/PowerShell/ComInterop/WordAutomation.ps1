Write-Host "This script will open Word, create a new document, and type some text."

# add exception handling
try 
{
    # Create instance of Word
    $word = New-Object -ComObject Word.Application
    $word.Visible = $true
    # Create a new document
    $doc = $word.Documents.Add()
    # Type some text
    $doc.Content.Text = "Hello, World!"

    # close the doc wihtout saving
    $doc.Close([ref]$false)

    # Write-Host "Press any key to close Word"
    #[System.Console]::ReadKey().Key.ToString();

    $word.Quit()
}
catch [System.Exception]
{
     Write-Host "An error occurred: $($_.Exception.Message)"
    #Write-Host 
}
