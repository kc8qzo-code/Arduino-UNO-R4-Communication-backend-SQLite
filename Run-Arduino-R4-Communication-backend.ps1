# ============================================
# Start Arduino Communication System Backend (Hidden)
# ============================================

# Run-Backend

$scriptDirectory = "C:\dev\Arduino-UNO-R4-Communication-backend-SQLite"
$scriptName = "Run-Arduino-R4-Communication-run-jar.ps1"

Set-Location $scriptDirectory

Write-Host "Starting Arduino Communication Backend..."

& ".\$scriptName"

Write-Host "Backend started"