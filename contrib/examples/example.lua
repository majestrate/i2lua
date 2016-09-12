-- load helper library (assumes you are in the root directory of the repo)
i2lua = assert(loadfile("runtime.lua"))()

-- this function returns true if we want to use this RI in tunnel building at hop
function filterRI(ri, hopNumber, inbound)
   if hopNumber == 1 then
      -- for first hop we want to only use floodfill routers that are reachable
      return ri.reachable and ri.floodfill
   else
      return ri.reachable
   end
end

-- this function is called when a successfull tunnel is built using hops
function onBuildSuccess(hops, inbound)
   print("tunnel build success")
   for idx, ri in pairs(hops) do
      print("Hop", idx, "is", ri.ident)
   end
end

-- this function is called when a tunnel is failed to be built using hops
function onBuildFail(hops, inbound)
   print("tunnel build failed")
end

-- our main function
local function main()
   print("create selector")
   local selector = i2lua.PeerSelector(filterRI, onBuildSuccess, onBuildFail)
   print("create destination")
   -- create a new destination
   local dest = i2lua.Destination.new("privatekeys.dat")
   dest:setPeerSelector(selector)
   dest:run(function(d)
         print("we are", dest:base32())
   end)
   print("okay")
end

-- initialize i2lua
i2lua.Init("i2lua.conf")
-- run mainloop
print("run i2p")
i2lua.RunI2P(main)
print("exited")
