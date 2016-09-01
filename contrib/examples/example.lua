-- load helper library (assumes you are in the root directory of the repo)
i2lua = assert(loadfile("runtime.lua"))()

-- this function returns true if we want to use this RI in tunnel building at hop
function filterRI(ri, hopNumber, inbound)
   i2lua.PrintTable(ri)
   if hopNumber == 0 then
      -- for first hop we want to only use floodfill routers that are reachable
      return ri.reachable and ri.floodfill
   else
      -- for all other hops we only care about reachability
      return ri.reachable
   end
end

-- our main function
local function main()
   print("create selector")
   local selector = i2lua.PeerSelector(filterRI)
   print("create destination")
   -- create a new destination
   local dest = i2lua.Destination.new("privatekeys.dat")
   local onReady = function(d)
      d:setPeerSelector(selector)
   end
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
