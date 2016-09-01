-- i2lua runtime
local I2Lua = {}

function I2Lua.PrintTable(t)
   for k, v in pairs(t) do
      print(k, v)
   end
end

I2Lua.PeerSelector = {}

-- create a new peer selector with function f that filters the ri
function I2Lua.PeerSelector.new(f)
   return setmetatable({
         _filter = f,
         _push = nil,
         _hop = 0,
         _inbound = false,
         _filterRI = 0,
         _selectPeers = 0,
         _nop = 0,
   }, PeerSelector)
end

setmetatable(I2Lua.PeerSelector, {__call = function(_, ...) return I2Lua.PeerSelector.new(...) end})

-- function for filtering RI
function I2Lua.PeerSelector:_filterRI(self, ri)
   local hop = self._hop
   local result = self:_filter(ri, _hop, self._inbound)
   if result == true then
      -- we accepted this hop, increment our hop counter
      self._hop = hop + 1
      print("selected hop", self._hop)
   end
   return result
end

-- select peers 
function I2Lua.PeerSelector:_selectPeers(self, push, hops, inbound)
   print("select peers")
   -- set push function
   self._push = push
   -- clear last hop number
   self._hop = 0
   -- set is inbound
   self._inbound = inbound
   -- do selection
   local filter = function(ri)
      I2Lua.PrintTable(ri)
      return self:_filterRI(ri)
   end
   local pushhop = function(ri)
      self:_push(ri.ident)
   end
   local selected = i2p.VisitRandomRIWithFilter(filter, pushhop, hops)
   -- clear push function
   self._push = nil
   return selected == hops
end

I2Lua.Destination = {}

-- create a new destination
function I2Lua.Destination.new(keyfile)
   return setmetatable({
         _keyfile = keyfile,
         _dest = 0,
         _selector = 0,
         setPeerSelector = function (self, selector)
            self._selector = selector
         end,
         _created = function(self, dest)
            self._dest = dest
            print("destination created", self._dest)
            if self._selector ~= 0 then
               local _selectPeers = function(push, hops, inbound)
                  print(push, hops, inbound)
                  I2Lua.PrintTable(self._selector)
                  return self._selector:_selectPeers(push, hops, inbound)
               end
               i2p.DestinationSetPeerSelector(self._dest, _selectPeers)
               print("using peer selector", _selectPeers)
            end
         end,
         base32 = function(self)
            return i2p.GetDestinationAddress(self._dest)
         end,
         run = function(self, running)
            print("run destination")
            I2Lua.PrintTable(self)
            local callback = function(dest)
               self:_created(dest)
            end
            i2p.NewDestination(self._keyfile, callback)
            local dest = self._dest
            print ("running", dest)
            if running ~= nil then
               running(dest)
            end
            i2p.RunDestination(dest)
         end
                       }, I2Lua.Destination)
end

function I2Lua.Init(configfile)
   print('load config file', configfile)
   i2p.Init(configfile)
end

-- run i2p router with main function
function I2Lua.RunI2P(f)
   i2p.Start()
   if f ~= nil then
      i2p.Sleep(1000)
      f()
   end
   i2p.Wait()
end

return I2Lua
