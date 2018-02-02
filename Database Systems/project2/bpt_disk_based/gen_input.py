from random import shuffle

f = open('input.txt', 'w')

for i in range(50):
    f.write('i ' + str(i+1) + ' a' + str(i+1) + '\n')

for i in range(50):
    f.write('f ' + str(i+1) + '\n')

f.write('l ' + '\n')

arr = list(range(25))
shuffle(arr)

for i in arr:
    f.write('d ' + str(i+1) + '\n')

f.write('l ' + '\n')

for i in arr:
    f.write('f ' + str(i+1) + '\n')

f.write('l ' + '\n')


for i in arr:
    f.write('i ' + str(i+1) + ' a' + str(i+1) + '\n')

for i in range(50):
    f.write('f ' + str(i+1) + '\n')

f.write('l ' + '\n')
f.close()
