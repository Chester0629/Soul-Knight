# 素材複製腳本 — Soul Knight
# 從 1.07/Sprite/ 複製必要素材到 Resources/ 各子目錄
# 使用方式：在 PowerShell 中執行，工作目錄設為 d:\Soul Knight\project\
#
# 使用方式：
#   cd "d:\Soul Knight\project"
#   .\copy_assets.ps1

$ErrorActionPreference = "Stop"

# ── 路徑設定 ──────────────────────────────────────────────────────────────────
$srcDir = "d:\Soul Knight\project\1.07\Sprite"
$dstRoot = "d:\Soul Knight\project\Resources"

# ── 建立目標子目錄 ──────────────────────────────────────────────────────────────
Write-Host "=== 建立 Resources 目錄結構 ===" -ForegroundColor Cyan

$dirs = @(
    "$dstRoot\Characters",
    "$dstRoot\Tiles",
    "$dstRoot\Enemies",
    "$dstRoot\Bullets",
    "$dstRoot\Boss",
    "$dstRoot\Objects",
    "$dstRoot\UI"
)
foreach ($dir in $dirs) {
    if (-not (Test-Path $dir)) {
        New-Item -ItemType Directory -Path $dir -Force | Out-Null
        Write-Host "  建立: $dir" -ForegroundColor Green
    } else {
        Write-Host "  已存在: $dir" -ForegroundColor Gray
    }
}

# ── 複製函式（含錯誤回報）───────────────────────────────────────────────────────
function Copy-Asset {
    param(
        [string]$FileName,
        [string]$DestDir
    )
    $src = Join-Path $srcDir $FileName
    $dst = Join-Path $DestDir $FileName
    if (Test-Path $src) {
        Copy-Item -Path $src -Destination $dst -Force
        Write-Host "  ✅ $FileName" -ForegroundColor Green
    } else {
        Write-Host "  ❌ 找不到：$src" -ForegroundColor Red
    }
}

# ── 角色 c01（M1 Hardcode 使用）────────────────────────────────────────────────
Write-Host "`n=== 角色 c01（幀 0-8）===" -ForegroundColor Cyan
for ($i = 0; $i -le 8; $i++) {
    Copy-Asset "c01_$i.png" "$dstRoot\Characters"
}

# ── 角色 c02、c03（M5 選角用，先一起複製備用）──────────────────────────────────
Write-Host "`n=== 角色 c02（幀 0-8）===" -ForegroundColor Cyan
for ($i = 0; $i -le 8; $i++) {
    Copy-Asset "c02_$i.png" "$dstRoot\Characters"
}
Write-Host "`n=== 角色 c03（幀 0-8）===" -ForegroundColor Cyan
for ($i = 0; $i -le 8; $i++) {
    Copy-Asset "c03_$i.png" "$dstRoot\Characters"
}

# ── 地板 Tile（第 1 層）────────────────────────────────────────────────────────
Write-Host "`n=== 地板 Tile ===" -ForegroundColor Cyan
Copy-Asset "f101.png" "$dstRoot\Tiles"

# ── 牆壁 Tile（第 1 層，先複製 001-010 涵蓋頂/底/側/角落）────────────────────────
Write-Host "`n=== 牆壁 Tile (w001-w010) ===" -ForegroundColor Cyan
for ($i = 1; $i -le 10; $i++) {
    $name = "w{0:D3}.png" -f $i
    Copy-Asset $name "$dstRoot\Tiles"
}

# ── 敵人 enemy22（M2 測試用，7 幀）────────────────────────────────────────────
Write-Host "`n=== 敵人 enemy22（幀 0-6）===" -ForegroundColor Cyan
for ($i = 0; $i -le 6; $i++) {
    Copy-Asset "enemy22_$i.png" "$dstRoot\Enemies"
}

# ── Boss boss08（M4 用，8 幀）────────────────────────────────────────────────
Write-Host "`n=== Boss boss08（幀 0-7）===" -ForegroundColor Cyan
for ($i = 0; $i -le 7; $i++) {
    Copy-Asset "boss08_$i.png" "$dstRoot\Boss"
}

# ── 傳送陣特效 effect04（M4 用，幀 5-12）────────────────────────────────────────
Write-Host "`n=== 傳送陣 effect04（幀 5-12）===" -ForegroundColor Cyan
for ($i = 5; $i -le 12; $i++) {
    Copy-Asset "effect04_$i.png" "$dstRoot\Objects"
}

# ── UI 元件（先複製 0-40，血量條等）────────────────────────────────────────────
Write-Host "`n=== UI 元件（ui_0 - ui_40）===" -ForegroundColor Cyan
for ($i = 0; $i -le 40; $i++) {
    Copy-Asset "ui_$i.png" "$dstRoot\UI"
}

# ── 子彈（先複製 0-20 備用）─────────────────────────────────────────────────────
Write-Host "`n=== 子彈素材（bullet_0 - bullet_20）===" -ForegroundColor Cyan
for ($i = 0; $i -le 20; $i++) {
    Copy-Asset "bullet_$i.png" "$dstRoot\Bullets"
}

# ── 哥布林敵方攻擊素材（M2 用）────────────────────────────────────────────────────
# ⚠️ 這三個素材的原始檔名含中文，直接放在 1.07\ 根目錄（不在 Sprite\ 子目錄）
# 使用 Get-ChildItem 萬用字元搜尋，再以大小特徵對應目標檔名，避免中文路徑硬編碼。
Write-Host "`n=== 哥布林攻擊素材（M2 用）===" -ForegroundColor Cyan

$srcRoot107 = "d:\Soul Knight\project\1.07"
$bulletDst  = "$dstRoot\Bullets"

# 比對規則：(大小特徵萬用字元, 目標檔名, 說明)
$goblinAssets = @(
    @("28px*", "enemy_bullet_pistol.png", "PistolGoblin 圓形子彈"),
    @("*0.png", "enemy_stab.png",         "SpearGoblin 突刺(突刺动图0)"),
    @("42px*", "enemy_arrow.png",         "ArcherGoblin 箭矢")
)

foreach ($entry in $goblinAssets) {
    $pattern = $entry[0]
    $dstName = $entry[1]
    $desc    = $entry[2]

    $found = Get-ChildItem -Path $srcRoot107 -Filter $pattern -File -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($found) {
        Copy-Item -Path $found.FullName -Destination "$bulletDst\$dstName" -Force
        Write-Host "  ✅ $desc  ($($found.Name) -> $dstName)" -ForegroundColor Green
    } else {
        Write-Host "  ❌ 找不到符合 [$pattern] 的檔案於 $srcRoot107" -ForegroundColor Red
    }
}


# ── 完成統計 ───────────────────────────────────────────────────────────────────
Write-Host "`n=== 複製完成 ===" -ForegroundColor Cyan
Write-Host "目的地：$dstRoot" -ForegroundColor Yellow

# 統計每個目錄的檔案數
foreach ($dir in $dirs) {
    $count = (Get-ChildItem $dir -File -ErrorAction SilentlyContinue).Count
    $dirName = Split-Path $dir -Leaf
    Write-Host "  $dirName : $count 個檔案" -ForegroundColor White
}

Write-Host "`n✅ 素材複製完成！請確認上方沒有 ❌ 錯誤，再回報結果。" -ForegroundColor Green
