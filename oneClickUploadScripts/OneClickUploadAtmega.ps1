for($i=1;$i -le 15;$i++){
    try {
        $ips = [System.Net.Dns]::GetHostAddresses("Node"+$i+".local").IPAddressToString
        Write-Output "Uploading Atmega code to Node$i ip:$ips"
        Start-Process -FilePath "python" -ArgumentList ".\atmegaota.py","-i",$ips,"-f", "atmegaCode.hex" -Wait -NoNewWindow  
    }
    catch {
        Write-Output "Cannot Find Node$i"
    }
}
Write-Output 'Press any key to continue...';
$null=$Host.UI.RawUI.ReadKey('NoEcho,IncludeKeyDown');
