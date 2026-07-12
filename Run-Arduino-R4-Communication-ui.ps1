# ============================================
# Start Arduino Communication System UI (Hidden)
# ============================================

$uiDirectory   = "C:\dev\Arduino-UNO-R4-Communication-ui"

# Start Angular hidden
Start-Process powershell.exe `
    -ArgumentList "-ExecutionPolicy Bypass -NoProfile -Command `"Set-Location '$uiDirectory'; npm run serve:prod`"" `
    -WindowStyle Hidden