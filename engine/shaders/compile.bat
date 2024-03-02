glslc.exe shader.vert -o generated/base_vert.spv
glslc.exe shader.frag -o generated/base_frag.spv

glslc.exe skybox.vert -o generated/skybox_vert.spv
glslc.exe skybox.frag -o generated/skybox_frag.spv

glslc.exe basic_P2T2.vert -o generated/brdf_lut_gen_vert.spv
glslc.exe brdf_lut_gen.frag -o generated/brdf_lut_gen_frag.spv

pause