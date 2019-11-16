emptyList = []


def addItemToList(theList, theItem):
  theList.append(theItem)


for i in range(5):
  theString = "Value is {0}".format(i)
  addItemToList(emptyList, theString)

print("Done, size of emptyList is {0}".format(len(emptyList)))
  