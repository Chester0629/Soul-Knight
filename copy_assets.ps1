# copy_assets.ps1 -- Soul Knight Step 1.1
# Copies required sprites from 1.07/Sprite/ to Resources/ subdirectories.
# Prints a missing-file audit report at the end.
#
# Usage:
#   cd "d:\Soul Knight\project"
#   powershell -ExecutionPolicy Bypass -File copy_assets.ps1

$ErrorActionPreference = "Stop"

$srcDir    = "d:\Soul Knight\project\1.07\Sprite"
$srcRoot   = "d:\Soul Knight\project\1.07"
$dstRoot   = "d:\Soul Knight\project\Resources"

# ---------- 1. Create subdirectories ----------------------------------------
$subDirs = @("Characters","Tiles","Enemies","Bullets","Boss","Objects","UI","Weapons")
Write-Host "=== Creating Resources subdirectories ===" -ForegroundColor Cyan
foreach ($d in $subDirs) {
    $path = Join-Path $dstRoot $d
    if (-not (Test-Path $path)) {
        New-Item -ItemType Directory -Path $path -Force | Out-Null
        Write-Host "  [NEW] $d" -ForegroundColor Green
    } else {
        Write-Host "  [OK ] $d already exists" -ForegroundColor Gray
    }
}

# ---------- 2. Helper: copy single file if destination missing ---------------
function Copy-Asset {
    param([string]$Src, [string]$Dst, [string]$Label)
    if (-not (Test-Path $Dst)) {
        if (Test-Path $Src) {
            Copy-Item -Path $Src -Destination $Dst -Force
            Write-Host "  [COPY] $Label" -ForegroundColor Green
        } else {
            Write-Host "  [MISS] $Label  (src: $Src)" -ForegroundColor Red
        }
    }
    # silently skip if already exists
}

# ---------- 3. Characters -- c01, c02, c03 (frames 0-8) ---------------------
Write-Host "`n=== Characters ===" -ForegroundColor Cyan
foreach ($ch in @("c01","c02","c03")) {
    for ($i = 0; $i -le 8; $i++) {
        Copy-Asset "$srcDir\${ch}_${i}.png" "$dstRoot\Characters\${ch}_${i}.png" "${ch}_${i}.png"
    }
}

# ---------- 4. Tiles ---------------------------------------------------------
Write-Host "`n=== Tiles ===" -ForegroundColor Cyan
Copy-Asset "$srcDir\f101.png" "$dstRoot\Tiles\f101.png" "f101.png"
Copy-Asset "$srcDir\w001.png" "$dstRoot\Tiles\w001.png" "w001.png"
Copy-Asset "$srcDir\w004.png" "$dstRoot\Tiles\w004.png" "w004.png"

# ---------- 5. Enemies -- enemy22 (frames 0-6) --------------------------------
Write-Host "`n=== Enemies ===" -ForegroundColor Cyan
for ($i = 0; $i -le 6; $i++) {
    Copy-Asset "$srcDir\enemy22_${i}.png" "$dstRoot\Enemies\enemy22_${i}.png" "enemy22_${i}.png"
}

# ---------- 6. Boss -- boss08 (frames 0-7) ------------------------------------
Write-Host "`n=== Boss ===" -ForegroundColor Cyan
for ($i = 0; $i -le 7; $i++) {
    Copy-Asset "$srcDir\boss08_${i}.png" "$dstRoot\Boss\boss08_${i}.png" "boss08_${i}.png"
}

# ---------- 7. Objects -- effect04 (frames 5-12) -----------------------------
Write-Host "`n=== Objects (teleporter) ===" -ForegroundColor Cyan
for ($i = 5; $i -le 12; $i++) {
    Copy-Asset "$srcDir\effect04_${i}.png" "$dstRoot\Objects\effect04_${i}.png" "effect04_${i}.png"
}

# ---------- 8. UI -- ui_0 to ui_20 + Settlement screen -----------------------
Write-Host "`n=== UI ===" -ForegroundColor Cyan
for ($i = 0; $i -le 20; $i++) {
    Copy-Asset "$srcDir\ui_${i}.png" "$dstRoot\UI\ui_${i}.png" "ui_${i}.png"
}
# Settlement screen.png (GameOver / Victory background)
Copy-Asset "$srcDir\Settlement screen.png" "$dstRoot\UI\Settlement screen.png" "Settlement screen.png"

# ---------- 9. Weapons (rename on copy) --------------------------------------
Write-Host "`n=== Weapons (renamed) ===" -ForegroundColor Cyan
Copy-Asset "$srcDir\weapons_19.png"  "$dstRoot\Weapons\weapon_pistol.png"  "weapons_19 -> weapon_pistol.png"
Copy-Asset "$srcDir\weapons_18.png"  "$dstRoot\Weapons\weapon_shotgun.png" "weapons_18 -> weapon_shotgun.png"
Copy-Asset "$srcDir\weapons2_78.png" "$dstRoot\Weapons\weapon_laser.png"   "weapons2_78 -> weapon_laser.png"

# ---------- 10. Bullets (goblin attack sprites -- in 1.07\ root) -------------
Write-Host "`n=== Bullets (enemy attack sprites) ===" -ForegroundColor Cyan

# Find by glob to avoid hardcoding Chinese filenames in ASCII script
$bulletMap = @(
    @{ Pattern = "28px*";  Dst = "$dstRoot\Bullets\enemy_bullet_pistol.png"; Label = "enemy_bullet_pistol.png" }
    @{ Pattern = "*0.png"; Dst = "$dstRoot\Bullets\enemy_stab.png";          Label = "enemy_stab.png" }
    @{ Pattern = "42px*";  Dst = "$dstRoot\Bullets\enemy_arrow.png";         Label = "enemy_arrow.png" }
)
foreach ($b in $bulletMap) {
    if (-not (Test-Path $b.Dst)) {
        $found = Get-ChildItem -Path $srcRoot -Filter $b.Pattern -File -ErrorAction SilentlyContinue | Select-Object -First 1
        if ($found) {
            Copy-Item -Path $found.FullName -Destination $b.Dst -Force
            Write-Host "  [COPY] $($b.Label)  (from $($found.Name))" -ForegroundColor Green
        } else {
            Write-Host "  [MISS] $($b.Label)  (no file matching $($b.Pattern) in $srcRoot)" -ForegroundColor Red
        }
    }
}

# ---------- 11. Directory summary --------------------------------------------
Write-Host "`n=== File count per directory ===" -ForegroundColor Cyan
foreach ($d in $subDirs) {
    $path  = Join-Path $dstRoot $d
    $count = (Get-ChildItem $path -File -ErrorAction SilentlyContinue).Count
    Write-Host ("  {0,-12} : {1} files" -f $d, $count)
}

# ---------- 12. Full audit: report any missing expected files ----------------
Write-Host "`n=== Audit Report ===" -ForegroundColor Cyan

$expected = @()
# Characters
foreach ($ch in @("c01","c02","c03")) {
    0..8 | ForEach-Object { $expected += "$dstRoot\Characters\${ch}_${_}.png" }
}
# Tiles
$expected += "$dstRoot\Tiles\f101.png"
$expected += "$dstRoot\Tiles\w001.png"
$expected += "$dstRoot\Tiles\w004.png"
# Enemies
0..6 | ForEach-Object { $expected += "$dstRoot\Enemies\enemy22_${_}.png" }
# Boss
0..7 | ForEach-Object { $expected += "$dstRoot\Boss\boss08_${_}.png" }
# Objects
5..12 | ForEach-Object { $expected += "$dstRoot\Objects\effect04_${_}.png" }
# UI
0..20 | ForEach-Object { $expected += "$dstRoot\UI\ui_${_}.png" }
$expected += "$dstRoot\UI\Settlement screen.png"
# Weapons
$expected += "$dstRoot\Weapons\weapon_pistol.png"
$expected += "$dstRoot\Weapons\weapon_shotgun.png"
$expected += "$dstRoot\Weapons\weapon_laser.png"
# Bullets
$expected += "$dstRoot\Bullets\enemy_bullet_pistol.png"
$expected += "$dstRoot\Bullets\enemy_stab.png"
$expected += "$dstRoot\Bullets\enemy_arrow.png"

$missing = $expected | Where-Object { -not (Test-Path $_) }

if ($missing.Count -eq 0) {
    Write-Host "All expected assets present -- OK" -ForegroundColor Green
} else {
    Write-Host "MISSING $($missing.Count) file(s):" -ForegroundColor Red
    $missing | ForEach-Object { Write-Host ("  MISSING: " + $_.Replace($dstRoot, "Resources")) -ForegroundColor Red }
}
Write-Host "=== Done ===" -ForegroundColor Cyan
