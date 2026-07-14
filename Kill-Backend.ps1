# Kill-8080.ps1

$port = 8080

Write-Host "Searching for process using port $port..."

# Find the PID using the port
$connection = Get-NetTCPConnection -LocalPort $port -State Listen -ErrorAction SilentlyContinue

if ($null -eq $connection) {
    Write-Host "No process is listening on port $port."
    exit
}

$processId = $connection.OwningProcess

$process = Get-CimInstance Win32_Process -Filter "ProcessId = $processId"

Write-Host "PID: $($process.ProcessId)"
Write-Host "Process: $($process.Name)"
Write-Host "Command Line:"
Write-Host $process.CommandLine

Write-Host " "

$answer = Read-Host "Kill this process? (Y/N)"

if ($answer -match "^[Yy]") {
	Stop-Process -Id $process.ProcessId -Force
	Write-Host "Process $process.ProcessId has been terminated."
}
else {
	Write-Host "Operation cancelled."
}

Read-Host -Prompt "Press Enter to exit"