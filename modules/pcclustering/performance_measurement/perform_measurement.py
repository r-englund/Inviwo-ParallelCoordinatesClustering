import inviwo
import inviwoqt

nTests = 100

print("###" + str(nTests) + "###")

print("##PCPRenderer (plain)##")
for i in range(0, nTests):
    inviwo.clickButton("PCP Renderer.invalidate")
    inviwo.wait()
    inviwoqt.update()

inviwo.setPropertyValue("PCP Renderer._depthTesting", True)
inviwo.wait()

print("##PCPRenderer (transparency)##")
for i in range(0, nTests):
    inviwo.clickButton("PCP Renderer.invalidate")
    inviwo.wait()
    inviwoqt.update()

print("##DensityMapGenerator##")
for i in range(0, nTests):
    inviwo.clickButton("Density Map Generator._invalidate")
    inviwo.wait()
    inviwoqt.update()
