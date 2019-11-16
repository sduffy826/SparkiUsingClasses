
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


