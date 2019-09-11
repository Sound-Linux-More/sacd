ARCH = $(shell getconf LONG_BIT)

CXX = g++

CXXFLAGS_32 = -msse2
CXXFLAGS_64 =
CXXFLAGS = $(CXXFLAGS_$(ARCH)) -std=c++11 -Wall -O3
#CXXFLAGS += -g -ggdb3

VPATH = libdstdec:libdsd2pcm:libsacd

INCLUDE_DIRS = libdstdec libdsd2pcm libsacd
CPPFLAGS = $(foreach includedir,$(INCLUDE_DIRS),-I$(includedir))

LIBRARIES = rt pthread
LIBRARY_DIRS = libdstdec libdsd2pcm libsacd
LDFLAGS = $(foreach librarydir,$(LIBRARY_DIRS),-L$(librarydir))
LDFLAGS += $(foreach library,$(LIBRARIES),-l$(library))

.PHONY: all clean install

all: clean str_data ac_data coded_table frame_reader dst_decoder dst_decoder_mt \
     upsampler dsd_pcm_converter_hq \
     dsd_pcm_converter_engine \
     scarletbook sacd_disc sacd_media sacd_dsdiff sacd_dsf \
     main \
     sacd

str_data: dst_defs.h str_data.h str_data.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c libdstdec/str_data.cpp -o libdstdec/str_data.o

ac_data: dst_defs.h ac_data.h ac_data.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c libdstdec/ac_data.cpp -o libdstdec/ac_data.o

coded_table: dst_defs.h coded_table.h coded_table.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c libdstdec/coded_table.cpp -o libdstdec/coded_table.o

frame_reader: str_data.h coded_table.h frame_reader.h frame_reader.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c libdstdec/frame_reader.cpp -o libdstdec/frame_reader.o

dst_decoder: str_data.h ac_data.h coded_table.h frame_reader.h dst_decoder.h dst_decoder.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c libdstdec/dst_decoder.cpp -o libdstdec/dst_decoder.o

dst_decoder_mt: dst_decoder.h dst_decoder_mt.h dst_decoder_mt.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c libdstdec/dst_decoder_mt.cpp -o libdstdec/dst_decoder_mt.o

dsd_pcm_converter_engine: dsd_pcm_converter_multistage.h dsd_pcm_converter_engine.h dsd_pcm_converter_engine.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c libdsd2pcm/dsd_pcm_converter_engine.cpp -o libdsd2pcm/dsd_pcm_converter_engine.o

upsampler: dither.h upsampler.h upsampler.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c libdsd2pcm/upsampler.cpp -o libdsd2pcm/upsampler.o

dsd_pcm_converter_hq: upsampler.h dsd_pcm_converter_hq.h dsd_pcm_converter_hq.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c libdsd2pcm/dsd_pcm_converter_hq.cpp -o libdsd2pcm/dsd_pcm_converter_hq.o

scarletbook: scarletbook.h scarletbook.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c libsacd/scarletbook.cpp -o libsacd/scarletbook.o

sacd_disc: endianess.h scarletbook.h sacd_reader.h sacd_disc.h sacd_disc.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c libsacd/sacd_disc.cpp -o libsacd/sacd_disc.o

sacd_media: scarletbook.h scarletbook.h sacd_media.h sacd_media.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c libsacd/sacd_media.cpp -o libsacd/sacd_media.o

sacd_dsdiff: scarletbook.h sacd_dsd.h sacd_reader.h endianess.h sacd_dsdiff.h sacd_dsdiff.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c libsacd/sacd_dsdiff.cpp -o libsacd/sacd_dsdiff.o

sacd_dsf: scarletbook.h sacd_dsd.h sacd_reader.h endianess.h sacd_dsf.h sacd_dsf.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c libsacd/sacd_dsf.cpp -o libsacd/sacd_dsf.o

main: version.h sacd_reader.h sacd_disc.h sacd_dsdiff.h sacd_dsf.h dsd_pcm_converter_hq.h dsd_pcm_converter_engine.h main.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c main.cpp -o main.o

sacd: frame_reader.o ac_data.o str_data.o coded_table.o dst_decoder.o dst_decoder_mt.o dsd_pcm_converter_hq.o dsd_pcm_converter_engine.o sacd_media.o sacd_dsf.o sacd_dsdiff.o sacd_disc.o main.o
	$(CXX) $(CXXFLAGS) -o sacd libdsd2pcm/upsampler.o libdsd2pcm/dsd_pcm_converter_hq.o libdsd2pcm/dsd_pcm_converter_engine.o libdstdec/frame_reader.o libdstdec/ac_data.o libdstdec/str_data.o libdstdec/coded_table.o libdstdec/dst_decoder.o libdstdec/dst_decoder_mt.o libsacd/sacd_media.o libsacd/sacd_dsf.o libsacd/sacd_dsdiff.o libsacd/scarletbook.o libsacd/sacd_disc.o main.o $(LDFLAGS)

clean:
	rm -f sacd *.o $(foreach librarydir,$(LIBRARY_DIRS),$(librarydir)/*.o)

install: sacd

	install -d $(DESTDIR)/usr/bin
	install -m 0755 ./sacd $(DESTDIR)/usr/bin
	install -d $(DESTDIR)/usr/share/man/man1
	install -m 0644 ./man/sacd.1 $(DESTDIR)/usr/share/man/man1

