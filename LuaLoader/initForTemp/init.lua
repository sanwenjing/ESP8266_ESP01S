function getString(body)
if(body) then
return "<html><head><title>ESP-01s</title></head><body>"..body.."</body></html>"
else
return "<html><head><title>ESP-01s</title></head><body>Bad actions! </body></html>"
end
end

function setWifi(ap,pwd)
file.open("wifi.lua", "w+")    -- 写入wifi配?
file.writeline("ap="..ap.."\r\npwd="..pwd)
file.close()
end


function getConf()--读WIFI配置
local line=nil
local conf={}
if(file.open("wifi.lua","r")) then
	for i=1,2 do
		line=file.readline()
		--print(line)
		if(line) then 
			for k, v in string.gmatch(line, "([%w_]+)=([%w_]+)") do
                		conf[k] = v
						--print(k,v)
            		end 
		end
	end
file.close()
return conf or "nothing"
end
return conf
end


wifi.sta.disconnect()
tmr.delay(100)
wifi.setmode(wifi.STATION) 
if(getConf().ap and getConf().pwd) then
wifi.sta.config(getConf().ap,getConf().pwd)
else
wifi.sta.config("yourwifi","yourpassword") -- connecting to server
end
--wifi.sta.connect()
wifi.sta.autoconnect(1)--Auto connectting!

led1 = 3
led2 = 4
gpio.mode(led1, gpio.OUTPUT)
gpio.mode(led2, gpio.OUTPUT)
srv=net.createServer(net.TCP,1)
srv:listen(80,function(conn)
    conn:on("receive", function(client,request)
		local buf = "";
        buf = buf.."HTTP/1.1 200 OK\n\n"
        local _, _, method, path, vars = string.find(request, "([A-Z]+) (.+)?(.+) HTTP");
        if(method == nil)then
            _, _, method, path = string.find(request, "([A-Z]+) (.+) HTTP");
        end
        local _GET = {}
        if (vars ~= nil)then
            for k, v in string.gmatch(vars, "([%w_]+)=([%w_]+)&*") do
                _GET[k] = v
            end
        end
		local txt=_GET.action
		if(txt == "ON1")then
              gpio.write(led1, gpio.HIGH)
			  print("led1 on")
        elseif(txt == "OFF1")then
              gpio.write(led1, gpio.LOW)
			print("led1 off")
		elseif(txt == "ON2")then
              gpio.write(led2, gpio.HIGH)
			print("led2 on")
        elseif(txt == "OFF2")then
              gpio.write(led2, gpio.LOW)
			print("led2 off")
		elseif(txt == "getid")then --getid
			  txt=node.chipid()
			print("SN:"..txt)
		elseif(txt == "delay")then --delay 5s
			  gpio.write(led1, gpio.HIGH)
			  tmr.delay(5000000)
			  gpio.write(led1, gpio.LOW)
			print("delay 5S")
		elseif(txt == "delay1")then --delay 5s
			  gpio.write(led1, gpio.LOW)
			  tmr.delay(5000000)
			  gpio.write(led1, gpio.HIGH)
			print("delay 5S")
        elseif(txt == "states")then
				txt = ""
				if(_GET.PIN and _GET.PIN=="IO0") then
				--For homebridge
				if(gpio.read(3)==1) then txt="0" else txt="1" end
				client:send(buf..txt);
				client:close();
				collectgarbage();
				return;
				elseif(_GET.PIN and _GET.PIN=="IO2") then
				--For homebridge
				if(gpio.read(4)==1) then txt="0" else txt="1" end
				client:send(buf..txt);
				client:close();
				collectgarbage();
				return;
				else
				txt=txt.."IO0="..gpio.read(3).."<br />"
				txt=txt.."IO2="..gpio.read(4).."<br />"
				end
        elseif(txt == "getip")then
				txt = ""
				txt=txt.."IP:"..wifi.sta.getip().."<br />"
		elseif(txt == "setwifi")then
				txt = ""
				if(_GET.ap and _GET.pwd) then
				setWifi(_GET.ap,_GET.pwd)
				txt="Wifi config updated!<br />New ap:".._GET.ap.." New pwd:".._GET.pwd.." Please reboot!" 
				else
				txt="No ap and pwd pram!"
				end
		elseif(txt == "getwifi")then
				txt = ""
				if(getConf().ap and getConf().pwd) then
				txt="Wifi config:<br />ap:"..getConf().ap.." pwd:"..getConf().pwd 
				else
				txt="No config!"
				end
		elseif(txt == "gettemp")then --Temperature
				txt = ""
				getTemp()
				if(getConf().ap and getConf().pwd) then
				txt="Temperature:"..Temperature.."."..TemperatureDec.."<br />"..("Humidity: "..Humidity.."."..HumidityDec)
				else
				txt="No data!"
				end
		end
		client:send(buf..getString(txt));
        client:close();
        collectgarbage();
    end)
end)
connect=nil
tmr.alarm(0, 20000,1, function()
	if(wifi.sta.getip()) then
	connect=true
	else
	connect=false
	--print("Rebooting...!")
	--node.restart()
	wifi.sta.disconnect()
	wifi.sta.connect()
	tmr.delay(3000)
	if(wifi.sta.getip()~=nil) then connect=true end
	end
	end)

tmr.alarm(1, 1000,1, function()
	tmr.wdclr() --清除开门狗计数

	end)
	
	
--以下是温度函数
pin = 4
Humidity = 0
HumidityDec=0
Temperature = 0
TemperatureDec=0
Checksum = 0
ChecksumTest=0


function getTemp()
Humidity = 0
HumidityDec=0
Temperature = 0
TemperatureDec=0
Checksum = 0
ChecksumTest=0

--Data stream acquisition timing is critical. There's
--barely enough speed to work with to make this happen.
--Pre-allocate vars used in loop.

bitStream = {}
for j = 1, 40, 1 do
     bitStream[j]=0
end
bitlength=0

gpio.mode(pin, gpio.OUTPUT)
gpio.write(pin, gpio.LOW)
tmr.delay(20000)   --default 20000
--Use Markus Gritsch trick to speed up read/write on GPIO
gpio_read=gpio.read
gpio_write=gpio.write

gpio.mode(pin, gpio.INPUT)

--bus will always let up eventually, don't bother with timeout
while (gpio_read(pin)==0 ) do end

c=0
while (gpio_read(pin)==1 and c<100) do c=c+1 end

--bus will always let up eventually, don't bother with timeout
while (gpio_read(pin)==0 ) do end

c=0
while (gpio_read(pin)==1 and c<100) do c=c+1 end

--acquisition loop
for j = 1, 40, 1 do
     while (gpio_read(pin)==1 and bitlength<10 ) do
          bitlength=bitlength+1
     end
     bitStream[j]=bitlength
     bitlength=0
     --bus will always let up eventually, don't bother with timeout
     while (gpio_read(pin)==0) do end
end

--DHT data acquired, process.

for i = 1, 8, 1 do
     if (bitStream[i+0] > 2) then
          Humidity = Humidity+2^(8-i)
     end
end
for i = 1, 8, 1 do
     if (bitStream[i+8] > 2) then
          HumidityDec = HumidityDec+2^(8-i)
     end
end
for i = 1, 8, 1 do
     if (bitStream[i+16] > 2) then
          Temperature = Temperature+2^(8-i)
     end
end
for i = 1, 8, 1 do
     if (bitStream[i+24] > 2) then
          TemperatureDec = TemperatureDec+2^(8-i)
     end
end
for i = 1, 8, 1 do
     if (bitStream[i+32] > 2) then
          Checksum = Checksum+2^(8-i)
     end
end
ChecksumTest=(Humidity+HumidityDec+Temperature+TemperatureDec) % 0xFF

print ("Temperature: "..Temperature.."."..TemperatureDec)
print ("Humidity: "..Humidity.."."..HumidityDec)
print ("ChecksumReceived: "..Checksum)
print ("ChecksumTest: "..ChecksumTest)
end
