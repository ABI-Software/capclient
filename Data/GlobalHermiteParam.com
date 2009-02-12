#a/testing/cube: Graphical element groups: viewing a cube

#Read nodes and elements

#gfx read nodes example GlobalHermiteParam.exnode

gfx read nodes example test_1.model.exnode time 0

gfx read elements example GlobalHermiteParam.exelem

#Create material

#gfx create material orange ambient 1 0.25 0 diffuse 1 0.25 0
#gfx modify g_element cube node_points label cmiss_number glyph cross size 0.4 material orange

# Now add surfaces to the distorted cube, and view it in the graphics window.
#gfx modify g_element cube surfaces mat orange
#


#gfx create window
#gfx mod win 1 view perspective


for ($i = 2 ; $i < 28 ; $i++)
  {
	 gfx read nodes example test_$i.model.exnode time ($i-1)/30;
  }

