$port=new-Object System.IO.Ports.SerialPort COM3,115200,None,8,one
$port.Open()
$port.WriteLine("...")
  $message=$port.ReadLine()
  Write-Output $message
$port.Close()

