# Assignment 2 — "3D Fan"

A 3D table fan (base, pole, hub, 4 spinning blades) built from a single
cube and the provided `Transform` push/pop matrix stack, running on
Desktop, Android, and WebGL from one shared C++ code base.

- Tap / click = toggle fan ON/OFF (logged as `Fan ON` / `Fan OFF`)
- Drag / swipe = temporarily speeds up the spin, proportional to drag velocity

## Project layout

```
CMakeLists.txt            <- Desktop (GLFW + GLEW + GLM) entry point
script_build_and_run.bat  <- Desktop convenience wrapper
build_web.bat / .sh       <- Web (Emscripten) entry point
cmake/ImportDependencies.cmake
Scene/                    <- shared by ALL THREE platforms
  Platform.h, Model.h, Renderer.h/.cpp, ShaderHelper.h
  Transform.h/.cpp        <- provided, unmodified
  Fan.h/.cpp               <- assignment implementation
  main.cpp
android/                  <- Android Studio / Gradle project
```

## 1. Desktop (Windows)

Prerequisites: CMake 3.15+, Visual Studio 2022 (Desktop C++ workload), Git.

```bat
script_build_and_run.bat
```

This configures CMake (first run fetches GLFW, GLEW, and GLM via
`FetchContent` — allow a few minutes), builds the `Debug` configuration,
and launches `build\Debug\GLPIFrameworkIntro.exe`. Click-and-hold to
drag/swipe; a plain click taps. Press `ESC` to quit.

## 2. Android

Prerequisites: Android Studio (Arctic Fox+), Android NDK + CMake
components installed via the SDK Manager.

1. Open the `android/` folder in Android Studio ("Open an existing project").
2. Let Gradle sync (it fetches GLM automatically the first time it
   configures the native module).
3. Run on a device/emulator with API 24+ and OpenGL ES 3.0 support.
4. Tap the screen to toggle the fan; drag to speed it up.
5. Watch gesture logs with:
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

Then open `http://localhost:8080` in a browser. Click-and-drag maps to
the same tap/swipe gestures as touch.

## CI/CD

`.github/workflows/build-and-deploy.yml` builds the Desktop target on
every push to `main`, then builds the Web target and deploys it to
GitHub Pages via `actions/deploy-pages`. See the repository's **Actions**
tab for the latest run, and **Settings → Pages** for the live URL
(Pages source: **GitHub Actions**).

> Publishing Pages from a **private** repository requires GitHub Pro,
> available for free via the GitHub Student Developer Pack.
