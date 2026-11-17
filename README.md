*This project has been created as part of the 42 curriculum by henrike.*

# scop

## Description

`scop` is a small GPU renderer that loads a Wavefront `.obj` model, displays it in
perspective, and lets you rotate/translate it around the object's center. Faces are
shown with subtle gray shades; pressing a key smoothly blends to a cheerful texture
and back. Matrices, OBJ parsing, BMP loading, and shader compilation are implemented
without third-party math/model/image libraries — only GLFW is used for the window
and input.

## Instructions

### Dependencies (one-liner)

```bash
make deps
```

This installs GLFW via Homebrew (macOS) or `apt` (Debian/Ubuntu).

### Build & run

```bash
make assets   # generate fallback textures (+ cube.obj) if missing
make
./scop resources/42.obj
```

Default texture: `resources/kittens.bmp` (24-bit BMP). Alternatives: `unicorn.bmp`.

Official subject resources live in `resources/` (`42.obj`, `teapot.obj`, `teapot2.obj`).

```bash
./scop resources/teapot2.obj
./scop resources/teapot.obj   # bonus: original with edge artifacts
./scop resources/cube.obj
```

### Controls

| Key | Action |
| --- | --- |
| Arrow keys / `J` `L` | Rotate around X / Y / Z |
| `W` `A` `S` `D` `Q` `E` | Translate |
| `T` | Toggle texture (smooth mix) |
| `Space` | Toggle auto-spin |
| `R` | Reset transform |
| `Z` | Wireframe |
| `Esc` | Quit |

## Technical choices

- **Language:** C++17
- **Graphics API:** OpenGL 4.1 core (works on macOS)
- **Window/input:** GLFW only
- **Implemented in-house:** `Mat4` / `Vec3`, OBJ parser (fan triangulation), 24-bit BMP loader, GLSL load/compile/link
- Object vertices are recentered so rotation/translation happen around the model center

## Resources

- [LearnOpenGL](https://learnopengl.com/) — OpenGL pipeline, shaders, textures
- [GLFW documentation](https://www.glfw.org/docs/latest/)
- [Wavefront OBJ format](https://en.wikipedia.org/wiki/Wavefront_.obj_file)
- [OpenGL 4.1 reference](https://www.khronos.org/registry/OpenGL-Refpages/gl4/)

### AI usage

AI assisted with scaffolding (Makefile layout, BMP header packing, and iterating on
the control loop / smooth texture mix). OBJ parsing, matrix math, shader code, and
rendering decisions were reviewed and adjusted manually so the defense can be
explained without relying on generated architecture.
