NAME      := scop
CXX       := c++
CXXFLAGS  := -std=c++17 -Wall -Wextra -Werror -O2 -Iinclude

UNAME_S   := $(shell uname -s)
UNAME_M   := $(shell uname -m)

ifeq ($(UNAME_S),Darwin)
  ifeq ($(UNAME_M),arm64)
    GLFW_PREFIX ?= /opt/homebrew
  else
    GLFW_PREFIX ?= /usr/local
  endif
  CXXFLAGS += -I$(GLFW_PREFIX)/include -DGL_SILENCE_DEPRECATION
  LDFLAGS  := -L$(GLFW_PREFIX)/lib -lglfw -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
else
  CXXFLAGS += $(shell pkg-config --cflags glfw3 2>/dev/null)
  LDFLAGS  := $(shell pkg-config --libs glfw3 2>/dev/null) -lGL -ldl -lpthread
endif

SRC := \
	src/main.cpp \
	src/math.cpp \
	src/shader.cpp \
	src/texture.cpp \
	src/obj_parser.cpp \
	src/mesh.cpp

OBJ := $(SRC:.cpp=.o)

.PHONY: all clean fclean re run deps assets

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $@ $(LDFLAGS)

src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all

run: $(NAME)
	./$(NAME) resources/42.obj

# One-line dependency bootstrap (macOS Homebrew / Linux apt).
deps:
	@if [ "$(UNAME_S)" = "Darwin" ]; then \
		brew install glfw; \
	elif command -v apt-get >/dev/null 2>&1; then \
		sudo apt-get update && sudo apt-get install -y libglfw3-dev pkg-config; \
	else \
		echo "Install GLFW3 for your platform, then re-run make"; \
		exit 1; \
	fi

assets:
	python3 scripts/gen_assets.py
