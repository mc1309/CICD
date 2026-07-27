# Assignment 3 — "Fan Dashboard HUD"

The Assignment 2 spinning 3D fan, extended with a screen-space HUD
dashboard and Phong lighting. The renderer is now split into a 3D world
scene (fan + procedural checkerboard ground, Phong-lit) and a 2D
screen-space HUD scene (buttons + a live speed bar), rendered in a fixed
order every frame. Runs on Desktop, Android, and WebGL from one shared
C++ code base.

- HUD `+SPD` / `-SPD` = adjust fan rotation speed (0–20), eased smoothly
- HUD `+ZOOM` / `-ZOOM` = adjust camera distance (3–15), eased smoothly
- Click / tap directly on the fan (not a HUD button) = 3D pick a part
  (base, pole, hub, or a specific blade) and highlight it
- Old tap-to-toggle / drag-to-boost gestures from Assignment 2 are removed

## Project layout

```
CMakeLists.txt            <- Desktop (GLFW + GLEW + GLM) entry point
script_build_and_run.bat  <- Desktop convenience wrapper
build_web.bat / .sh       <- Web (Emscripten) entry point
cmake/ImportDependencies.cmake
Scene/                    <- shared by ALL THREE platforms
  Platform.h, Model.h, ShaderHelper.h
  Transform.h/.cpp        <- provided, unmodified
  Fan.h/.cpp               <- fan geometry + Phong lighting + picking data
  Ground.h/.cpp            <- procedural checkerboard floor
  Scene3D.h/.cpp           <- owns Fan + Ground, camera, light, PickAt()
  SceneHUD.h/.cpp          <- buttons + speed bar, smooth easing bonus
  Renderer.h/.cpp          <- owns Scene3D + SceneHUD, draw/touch pipeline
  main.cpp
shader/                   <- Desktop/Web .glsl files
android/                  <- Android Studio / Gradle project
  app/src/main/assets/shader/   <- Android copy of the .glsl files
```

## 1. Desktop (Windows)

Prerequisites: CMake 3.15+, Visual Studio 2022 (Desktop C++ workload), Git.

```bat
script_build_and_run.bat
```

This configures CMake (first run fetches GLFW, GLEW, and GLM via
`FetchContent` — allow a few minutes), builds the `Debug` configuration,
and launches `build\Debug\GLPIFrameworkIntro.exe`. Click a HUD button to
adjust speed/zoom, or click directly on the fan to pick a part. Press
`ESC` to quit.

## 2. Android

Prerequisites: Android Studio (Arctic Fox+), Android NDK + CMake
components installed via the SDK Manager.

1. Open the `android/` folder in Android Studio ("Open an existing project").
2. Let Gradle sync (it fetches GLM automatically the first time it
   configures the native module).
3. Run on a device/emulator with API 24+ and OpenGL ES 3.0 support.
4. Tap a HUD button to adjust speed/zoom, or tap directly on the fan to
   pick a part.
5. Watch logs with:
   ```
   adb logcat -s Fan3D GLPIFrameworkIntroNative
   ```

## 3. Web (WebGL / Emscripten)

Prerequisites: [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html)
activated in your shell (`emsdk_env.bat` / `source emsdk_env.sh`), Git,
Python 3 (for serving locally).

```bat
build_web.bat
```
```bash
./build_web.sh
```

This fetches GLM headers into `external/glm` (first run only) and
produces `web/index.html` + `web/index.js` + `web/index.wasm`. Serve it
locally, since browsers block `file://` WASM loads:

```bash
cd web
python -m http.server 8080
```

Then open `http://localhost:8080` in a browser. Click maps to the same
tap gestures as touch.

## Bonus feature: smooth zoom/speed easing

Pressing a HUD button no longer jumps speed/zoom straight to the new
value — the button sets a target, and the displayed value glides toward
it a little each frame, so the fan, the camera, and the speed bar all
move smoothly instead of snapping.

Implemented entirely in `Scene/SceneHUD.h` / `Scene/SceneHUD.cpp`:

| Function | Role |
|---|---|
| `SceneHUD.h` private fields | `speed`/`targetSpeed`, `zoomDistance`/`targetZoom`, and the tuning constant `kEaseFactor = 0.12f` |
| `HandleTouchDown()` | A button press moves only the *target* instantly, not the displayed value |
| `Render()` | Each frame, eases the displayed value 12% of the way toward its target, snapping once the gap is negligible |
| `CurrentSpeed()` / `CurrentZoom()` | Unchanged getters — since they already return the eased values, `Renderer`/`Scene3D` needed no changes to inherit the smooth motion |

## CI/CD

`.github/workflows/build-and-deploy.yml` builds the Desktop target on
every push to `main`, then builds the Web target and deploys it to
GitHub Pages via `actions/deploy-pages`. See the repository's **Actions**
tab for the latest run, and **Settings → Pages** for the live URL
(Pages source: **GitHub Actions**).

> Publishing Pages from a **private** repository requires GitHub Pro,
> available for free via the GitHub Student Developer Pack.