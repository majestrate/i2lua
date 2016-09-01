-- load helper library (assumes you are in the root directory of the repo)
i2lua = assert(loadfile("runtime.lua"))()

-- this function returns true if we want to use this RI in tunnel building at hop
function filterRI(ri, hopNumber, inbound)
   if hopNumber == 1 then
      -- for first hop we want to only use floodfill routers that are reachable
      return ri.reachable and ri.floodfill
   else
      print("check hop", hopNumber)
      -- for all other hops we want whatever
      return true
   end
end

-- our main function
local function main()
   print("create selector")
   local selector = i2lua.PeerSelector(filterRI)
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
