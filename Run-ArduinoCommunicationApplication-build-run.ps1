[CmdletBinding()]
param(
    [string]$DatabasePath = (Join-Path $PSScriptRoot "data\ArduinoCommunication.db"),
    [int]$Port = 8080,
    [switch]$SkipBuild
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$requiredJavaVersion = 17
$endpointModule = Join-Path $PSScriptRoot "arduino-communication-endpoints"
$jarPath = Join-Path $endpointModule "target\arduino-communication-endpoints-0.0.1-SNAPSHOT.jar"
$mavenWrapper = Join-Path $PSScriptRoot "mvnw.cmd"

if (-not (Get-Command java -ErrorAction SilentlyContinue)) {
    throw "Java is not available on PATH. Install Java $requiredJavaVersion or newer."
}

$javaVersionOutput = (& java --version | Select-Object -First 1) -join ""
$javaVersionMatch = [regex]::Match($javaVersionOutput, '(?<!\d)(?<major>\d+)(?:\.\d+)*')
if (-not $javaVersionMatch.Success) {
    throw "Unable to determine the installed Java version from: $javaVersionOutput"
}
$installedJavaVersion = [int]$javaVersionMatch.Groups['major'].Value
if ($installedJavaVersion -lt $requiredJavaVersion) {
    throw "Java $requiredJavaVersion or newer is required; found Java $installedJavaVersion."
}

$databaseFullPath = [System.IO.Path]::GetFullPath($DatabasePath)
$databaseDirectory = Split-Path -Parent $databaseFullPath
if (-not (Test-Path -LiteralPath $databaseDirectory)) {
    New-Item -ItemType Directory -Path $databaseDirectory -Force | Out-Null
}

if (-not $SkipBuild) {
    Write-Host "Building the production application..."
    & $mavenWrapper -pl arduino-communication-endpoints -am clean package
    if ($LASTEXITCODE -ne 0) {
        throw "Maven build failed with exit code $LASTEXITCODE."
    }
}

if (-not (Test-Path -LiteralPath $jarPath -PathType Leaf)) {
    throw "Application JAR not found at '$jarPath'. Run without -SkipBuild first."
}

# SQLite creates the file on first connection; Hibernate creates/updates its tables.
$env:ARDUINO_DB_PATH = $databaseFullPath.Replace('\', '/')
Write-Host "Starting ArduinoCommunicationApplication on port $Port"
Write-Host "SQLite database: $databaseFullPath"

& java -jar $jarPath "--server.port=$Port"
exit $LASTEXITCODE
