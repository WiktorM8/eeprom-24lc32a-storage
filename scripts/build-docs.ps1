param(
  [string]$DocsDir = "docs",
  [string]$Engine = "pdflatex"
)

if (-not (Test-Path -LiteralPath $DocsDir)) {
  Write-Error "Brak katalogu: $DocsDir"
  exit 1
}

$texFiles = Get-ChildItem -LiteralPath $DocsDir -Recurse -Filter "*.tex" | Sort-Object FullName

foreach ($file in $texFiles) {
  Write-Host "=== Kompilacja: $($file.FullName) ==="
  $pdfPath = [System.IO.Path]::ChangeExtension($file.FullName, ".pdf")
  Push-Location -LiteralPath $file.DirectoryName
  try {
    for ($i = 1; $i -le 2; $i++) {
      $ErrorActionPreference = "Continue"
      $out = & $Engine -interaction=nonstopmode -halt-on-error $file.Name 2>&1 | ForEach-Object { $_.ToString() }
      $code = $LASTEXITCODE
      if ($code -ne 0 -or -not (Test-Path -LiteralPath $pdfPath)) {
        $errors = $out | Where-Object { $_ -match "^! |^l\.[0-9]+" }
        if ($errors) {
          $errors | ForEach-Object { Write-Host $_ -ForegroundColor Red }
        } else {
          $out | Select-Object -Last 20 | ForEach-Object { Write-Host $_ -ForegroundColor Red }
        }
        Get-ChildItem -LiteralPath $file.DirectoryName -File |
          Where-Object { $_.Extension -notin @(".tex", ".pdf") } |
          Remove-Item -Force -ErrorAction SilentlyContinue
        exit 1
      }
    }
  }
  finally {
    Pop-Location
  }
}

Get-ChildItem -LiteralPath $DocsDir -Recurse -File |
  Where-Object { $_.Extension -notin @(".tex", ".pdf") } |
  Remove-Item -Force

Write-Host "Gotowe. Skompilowano pliki .tex w: $DocsDir"