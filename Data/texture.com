gfx read node $example/ImagePlane.exnode
gfx read elem $example/ImagePlane.exelem

#gfx modify g_element neutralImage21 lines select_on invisible material default selected_material default_selected;

#gfx create texture "texture" image "$example/68708398" width 1.0 height 1.0 decal
gfx create texture "texture" image "dcm:$example/68708398" width 1 height 1 depth 1 distortion 0 0 0 colour 0 0 0 alpha 0 decal linear_filter resize_nearest_filter clamp_wrap;


gfx create material "texture"  texture "texture"
gfx modify g_element ImagePlane surface material "texture" texture_coordinates xi
gfx mod g_elem ImagePlane line mat default

