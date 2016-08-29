-- example i2lua runtime



function filter(ri)
   -- filter
   return ri.floodfill
end

function visitPrint(ri)
   print("RI")
   for k, v in pairs(ri) do
      print(k, v)
   end
   print("")
end


print('init...')
i2p.Init('i2lua.conf')
i2p.Start()
print('started')
local f = i2p.VisitRIWithFilter
f(filter, visitPrint)
print("done")
