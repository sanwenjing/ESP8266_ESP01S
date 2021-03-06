function getString(body)
if(body) then
return "<html><head><title>ESP-01s</title></head><body>"..body.."</body></html>"
else
return "<html><head><title>ESP-01s</title></head><body>Bad actions! </body></html>"
end
end

function setWifi(ap,pwd)
file.open("wifi.lua", "w+")    -- д��wifi��?
file.writeline("ap="..ap.."\r\npwd="..pwd)
file.close()
end


function getConf()--��WIFI����
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
	tmr.wdclr()
	if(connect) then
		if(gpio.read(led2)==0) then  gpio.write(led2, gpio.HIGH) else gpio.write(led2, gpio.LOW) end
	else
		gpio.write(led2, gpio.HIGH)
	end
	end)
