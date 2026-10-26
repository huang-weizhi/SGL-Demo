# Thermal + controller simulation (mirrors heat_control.c integer arithmetic)
# plant: C=400 J/degC, loss k=0.41 W/degC, Pmax=300 W, sensor lag 15 s
param([string]$mode = 'new', [int]$setC = 240, [int]$minutes = 40, [double]$k = 0.41, [double]$Cm = 400.0, [double]$tauSen = 15.0, [int]$trimLim = 400, [int]$FFD = 8, [int]$KP = 20)

$C = $Cm; $Pmax = 300.0; $amb = 25.0; $tauS = $tauSen
$dt = 0.1
$steps = [int]($minutes * 60 / $dt)

# new law parameters (heat_control.h)
$FF_DIV = $FFD; $KPX10 = $KP; $KIX1000 = 20; $ILIM = $trimLim; $IWINMS = 100000; $IRATE = 2; $DB = 1; $OUTMAX = 100
# old (current firmware) parameters
$oldKPX10 = 15; $oldKIX1000 = 25; $oldILIM = 800; $oldWARM = 70; $oldBAND = 40

$Tp = $amb; $Ts = $amb
$iTerm = 0; $iAcc = 0; $iRef = [int][math]::Round($Ts); $winAcc = 0; $iFrozen = $true
$duty = 0; $oldDuty = 0
$maxT = $Ts; $maxTt = 0.0
$log = @()

for ($n = 0; $n -lt $steps; $n++) {
    $t = $n * $dt
    $Tmeas = [int][math]::Round($Ts)          # 1 degC sensor resolution
    $e = $setC - $Tmeas

    if ($mode -eq 'new') {
        # ---- port of heat_pi_step() ----
        $base = [int][math]::Floor([double]$Tmeas / $FF_DIV) * 10
        if ($base -lt 0) { $base = 0 }
        $base += $KPX10 * $e
        $winAcc += 100
        if ($winAcc -ge $IWINMS) {
            $moved = $Tmeas - $iRef
            $winAcc = 0; $iRef = $Tmeas
            $iFrozen = (($moved -ge $IRATE) -or ($moved -le -$IRATE))
        }
        $limit = $OUTMAX * 10
        if ((($base + $iTerm) -ge $limit -and $e -gt 0) -or (($base + $iTerm) -le 0 -and $e -lt 0)) {
            # anti-windup
        } elseif ((-not $iFrozen) -and (($e -gt $DB) -or ($e -lt -$DB))) {
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
    }
    else {
        # ---- port of the CURRENT firmware PI (units bug) ----
        if ($e -gt $oldBAND) {
            $iTerm = 0; $duty = $oldWARM
        }
        elseif ($e -le $DB -and $e -ge -$DB) {
            $duty = $oldDuty
        }
        else {
            if ($e -gt 0) {
                $iTerm += [int][math]::Floor($oldKIX1000 * $e / 10.0)
                if ($iTerm -gt $oldILIM) { $iTerm = $oldILIM }
            } else {
                $sub = [int][math]::Floor(-$oldKIX1000 * $e / 10.0)
                $iTerm = if ($iTerm -gt $sub) { $iTerm - $sub } else { 0 }
            }
            $out = [int][math]::Floor($oldKPX10 * $e / 10.0) + $iTerm
            if ($out -lt 0) { $out = 0 }
            if ($out -gt 1000) { $out = 1000 }
            $duty = [int]($out / 10)
        }
    }
    $oldDuty = $duty

    # ---- plant ----
    $P = $Pmax * $duty / 100.0
    $Tp += $dt * ($P - $k * ($Tp - $amb)) / $C
    $Ts += $dt * ($Tp - $Ts) / $tauS

    if ($Ts -gt $maxT) { $maxT = $Ts; $maxTt = $t }
    if (($n % 600) -eq 0) {   # every 60 s
        $log += [pscustomobject]@{ t_min = [math]::Round($t / 60, 1); T = [math]::Round($Ts, 1); duty = $duty; i = $iTerm }
    }
}

$hdr = "$mode  set=$setC  k=$k  C=$C  Pmax=$Pmax  tau_sensor=$tauS"
Write-Output "==== $hdr ===="
Write-Output ("max sensor temp = {0:N1} degC at {1:N1} min   (overshoot {2:N1} degC)" -f $maxT, ($maxTt / 60), ($maxT - $setC))
Write-Output ("final temp = {0:N1} degC,  final duty = {1} %,  i_term = {2}" -f $Ts, $duty, $iTerm)
$smp = $log | Where-Object { $_.t_min -in @(2, 4, 6, 8, 10, 15, 20, 30, 40) }
$smp | Format-Table -AutoSize | Out-String -Width 100
