
function printTable(t)
	 for k, v in pairs(t) do
			print(k, v)
	 end
end

function printHops(hops)
	 for k, v in pairs(hops) do
			print(k, v.ident)
	 end
end

-- hops we tried that are "dead"
deadHops = {}

-- this function selects hops for a tunnel build
function selectHops(push, numhops, inbound)
	 print ("select hops")
	 -- function for visiting router infos in netdb for selection
	 function visit(ri)
			return ri.floodfill and ri.reachable
	 end
	 -- function that pushes ri onto build result
	 function accept(ri)
			print(ri.ident)
			push(ri.ident)
	 end
	 i2p.VisitRandomRIWithFilter(numhops, visit, accept)
	 return true
end

function tunnelBuildSuccess(hops, isInbound)
	 print("tunnel build succeeded with hops")
	 printHops(hops)
end

function tunnelBuildFail(hops, isInbound)
	 print("tunnel build failed")
	 printHops(hops)
end


-- callback for for when destination is created
function destinationCreated(dest)
	 i2p.DestinationSetPeerSelector(dest, selectHops, tunnelBuildSuccess, tunnelBuildFail)
	 print(i2p.GetDestinationAddress(dest))
end

i2p.Init("i2lua.conf")

-- start i2p router
i2p.Start()
-- create our test destination
i2p.NewDestination("test-privkey.dat", destinationCreated)
-- wait for router to stop
i2p.Wait()
