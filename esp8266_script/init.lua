wifi.setmode(wifi.STATION)
wifi.sta.config("Depto 701", "pacman100")
ip = wifi.sta.getip()
--print(ip)

dofile("tcpserver.lua")