import sharedVars as gv

def fooBarf():
  cat = 0
  for i in range(5):
    cat += i 
  return cat

print("foo")

idx = 0
while (idx < 10):
  print(idx)
  print(fooBarf())
  idx += 1


gv.worldXMin = min(gv.worldXMin, 4.2, 3.90)
print("Min is: ")
print(gv.worldXMin)

chicken = { "abc" : "letters",
            "123" : "numbers",
            "boo" : 4 }
print(len(chicken))
print(chicken)

