# graph = {'a':{'b':10,'c':3},'b':{'c':1,'d':2},'c':{'b':4,'d':8,'e':2},'d':{'e':7},'e':{'d':9}}
 
# Class for the dijkstra's algorithm, this one expects a graph that's in the format
#   graphOfNodes = { nodeId : { childNode1 : distance1, childNode2 : distance2, ... }, nodeId2 : {....}.. } 
# here I'm using distance but you can think of it as a weight also (weight of that node)
class dijkstraGraphPoints:
  # Constructor, pass in the graph and the starting node, later call the methods
  # to get the path to a goal or a distance from the start to a particular node
  def __init__(self, graphOfNodes, startNode):
    self.graphOfNodes     = graphOfNodes.copy()
    self.startNode        = startNode
    
    self.shortestDistance = {}
    self.preceedingNode   = {}
    self.unvisitedNodes   = self.graphOfNodes.copy()
    self.infinity         = 9999999
    
    # Make the distance to every node infinity
    for node in self.unvisitedNodes:
      self.shortestDistance[node] = self.infinity
    # Distance to start node is zero
    self.shortestDistance[self.startNode] = 0

    # Calculate the shortest distance to each node
    while self.unvisitedNodes:
      minNode = None
      # Get the node with the shortest distance, that's the one we'll inspect 
      for node in self.unvisitedNodes:
        if minNode is None:
          minNode = node
        elif self.shortestDistance[node] < self.shortestDistance[minNode]:
          minNode = node

      # Loop thru all the nodes connected to the minimum node, we'll calculate the
      # distance to get to that child thru minMode, if that distance is less than the
      # shortest distance to the child then we'll update it's shortest distance and
      # identify minNode as it's preceedingNode
      for childNode, childDistance in self.graphOfNodes[minNode].items():
        if childDistance + self.shortestDistance[minNode] < self.shortestDistance[childNode]:
          self.shortestDistance[childNode] = childDistance + self.shortestDistance[minNode]
          self.preceedingNode[childNode]   = minNode
      
      # Remove minNode from the list of unvisited nodes
      self.unvisitedNodes.pop(minNode)

  # ------------------------------------------------------------------------------------------------
  # Return the distance from the startNode to this node
  def getDistanceToNode(self, node2GetTo):
    try:
      return self.shortestDistance[node2GetTo]
    except:
      return self.infinity

   # ------------------------------------------------------------------------------------------------
   # Return an array that is the shortest path to the goal node passed in
  def getShortestPathToGoal(self, node2GetTo):
    path2Return = []
    currentNode = node2GetTo
    foundPath   = True
    while currentNode != self.startNode:
      try:
        path2Return.insert(0,currentNode)               # Push the current node onto list
        currentNode = self.preceedingNode[currentNode]  # Set it to the preceeding node
      except KeyError:
        foundPath = False
        break
    
    if ((foundPath == True) and (self.shortestDistance[node2GetTo] != self.infinity)):
      path2Return.insert(0,self.startNode)
    else:
      path2Return.clear()
    return path2Return
