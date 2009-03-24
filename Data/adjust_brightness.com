gfx define field tex sample_texture coordinates xi texture LA1;
gfx define field rescaled_tex rescale_intensity_filter field tex output_min 0 output_max 1;
gfx cre spectrum monochrome clear;
gfx modify spectrum monochrome linear range 0 1 extend_above extend_below monochrome colour_range 0 1 ambient diffuse component 1;
#create a texture using the rescaled sample_texture field 
gfx create texture tract linear;
gfx modify texture tract width 1 height 1 distortion 0 0 0 colour 0 0 0 alpha 0 decal linear_filter resize_nearest_filter clamp_wrap specify_number_of_bytes 2 evaluate field rescaled_tex element_group LA1 spectrum monochrome texture_coordinate xi fail_material transparent_gray50;

# create a material containing the texture so that we can select along
# with appropriate texture coordinates to visualise the 3D image
gfx create material tract texture tract

gfx modify g_element LA1 surfaces material tract texture_coordinates xi;



