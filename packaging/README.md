# CI / AppImage packaging

| File | Role |
|------|------|
| `Dockerfile` | Ubuntu 24.04 + build deps + Qt 6.11.0 (`aqtinstall`) |
| `cudasdr.desktop` | Desktop entry for linuxdeploy |
| `../scripts/ci_appimage.sh` | Build + AppImage (runs inside the image) |
| `../scripts/ci_ctest.sh` | Configure, build `build_tests`, run `ctest` (inside the image) |
| `../scripts/docker_appimage.sh` | Local wrapper: build image, run AppImage CI script |
| `../.github/workflows/appimage.yml` | GitHub Actions AppImage (tags / manual) |
| `../.github/workflows/ctest.yml` | GitHub Actions unit tests on PR / push |

Local and GHA intentionally share one environment: **no host Qt bind-mount**.

### Unit tests locally (same image as CI)

```bash
docker build -f packaging/Dockerfile -t cudasdr-ci:24.04 packaging
docker run --rm -e JOBS="$(nproc)" -v "$PWD:/src:rw" -w /src \
  cudasdr-ci:24.04 ./scripts/ci_ctest.sh
```
