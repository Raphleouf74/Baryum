# ----------------------------
# Makefile Options
# ----------------------------
NAME = BARYUM
DESCRIPTION = "Baryum v0.2"
COMPRESSED = NO
ARCHIVED = NO

CFLAGS = -Wall -Wextra -Oz
CXXFLAGS = -Wall -Wextra -Oz

# ----------------------------
include $(shell cedev-config --makefile)
