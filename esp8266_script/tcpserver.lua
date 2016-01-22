SYNC = 2
SET_ALARM = 4

tm = loadfile("ntp.lua")()

mspSyncTime = function(T)
	cmd = string.char(255, 255, SYNC, 255 - T.hour, 255 - T.minute, 255 - T.second)
	print(cmd)
end

mspSetAlarm = function(tab)
	cmd = string.char(255, 255, SET_ALARM, 255 - tab["H"], 255 - tab["m"], 255 - tab["TONE"])
	print(cmd)
end

srv=net.createServer(net.TCP)
srv:listen(1313, function(conn)
	conn:on("receive", function(conn,payload)
		jsontab = cjson.decode(payload)
		if jsontab["INTR"] == SYNC then
			tm:sync(mspSyncTime)
		elseif jsontab["INTR"] == SET_ALARM then
			mspSetAlarm(jsontab["VALUES"])
		end
		conn:send('{"STATUS": "OK"}')
		end)
	conn:on("sent", function(conn) conn:close() end)
end)