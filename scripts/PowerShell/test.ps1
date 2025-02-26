Add-Type -AssemblyName PresentationFramework 

function FixedArray {
    $strCustomers = New-Object Object[] 4
    $strCustomers[0] = "Abe"
    $strCustomers[1] = "Ben"
    $strCustomers[2] = "Chris"
    $strCustomers[3] = "Dustin"
    [System.Windows.MessageBox]::Show("strCustomers(0) is $($strCustomers[0])")
}

function DynamicArray {
    $strCustomersNew = New-Object Object[] 4
    $strCustomersNew[0] = "Abe"
    $strCustomersNew[1] = "Ben"
    $strCustomersNew[2] = "Chris"
    $strCustomersNew[3] = "Dustin"
    $strCustomersNew += "Eddie"
    $strCustomersNew += "Fred"

    for ($i=0; $i -lt $strCustomersNew.Length; $i++) {
        [System.Windows.MessageBox]::Show("The element $i is $($strCustomersNew[$i])")
    }
}