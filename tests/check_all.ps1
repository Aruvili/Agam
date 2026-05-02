$AGAMC = "./build/bin/agamc.exe"
$FILES = Get-ChildItem -Path "." -Recurse -Filter "*.agam" | Where-Object { 
    $_.FullName -notmatch "build" -and 
    $_.FullName -notmatch "tests\\samples\\errors" -and 
    $_.FullName -notmatch "std\\" -and
    $_.FullName -notmatch "install_test\\" -and
    $_.FullName -notmatch "tests\\diagnostics\\" -and
    $_.FullName -notmatch "tests\\integration\\math\.agam" -and
    $_.FullName -notmatch "fail\.agam" 
}

Write-Host "Checking Agam files..." -ForegroundColor Cyan
$passCount = 0
$failCount = 0
$totalCount = 0

foreach ($file in $FILES) {
    $totalCount++
    Write-Host "----------------------------------------"
    Write-Host "Checking $($file.FullName)..."
    
    # Run agamc.exe and capture output
    $process = Start-Process -FilePath $AGAMC -ArgumentList $file.FullName -NoNewWindow -PassThru -Wait -RedirectStandardError "./tests/last_error.txt"
    
    if ($process.ExitCode -eq 0) {
        Write-Host "PASS" -ForegroundColor Green
        $passCount++
    } else {
        $errorMsg = ""
        if (Test-Path "./tests/last_error.txt") {
            $errorMsg = Get-Content "./tests/last_error.txt" -Raw
        }
        
        if ($errorMsg -match "மைய") {
            Write-Host "SKIP (No main function)" -ForegroundColor Yellow
        } else {
            Write-Host "FAIL" -ForegroundColor Red
            $failCount++
            Write-Host ($errorMsg | Out-String)
        }
    }
}

Write-Host "========================================"
Write-Host "Total: $totalCount, PASS: $passCount, FAIL: $failCount"
