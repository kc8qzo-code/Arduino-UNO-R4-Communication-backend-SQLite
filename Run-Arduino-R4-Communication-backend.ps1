# ============================================
# Start Arduino Communication System Backend (Hidden)
# ============================================

# Run-Backend.ps1

$scriptDirectory = "C:\dev\Arduino-UNO-R4-Communication-backend-SQLite"
$scriptName = "Run-ArduinoCommunicationApplication.ps1"
$scriptPath = Join-Path $scriptDirectory $scriptName

Set-Location $scriptDirectory

Write-Host "Starting Arduino Communication Backend..."

& ".\Run-ArduinoCommunicationApplication.ps1"

Write-Host "Backend started"