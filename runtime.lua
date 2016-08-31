-- example i2lua runtime


function filterFloodfill(ri)
   -- get reachable floodfill
   return ri.floodfill and ri.reachable
end

function printRI(ri)
   for k, v in pairs(ri) do
      print(k, v)
   end
end

function selectPeers(push, hops, inbound)
   local function visit(ri)
      push(ri.ident)
   end
   local selected = i2p.VisitRandomRIWithFilter(filterFloodfill, visit, hops)
   return selected == hops
end

function onStarted()
   print("i2p router started")
   function created(dest)
      print("created destination")
      -- our custom peer selector function
      i2p.DestinationSetPeerSelector(dest, selectPeers)
      print("Peer Selector", selectPeers)
      print("Running destination...")
      i2p.RunDestination(dest)
      print("destination run is done")
   end
   i2p.NewDestination("keys.data", created)
end

print('init...')
i2p.Init('i2lua.conf')
i2p.Start(onStarted)
i2p.Wait()
print("done")
