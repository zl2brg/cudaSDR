# CI / AppImage / .deb packaging

| File | Role |
|------|------|
| `Dockerfile` | Ubuntu 24.04 + build deps + Qt 6.12.0 (`aqtinstall`) |
| `cudasdr.desktop` | Desktop entry for linuxdeploy and the .deb |
| `../scripts/make_appimage.sh` | AppImage from an existing build (`--appdir-only` stops after the AppDir) |
| `../scripts/make_deb.sh` | .deb from an existing build: Qt 6.12 bundled under `/opt/cudasdr`, distro libs as `Depends` |
| `../scripts/ci_appimage.sh` | Build + AppImage + .deb (runs inside the image; `BUILD_DEB=0` skips the .deb) |
| `../scripts/ci_ctest.sh` | Configure, build `build_tests`, run `ctest` (inside the image) |
| `../scripts/docker_appimage.sh` | Local wrapper: build image, run the CI packaging script |
| `../.github/workflows/appimage.yml` | GitHub Actions AppImage + .deb (tags / manual), smoke-installs the .deb |

The .deb targets Ubuntu 24.04 and KDE neon (the CI image's base). Settings live
in `~/.cudaSDR`, so they survive upgrades and removal of the package.
| `../.github/workflows/ctest.yml` | GitHub Actions unit tests on PR / push |

Local and GHA intentionally share one environment: **no host Qt bind-mount**.

### Unit tests locally (same image as CI)

```bash
docker build -f packaging/Dockerfile -t cudasdr-ci:24.04 packaging
docker run --rm -e JOBS="$(nproc)" -v "$PWD:/src:rw" -w /src \
  cudasdr-ci:24.04 ./scripts/ci_ctest.sh
```
