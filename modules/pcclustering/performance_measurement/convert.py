import re
import os
import sys
import csv
import functools

file = sys.argv[1]
print(file)

f = open(file)

destFile = file[:-5] + ".txt"
print(destFile)

content = f.readlines()

# Remove all ProcessorNetworkEvaluatior lines

print(len(content))
content = [k for k in content if 'ProcessorNetworkEvaluator' not in k]
# content = filter(lambda k: 'ProcessorNetworkEvaluator' in k, content)
# print(content)
print(len(content))

offset = -1;
for i, j in enumerate(content):
    if j.find('###') != -1:
        offset = i
        break

print(offset)

print(content[offset])
nTestsLine = content[offset]
m = re.search('###(.+?)###', nTestsLine)
nTests = int(m.group(1))
print(nTests)
offset = offset + 2

rendererPlain = []
for i in range(0, nTests):
    m = re.search('PCPRenderer:&nbsp;(.+?)&nbsp;ms', content[offset+i])
    rendererPlain.append(m.group(1))
offset = offset + nTests + 2

rendererTransparency = []
for i in range(0, nTests):
    m = re.search('PCPRenderer:&nbsp;(.+?)&nbsp;ms', content[offset+i])
    rendererTransparency.append(m.group(1))
offset = offset + nTests + 1

densityMapGenerator = []
countElements = []
clusterDetection = []
filterData = []
# print(content[offset])
for i in range(0, nTests):
    inc = 7
    pos = offset + inc * i

    m = re.search('DensityMapGenerator:&nbsp;(.+?)&nbsp;ms', content[pos + 0])
    densityMapGenerator.append(m.group(1))

    m = re.search('CountElements:&nbsp;(.+?)&nbsp;ms', content[pos + 2])
    countElements.append(m.group(1))

    m = re.search('ClusterDetection:&nbsp;(.+?)&nbsp;ms', content[pos + 4])
    clusterDetection.append(m.group(1))

    m = re.search('FilterData:&nbsp;(.+?)&nbsp;ms', content[pos + 5])
    filterData.append(m.group(1))


# print(rendererPlain)
# print(rendererTransparency)
# print(densityMapGenerator)
# print(countElements)
# print(clusterDetection)
# print(filterData)

rp = functools.reduce(lambda x,y: float(x)+float(y), rendererPlain) / len(rendererPlain)
rt = functools.reduce(lambda x,y: float(x)+float(y), rendererTransparency) / len(rendererTransparency)
dg = functools.reduce(lambda x,y: float(x)+float(y), densityMapGenerator) / len(densityMapGenerator)
ce = functools.reduce(lambda x,y: float(x)+float(y), countElements) / len(countElements)
cd = functools.reduce(lambda x,y: float(x)+float(y), clusterDetection) / len(clusterDetection)
fd = functools.reduce(lambda x,y: float(x)+float(y), filterData) / len(filterData)

print(rp)
print(rt)
print(dg)
print(ce)
print(cd)
print(fd)

with open(destFile, "w") as f:
    f.write(str(rp) + '\n')
    f.write(str(rt) + '\n')
    f.write(str(dg) + '\n')
    f.write(str(ce) + '\n')
    f.write(str(cd) + '\n')
    f.write(str(fd) + '\n')

