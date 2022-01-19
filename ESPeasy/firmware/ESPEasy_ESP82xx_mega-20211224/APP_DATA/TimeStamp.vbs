Dim str
Do While Not WScript.StdIn.AtEndOfStream
  str = WScript.StdIn.ReadLine
  WScript.StdErr.WriteLine "[" & now & "] " & str
Loop

