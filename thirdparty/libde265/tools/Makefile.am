
bin_PROGRAMS = gen-enc-table yuv-distortion rd-curves block-rate-estim tests bjoentegaard

AM_CPPFLAGS = -I../libde265

gen_enc_table_DEPENDENCIES = ../libde265/libde265.la
gen_enc_table_CXXFLAGS =
gen_enc_table_LDFLAGS =
gen_enc_table_LDADD = ../libde265/libde265.la -lstdc++
gen_enc_table_SOURCES = gen-entropy-table.cc

yuv_distortion_DEPENDENCIES = ../libde265/libde265.la
yuv_distortion_CXXFLAGS =
yuv_distortion_LDFLAGS =
yuv_distortion_LDADD = ../libde265/libde265.la -lstdc++
yuv_distortion_SOURCES = yuv-distortion.cc

if HAVE_VIDEOGFX
  yuv_distortion_CXXFLAGS += $(VIDEOGFX_CFLAGS)
  yuv_distortion_LDFLAGS += $(VIDEOGFX_LIBS)
endif

rd_curves_DEPENDENCIES = ../libde265/libde265.la
rd_curves_CXXFLAGS =
rd_curves_LDFLAGS =
rd_curves_LDADD = ../libde265/libde265.la -lstdc++
rd_curves_SOURCES = rd-curves.cc

block_rate_estim_DEPENDENCIES = ../libde265/libde265.la
block_rate_estim_CXXFLAGS =
block_rate_estim_LDFLAGS =
block_rate_estim_LDADD = ../libde265/libde265.la -lstdc++
block_rate_estim_SOURCES = block-rate-estim.cc

tests_DEPENDENCIES = ../libde265/libde265.la
tests_CXXFLAGS =
tests_LDFLAGS =
tests_LDADD = ../libde265/libde265.la -lstdc++
tests_SOURCES = tests.cc

bjoentegaard_DEPENDENCIES = ../libde265/libde265.la
bjoentegaard_CXXFLAGS =
bjoentegaard_LDFLAGS =
bjoentegaard_LDADD = ../libde265/libde265.la -lstdc++
bjoentegaard_SOURCES = bjoentegaard.cc
