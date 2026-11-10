# ----------------------------
# Makefile Options
# ----------------------------
NAME = BARIUM
DESCRIPTION = "Barium v0.5.1"
COMPRESSED = NO
ARCHIVED = YES

CFLAGS = -Wall -Wextra -Oz
CXXFLAGS = -Wall -Wextra -Oz

# ----------------------------
include $(shell cedev-config --makefile)
