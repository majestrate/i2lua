-- i2lua runtime
local I2Lua = {}

function I2Lua.PrintTable(t)
   for k, v in pairs(t) do
      print(k, v)
   end
end

I2Lua.PeerSelector = {}

-- create a new peer selector with function f that filters the ri
function I2Lua.PeerSelector.new(f, success, fail)
   return setmetatable({
         _success = success,
         _fail = fail,
         _select = function(push, hops, inbound)

            local hop = 1
            -- set is inbound
            -- do selection
            local filter = function(ri)
               local r = f(ri, hop, inbound)
               if r == true then
                  hop = hop + 1
               end
               return r
            end
            local pushhop = function(ri)
               push(ri.ident)
            end
            local selected = i2p.VisitRandomRIWithFilter(hops, filter, pushhop)
            print("Selected", selected)
            -- clear push function
            return selected == hops
         end,

   }, PeerSelector)
end

setmetatable(I2Lua.PeerSelector, {__call = function(_, ...) return I2Lua.PeerSelector.new(...) end})

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
                  print("Select peers", self._selector, push, hops, inbound)
                  I2Lua.PrintTable(self._selector)
                  return self._selector._select(push, hops, inbound)
               end
               local _buildSuccess = function(hops, inbound)
                  self._selector._success(hops, inbound)
               end
               local _buildFail = function(hops, inbound)
                  self._selector._fail(hops, inbound)
               end
               i2p.DestinationSetPeerSelector(self._dest, _selectPeers, _buildSuccess, _buildFail)
               print("using peer selector", _selectPeers, self._selector)
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
