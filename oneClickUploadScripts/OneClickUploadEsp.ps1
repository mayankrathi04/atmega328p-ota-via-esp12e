for($i=15;$i -le 15;$i++){
    try {
        $ips = [System.Net.Dns]::GetHostAddresses("Node"+$i+".local").IPAddressToString
        Write-Output "Uploading code to Node$i ip:$ips"
        $proc=Start-Process -FilePath "python" -ArgumentList ".\espota.py","-i",$ips,"-p","8266","-I", "192.168.1.200", "-f", "code.bin", "-a", "qwerty123.","-r" -Wait -NoNewWindow  
    }
    catch {
        Write-Output "Cannot Find Node$i"
    }
}
Write-Host -NoNewLine 'Press any key to continue...';
$null = $Host.UI.RawUI.ReadKey('NoEcho,IncludeKeyDown');
