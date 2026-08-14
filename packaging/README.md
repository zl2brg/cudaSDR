# CI / AppImage packaging

| File | Role |
|------|------|
| `Dockerfile` | Ubuntu 24.04 + build deps + Qt 6.11.0 (`aqtinstall`) |
| `cudasdr.desktop` | Desktop entry for linuxdeploy |
| `../scripts/ci_appimage.sh` | Build + AppImage (runs inside the image) |
| `../scripts/docker_appimage.sh` | Local wrapper: build image, run CI script |
| `../.github/workflows/appimage.yml` | GitHub Actions using the same image |

Local and GHA intentionally share one environment: **no host Qt bind-mount**.
