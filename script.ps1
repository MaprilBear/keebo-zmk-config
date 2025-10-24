$filePath = "C:\Users\April\Documents\Repos\Keebo\sw\zmk.uf2"
$commandToRun = "Move-Item", "-Path", $filePath, "-Destination", "F:\" # Command and its arguments as an array

while($true){
if (Test-Path $filePath) {
    Start-Sleep -Milliseconds 500
    Write-Host "UF2 file found, flashing board..."
    Move-Item -Path C:\Users\April\Documents\Repos\Keebo\sw\zmk.uf2 -Destination F:\
    Write-Host "Done!"
} 
}
