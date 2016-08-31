-- i2lua runtime

I2Lua = {}

function I2Lua.printRI(ri)
   for k, v in pairs(ri) do
      print(k, v)
   end
end

I2Lua.PeerSelector = {}

-- create a new peer selector with function f that filters the ri
function I2Lua.PeerSelector.new(f)
   return setmetatable({filter = f, _push = nil, _hop = 0, _inbound = false}, PeerSelector)
end

setmetatable(I2Lua.PeerSelector, {__call = function(_, ...) return I2Lua.PeerSelector.new(...) end})

-- function for filtering RI
function I2Lua.PeerSelector:_filterRI(ri)
   if self:filter != nil then
      local result = self:filter(ri, self:_hop, self:_inbound)
      if result == true then
         -- we accepted this hop, increment our hop counter
         self:_hop = self:_hop + 1
         -- push hop
         self:_push(ri.ident)
      end
      return result
   else
      -- if no filter function deny all
      return false
   end
end

-- do nothing and return false
function I2Lua.PeerSelector:_nop(ri)
   return false
end

-- select peers 
function I2Lua.PeerSelector:_selectPeers(push, hops, inbound)
   -- set push function
   self:_push = push
   -- clear last hop number
   self:_hop = 0
   -- set is inbound
   self:_inbound = inbound
   -- do selection
   local selected = i2p.VisitRandomRIWithFilter(self:_filterRI, self:_nop, hops)
   -- clear push function
   self:_push = nil
   return selected == hops
end

I2Lua.Destination = {}

-- create a new destination
function Destination.new(keyfile)
   return setmetatable({_keyfile = keyfile, _dest = nil, _selector = nil}, I2Lua.Destination)
end

setmetatable(I2Lua.Destination, {__call = function(_, ...) return I2Lua.Destination.new(...) end})

-- set custom peer selector
function I2Lua.Destination:setPeerSelector(selector)
   self:_selector = selector
end

-- callback for setting peer selector and running
function I2Lua.Destination:_onCreated(dest)
   self:_dest = dest
   if self:_selector != nil then
      i2p.DestinationSetPeerSelector(self:_dest, self:_selector:_selectPeers)
   end
   i2p.RunDestination(self:_dest)
end

-- start up i2p destination, blocks until dead
-- TODO: use another thread
function I2Lua.Destination:run()
   i2p.NewDestination(self:_keyfile, self:_onCreated)
end

function I2Lua.Init(configfile)
   i2p.Init(configfile)
end

-- run i2p router with main function
function I2Lua.RunI2P(f)
   i2p.Start()
   if f != nil then 
      f()
   end
   i2p.Wait()
end

return I2Lua
