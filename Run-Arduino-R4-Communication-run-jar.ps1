$ErrorActionPreference = 'Stop'

$jarPath = Join-Path $PSScriptRoot 'arduino-communication-endpoints\target\arduino-communication-endpoints-0.0.1-SNAPSHOT.jar'
$logDirectory = Join-Path $PSScriptRoot 'logs'
$standardOutputLog = Join-Path $logDirectory 'backend-output.log'
$standardErrorLog = Join-Path $logDirectory 'backend-error.log'

if (-not (Test-Path -LiteralPath $jarPath -PathType Leaf)) {
    throw "Application JAR was not found: $jarPath"
}

# Avoid launching a duplicate server on the application's configured port.
$listener = netstat.exe -ano -p TCP |
    Select-String '^\s*TCP\s+(?:0\.0\.0\.0|\[::\]|127\.0\.0\.1):8080\s+.*LISTENING\s+(\d+)\s*$' |
    Select-Object -First 1

if ($listener) {
    $listenerPid = [int]$listener.Matches[0].Groups[1].Value
    $listenerProcess = Get-Process -Id $listenerPid -ErrorAction SilentlyContinue

    if ($listenerProcess.ProcessName -in @('java', 'javaw')) {
        return
    }

    throw "Port 8080 is already in use by process $listenerPid ($($listenerProcess.ProcessName))."
}

$javaw = Get-Command 'javaw.exe' -ErrorAction SilentlyContinue
if (-not $javaw) {
    throw 'javaw.exe was not found. Install Java 17 or add its bin directory to PATH.'
}

New-Item -ItemType Directory -Path $logDirectory -Force | Out-Null

Start-Process `
    -FilePath $javaw.Source `
    -ArgumentList '-jar', ('"{0}"' -f $jarPath) `
    -WorkingDirectory $PSScriptRoot `
    -WindowStyle Hidden `
    -RedirectStandardOutput $standardOutputLog `
    -RedirectStandardError $standardErrorLog
