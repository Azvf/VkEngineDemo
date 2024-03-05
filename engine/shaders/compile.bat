glslc.exe shader.vert -o generated/base_vert.spv
glslc.exe shader.frag -o generated/base_frag.spv

glslc.exe skybox.vert -o generated/skybox_vert.spv
glslc.exe skybox.frag -o generated/skybox_frag.spv

glslc.exe basic_P2T2.vert -o generated/brdf_lut_gen_vert.spv
glslc.exe brdf_lut_gen.frag -o generated/brdf_lut_gen_frag.spv

glslc.exe cubemap.vert -o generated/cubemap_prefilter_gen_vert.spv
glslc.exe cubemap_prefilter_gen.frag -o generated/cubemap_prefilter_gen_frag.spv

glslc.exe cubemap.vert -o generated/irradiance_convolution_vert.spv
glslc.exe irradiance_convolution.frag -o generated/irradiance_convolution_frag.spv

pause