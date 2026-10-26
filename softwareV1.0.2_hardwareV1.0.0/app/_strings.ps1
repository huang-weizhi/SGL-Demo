$root='d:\Work Area\Code For WorkBench\FW_CZ01\softwareV1.0.0_hardwareV1.0.0\app\project'
$res=@()
Get-ChildItem -LiteralPath $root -Recurse -File -Include '*.c','*.h' | Where-Object { $_.FullName -notlike '*\MDK_V5\*' } | ForEach-Object {
  $f=$_
  $n=0
  Get-Content -LiteralPath $f.FullName -ErrorAction SilentlyContinue | ForEach-Object {
    $n++
    $line=$_
    if($line -match '"([^"]{6,})"'){
      foreach($mm in [regex]::Matches($line,'"([^"]{6,})"')){
        $res += [pscustomobject]@{File=$f.Name;Line=$n;Len=$mm.Groups[1].Value.Length;Text=$mm.Groups[1].Value.Substring(0,[Math]::Min(60,$mm.Groups[1].Value.Length))}
      }
    }
  }
}
$o='d:\Work Area\Code For WorkBench\FW_CZ01\softwareV1.0.0_hardwareV1.0.0\app\project\MDK_V5\_str_long.txt'
$sb=New-Object System.Text.StringBuilder
[void]$sb.AppendLine("total long string literals: $($res.Count)")
$g=$res | Group-Object File | ForEach-Object { [pscustomobject]@{K=$_.Name;Cnt=$_.Count;Sum=($_.Group | Measure-Object Len -Sum).Sum} }
[void]$sb.AppendLine("=== by file (count, total chars) ===")
foreach($r in ($g | Sort-Object Sum -Descending)){ [void]$sb.AppendLine(("{0,6} chars {1,5} lits  {2}" -f $r.Sum,$r.Cnt,$r.K)) }
[void]$sb.AppendLine("=== longest 40 ===")
foreach($r in ($res | Sort-Object Len -Descending | Select-Object -First 40)){ [void]$sb.AppendLine(("{0,5}  {1}:{2}  {3}" -f $r.Len,$r.File,$r.Line,$r.Text)) }
Set-Content -LiteralPath $o -Value $sb.ToString() -Encoding UTF8
