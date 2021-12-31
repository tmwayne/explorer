TESTS = test_deque test_frame test_data_mmap

check_PROGRAMS = test_deque test_frame test_data_mmap

test_deque_SOURCES = test-deque.c
test_deque_LDADD = ../src/common/libcommon.la

test_frame_SOURCES = test-frame.c
test_frame_LDADD = ../src/common/libcommon.la

test_data_mmap_SOURCES = test-data-mmap.c
test_data_mmap_LDADD = ../src/common/libcommon.la

AM_CPPFLAGS = -I$(top_srcdir)/include
