-- load helper library
local i2lua = require("runtime.lua")

-- this function returns true if we want to use this RI in tunnel building at hop
function filterRI(ri, hopNumber, inbound)
   if hopNumber == 0 then
      -- for first hop we want to only use floodfill routers that are reachable
      return ri.reachable and ri.floodfill
   else
      -- for all other hops we only care about reachability
      return ri.reachable
   end
end

-- our main function
function main()
   local selector = i2lua.PeerSelector(filterRI)
   -- create a new destination
   local dest = i2lua.Destination("privatekeys.dat")
   dest:run()
end

-- initialize i2lua
i2lua.Init("i2lua.conf")
-- run mainloop
i2lua.RunI2P(main)
