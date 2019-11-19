import dijkstrasClass

graph = {'a':{'b':10,'c':3},'b':{'c':1,'d':2},'c':{'b':4,'d':8,'e':2},'d':{'e':7},'e':{'d':9}}

# Get reference to object, we'll use graph above and 'a' as the starting point
dijkstrasObj = dijkstrasClass.dijkstraGraphPoints(graph, 'c')

for x, y in graph.items():
  print("Len to {0} is: {1}".format(x,dijkstrasObj.getDistanceToNode(x)))
  print("Path is {0}".format(str(dijkstrasObj.getShortestPathToGoal(x))))
  print("")

print("Len to z is: {0}".format(dijkstrasObj.getDistanceToNode('z')))
print("Path is {0}".format(str(dijkstrasObj.getShortestPathToGoal('z'))))