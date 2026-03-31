@echo off
setlocal

set TARGET_DIR=%~dp0source
set EXTENSIONS=cpp h hpp c cmake txt ui qrc json

echo Converting UTF-8 with BOM to UTF-8 (no BOM)...
echo Target: %TARGET_DIR%
echo.

for %%E in (%EXTENSIONS%) do (
    for /r "%TARGET_DIR%" %%F in (*.%%E) do (
        powershell -NoProfile -Command ^
        "$bytes=[System.IO.File]::ReadAllBytes('%%F'); if ($bytes.Length -ge 3 -and $bytes[0]-eq 0xEF -and $bytes[1]-eq 0xBB -and $bytes[2]-eq 0xBF) { [System.IO.File]::WriteAllBytes('%%F',$bytes[3..($bytes.Length-1)]); Write-Host 'Converted:' '%%F' }"
    )
)

echo.
echo Done.
pause