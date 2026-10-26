# window diagnostics for the new law
param([int]$setC = 240, [int]$minutes = 12)

$C = 400.0; $Pmax = 300.0; $amb = 25.0; $tauS = 15.0
$dt = 0.1
$steps = [int]($minutes * 60 / $dt)
$FF_DIV = 8; $KPX10 = 20; $KIX1000 = 20; $ILIM = 200; $IWINMS = 100000; $IRATE = 2; $DB = 1; $OUTMAX = 100

$Tp = $amb; $Ts = $amb
$iTerm = 0; $iAcc = 0; $iRef = 25; $winAcc = 0; $iFrozen = $true
$duty = 0
$log = @()

for ($n = 0; $n -lt $steps; $n++) {
    $t = $n * $dt
    $Tmeas = [int][math]::Round($Ts)
    $e = $setC - $Tmeas
    $base = [int][math]::Floor([double]$Tmeas / $FF_DIV) * 10
    if ($base -lt 0) { $base = 0 }
    $base += $KPX10 * $e
    $winAcc += 100
    $moved = 0
    $winEdge = $false
    if ($winAcc -ge $IWINMS) {
        $moved = $Tmeas - $iRef
        $winAcc = 0; $iRef = $Tmeas
        $iFrozen = (($moved -ge $IRATE) -or ($moved -le -$IRATE))
        $winEdge = $true
    }
    $limit = $OUTMAX * 10
    $integrated = $false
    if ((($base + $iTerm) -ge $limit -and $e -gt 0) -or (($base + $iTerm) -le 0 -and $e -lt 0)) {
    } elseif ((-not $iFrozen) -and (($e -gt $DB) -or ($e -lt -$DB))) {
        $integrated = $true
        $iAcc += $KIX1000 * $e
        while ($iAcc -ge 1000) { $iAcc -= 1000; $iTerm++ }
        while ($iAcc -le -1000) { $iAcc += 1000; $iTerm-- }
        if ($iTerm -gt $ILIM) { $iTerm = $ILIM }
        if ($iTerm -lt -$ILIM) { $iTerm = -$ILIM }
    }
    $out = $base + $iTerm
    if ($out -lt 0) { $out = 0 }
    if ($out -gt $limit) { $out = $limit }
    $duty = [int]($out / 10)

    $P = $Pmax * $duty / 100.0
    $Tp += $dt * ($P - 0.41 * ($Tp - $amb)) / $C
    $Ts += $dt * ($Tp - $Ts) / $tauS

    if ($winEdge) {
        $log += [pscustomobject]@{ time_s = [math]::Round($t, 0); temp = $Tmeas; err = $e; moved = $moved;
            frozen = $iFrozen; integ = $integrated; i = $iTerm; duty = $duty; base = $base }
    }
}
$log | Format-Table -AutoSize | Out-String -Width 120
