srv_udp=net.createServer(net.UDP)
srv_udp:on("receive", function(conn, payload) conn:send(wifi.sta.getip()) end)
srv_udp:listen(5007)